#include "Partition.h"
using namespace std;

InstancePartitions::InstancePartitions(uint32_t allparts, uint32_t* numparts){
	this->allparts = allparts;
	uint32_t partsTemp = 0;
	for(int i = 0; i < numprocs; i++){
		this->numparts.push_back(numparts[i]);
		if(procid == i){
			this->startpart = partsTemp;
		}
		partsTemp += numparts[i];
	}

	for(int i = 0; i < numparts[procid]; i++){
		Partition* partition = new Partition();
		partitions.push_back(partition);
	}
}

// MPI进程交互，获取全局点的度信息
void InstancePartitions::getAllVerticesDegree(){
	int sendSize = vertexDegree.size() * 2;
	int recvSize = 0;
	MPI_Allreduce(&sendSize, &recvSize, 1, MPI_UINT32_T, MPI_SUM, MPI_COMM_WORLD);
	// LOG(INFO) << "Task: " << procid << " recvSize: " << recvSize << endl;

	uint32_t sendbuf[sendSize];
	uint32_t recvbuf[recvSize];
	map<uint32_t, uint32_t>::iterator iter = vertexDegree.begin();
	for(int i = 0; iter != vertexDegree.end(); iter++, i+=2){
		sendbuf[i] = iter->first;
		sendbuf[i+1] = iter->second;
	}

	int recvCounts[numprocs];
	int rdisps[numprocs];
	MPI_Allgather(&sendSize, 1, MPI_INT, recvCounts, 1, MPI_INT, MPI_COMM_WORLD);
	rdisps[0] = 0;
	for(int i = 1; i < numprocs; i++){
		rdisps[i] = rdisps[i-1] + recvCounts[i-1];
	}

	MPI_Allgatherv(sendbuf, sendSize, MPI_UINT32_T, \
		recvbuf, recvCounts, rdisps, MPI_UINT32_T, MPI_COMM_WORLD);

	vertexDegree.clear();
	for(int i = 0; i < recvSize; i+=2){
		if(vertexDegree.count(recvbuf[i]) == 0){
			vertexDegree[recvbuf[i]] = recvbuf[i+1];
		}
		else{
			vertexDegree[recvbuf[i]] += recvbuf[i+1];
		}
	}
}

void InstancePartitions::getVRF(){
	int numPartitionVertices[numparts[procid]];
	for(int i = 0; i < numparts[procid]; i++){
		numPartitionVertices[i] = partitions[i]->vertices.size();
	}
	int numInstanceVertices = 0;
	for(int i = 0; i < numparts[procid]; i++){
		numInstanceVertices += numPartitionVertices[i];
	}
	int sumVertices;
	MPI_Allreduce(&numInstanceVertices, &sumVertices, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	VRF = sumVertices / 1.0 / nvertices_global;
	// LOG(INFO) << "VRF: " << VRF << endl;
}

void InstancePartitions::getBalance(){
	int numPartitionEdges[numparts[procid]];
	for(int i = 0; i < numparts[procid]; i++){
		numPartitionEdges[i] = partitions[i]->edges.size();
	}
	int recvCounts[numprocs];
	MPI_Allgather(&numparts[procid], 1, MPI_INT, recvCounts, 1, MPI_INT, MPI_COMM_WORLD);
	int rdisps[numprocs];
	rdisps[0] = 0;
	for(int i = 1; i < numprocs; i++){
		rdisps[i] = rdisps[i-1] + recvCounts[i-1];
	}
	int numAllPartitionsEdges[allparts];
	MPI_Allgatherv(numPartitionEdges, numparts[procid], MPI_INT, \
		numAllPartitionsEdges, recvCounts, rdisps, MPI_INT, MPI_COMM_WORLD);
	// LOG(INFO) << "Get all partitions size";
	int maxPartitionSize = 0;
	int minPartitionSize = 1000000000;
	int avePartitionSize = nedges_global / allparts;
	long long sumSquare = 0;

	for(int i = 0; i < allparts; i++){
		if(maxPartitionSize < numAllPartitionsEdges[i]){
			maxPartitionSize = numAllPartitionsEdges[i];
		}
		if(minPartitionSize > numAllPartitionsEdges[i]){
			minPartitionSize = numAllPartitionsEdges[i];
		}
		sumSquare += pow((numAllPartitionsEdges[i] - avePartitionSize), 2);
	}
	balance_MAX_MIN = (maxPartitionSize - minPartitionSize) / 1.0 / avePartitionSize;
	balance_RSD = sqrt(sumSquare / 1.0 / (allparts-1)) / avePartitionSize;
	// LOG(INFO) << "balance_MAX_MIN: " << balance_MAX_MIN << " balance_RSD: " << balance_RSD << endl;
}

// 将小度数点全局分配
void InstancePartitions::getSmallDegreeVertices(uint32_t degree){
	map<uint32_t, uint32_t>::iterator iter;
	for(iter = vertexDegree.begin(); iter != vertexDegree.end(); iter++){
		if(iter->second > degree){
			continue;
		}
		uint32_t part = iter->first % allparts;
		if(part >= startpart && part < startpart + numparts[procid]){
			partitions[part - startpart]->smallDegreeVertices.push_back(iter->first);
		}
	}
}

// 小度点交换阶段，获取每条边对应partition
void InstancePartitions::getEdges2Partition(uint32_t degree){
	for(int i = 0; i < partitions.size(); i++){
		for(int j = 0; j < partitions[i]->edges.size(); j++){
			uint32_t ver = vertexDegree[partitions[i]->edges[j].src.ver] < vertexDegree[partitions[i]->edges[j].dst.ver] ? \
							partitions[i]->edges[j].src.ver : partitions[i]->edges[j].dst.ver;
			Edges2Partition[partitions[i]->edges[j]] = ver % allparts;
		}
	}
}

// 小度点交换阶段，分发全部的边
void InstancePartitions::exchangeAllEdges(map<Edge, int>& Edges2Partition){
	vector<Edge> coldEdges;
	map<Edge, int>::iterator iter;
	for(iter = Edges2Partition.begin(); iter != Edges2Partition.end(); iter++){
		coldEdges.push_back(iter->first);
	}
	QuickSortEdgePart(coldEdges, Edges2Partition, 0, coldEdges.size()-1);  // 将冷边按partition序排列
	// 全局更新交换边信息
	uint32_t sendArray[coldEdges.size()*3];
	for(int i = 0; i < coldEdges.size(); i++){
		sendArray[i*3] = coldEdges[i].src.ver;
		sendArray[i*3+1] = coldEdges[i].dst.ver;
		sendArray[i*3+2] = Edges2Partition[coldEdges[i]];
	}
	
	int sendCounts[numprocs] = {0};
	int endparts[numprocs];
	endparts[0] = numparts[0];
	for(int i = 1; i < numprocs; i++){
		endparts[i] = numparts[i] + endparts[i-1];
	}
	int index = 0;
	int k = 0;
	for(int i = 0; i < coldEdges.size(); i++){
		
		if(Edges2Partition[coldEdges[i]] < endparts[k]){
			index++;
		}
		else{
			sendCounts[k] = index * 3;
			k++;
			i--;                         // 刚才不满足条件的这个边要重新考虑
			index = 0;
		}
	}
	sendCounts[k] = index * 3;
	
	int recvCounts[numprocs] = {0};
	MPI_Alltoall(sendCounts, 1, MPI_INT, recvCounts, 1, MPI_INT, MPI_COMM_WORLD);
	
	int sdispls[numprocs];
	sdispls[0] = 0;
	for(int i = 1; i < numprocs; i++){
		sdispls[i] = sdispls[i-1] + sendCounts[i-1];
	}

	int rdispls[numprocs];
	rdispls[0] = 0;
	for(int i = 1; i < numprocs; i++){
		rdispls[i] = rdispls[i-1] + recvCounts[i-1];
	}

	int lenRecv = 0;
	for(int i = 0; i < numprocs; i++){
		lenRecv += recvCounts[i];
	}

	uint32_t recvArray[lenRecv];
	MPI_Alltoallv(sendArray, sendCounts, sdispls, MPI_INT, recvArray, recvCounts, rdispls, MPI_INT, MPI_COMM_WORLD);
	
	Edges2Partition.clear();
	for(int i = 0; i < lenRecv; i+=3){
		Edge e;
		e.src.ver = recvArray[i];
		e.dst.ver = recvArray[i+1];
		Edges2Partition[e] = recvArray[i+2];
	}
}

// 将内部市场中的边打包整理
void InstancePartitions::arrangeInternalMarket(){
	vertex2edgesets.clear();
	for(int i = 0; i < internalMarket.size(); i++){
		int src = internalMarket[i].src.ver;
		int dst = internalMarket[i].dst.ver;
		int ver = vertexDegree[src] < vertexDegree[dst] ? src : dst;
		if(vertex2edgesets.count(ver) == 0){
			EdgeSet edgeset;
			edgeset.e.push_back(internalMarket[i]);
			edgeset.coreVertex = ver;
			edgeset.times = 1;
			vertex2edgesets[ver] = edgeset;
		}
		else{
			vertex2edgesets[ver].e.push_back(internalMarket[i]);
			vertex2edgesets[ver].times += 1;
		}
	}
}

// 每个进程初始化
void InstancePartitions::InstanceInit(){
	// 对每个partition的初始化
	for(int i = 0; i < numparts[procid]; i++){
		partitions[i]->getVerticesAndDegree();
	}

	// 对每个MPI进程的初始化
	getAllVerticesDegree();
	nvertices_global = vertexDegree.size();
	getVRF();
	getBalance();

}

// 每个进程初步边交换
void InstancePartitions::InstanceExchange(){
	// if(procid == 0){
	// 	map<uint32_t, uint32_t>::iterator iter;
	// 	int k = 0;
	// 	for(iter = vertexDegree.begin(); iter != vertexDegree.end(); iter++){
	// 		if(iter->second > 1){
	// 			continue;
	// 		}
	// 		k++;
	// 	}
	// 	cout << k << endl;
	// }
	// getSmallDegreeVertices(3);
	// getEdges2Partition(3);
	// exchangeAllEdges(Edges2Partition);

	// for(int i = 0; i < partitions.size(); i++){
	// 	partitions[i]->edges.clear();
	// }

	// map<Edge, int>::iterator iter;
	// for(iter = Edges2Partition.begin(); iter != Edges2Partition.end(); iter++){
	// 	partitions[iter->second - startpart]->edges.push_back(iter->first);
	// }
	// updateAllPartitions();

	// 市场模型
	for(int i = 0; i < partitions.size(); i++){
		partitions[i]->getSellEdge(this, 1, 10, 0.3);
		internalMarket.insert(internalMarket.end(), partitions[i]->sellEdge.begin(), partitions[i]->sellEdge.end());
		partitions[i]->sellEdge.clear();
	}

	// 对内部市场的边进行整理，相当于预处理，对相同小度点的边打包卖
	arrangeInternalMarket();

    // map<uint32_t, EdgeSet>::iterator iter;
    // if(procid == 0)
    // for(iter = vertex2edgesets.begin(); iter != vertex2edgesets.end(); iter++){
    // 	cout << iter->first << " " << iter->second.times << endl;
    // }

	// 进行内部市场选购
	map<uint32_t, EdgeSet>::iterator iter;
	for(iter = vertex2edgesets.begin(); iter != vertex2edgesets.end(); iter++){
		bool selled = false;
		for(int i = 0; i < partitions.size(); i++){
			// 达到了认购的条件
			if(partitions[i]->vertices.find(iter->first) != partitions[i]->vertices.end() && \
				partitions[i]->money + 10 > iter->second.times){
				partitions[i]->edges.insert(partitions[i]->edges.end(), iter->second.e.begin(), iter->second.e.end());
				partitions[i]->money -= iter->second.times;
				selled = true;
				break;
			}
		}
		if(!selled){
			// restOfInternalMarket.push_back(iter->second);
			
		}
	}
	
}

// 每个进程迭代优化
bool InstancePartitions::InstanceIteration(){
	for(int i = 0; i < numparts[procid]; i++){
		partitions[i]->getHotColdVertices(this, 0.5, 0.5);
		partitions[i]->getColdEdges(0.5);
	}
	// print4debug(32);
	getAllHotVertices();
	computeEdgesMatchPartitions();
	distributeAllColdEdges();
	updateAllPartitions();
}

// 每个进程获取到全部的热点数据信息
void InstancePartitions::getAllHotVertices(){
	// 每一轮之后统一处理
	// allHotVertices.clear();
	// allHotVerticesScore.clear();
	// InstanceIndexStart.clear();
	// InstanceIndexEnd.clear();
	// PartitionIndexStart.clear();
	// PartitionIndexEnd.clear();
	// PartitionIndexLen.clear();
	
	// 热点信息构建
	vector<uint32_t> sendArray;
	for(int i = 0; i < partitions.size(); i++){
		// PartitionIndexStart.push_back(sendArray.size());
		for(int j = 0; j < partitions[i]->hotVertices.size(); j++){
			sendArray.push_back(partitions[i]->hotVertices[j]);
		}
		// PartitionIndexEnd.push_back(sendArray.size());
		PartitionIndexLen.push_back(partitions[i]->hotVertices.size());
	}

	if(debug){
		if(procid == 0){
			// for(int i = 0; i < PartitionIndexStart.size(); i++){
			// 	cout << PartitionIndexStart[i] << endl;
			// }
			// for(int i = 0; i < PartitionIndexEnd.size(); i++){
			// 	cout << PartitionIndexEnd[i] << endl;
			// }
			for(int i = 0; i < PartitionIndexLen.size(); i++){
				cout << PartitionIndexLen[i] << endl;
			}
		}
	}

	// 热点信息传输
	int sendcount = sendArray.size();
	int recvcounts[numprocs];
	MPI_Allgather(&sendcount, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD);
	
	int lenRecv = 0;
	for(int i = 0; i < numprocs; i++){
		lenRecv += recvcounts[i];
	}
	int recvArray[lenRecv];
	int displs[numprocs];
	displs[0] = 0;
	for(int i = 1; i < numprocs; i++){
		displs[i] = displs[i-1] + recvcounts[i-1];
	}
	MPI_Allgatherv(sendArray.data(), sendcount, MPI_INT, recvArray, recvcounts, displs, MPI_INT, MPI_COMM_WORLD);
	allHotVertices.reserve(lenRecv);
	allHotVertices.assign(&recvArray[0], &recvArray[lenRecv]);

	// 热点score信息传输
	vector<double> sendArray_score;
	for(int i = 0; i < partitions.size(); i++){
		for(int j = 0; j < partitions[i]->hotVertices.size(); j++){
			sendArray_score.push_back(partitions[i]->vertexScore[partitions[i]->hotVertices[j]]);
		}
	}
	double recvArray_score[lenRecv];
	MPI_Allgatherv(sendArray_score.data(), sendcount, MPI_DOUBLE, recvArray_score, recvcounts, displs, MPI_DOUBLE, MPI_COMM_WORLD);
	allHotVerticesScore.reserve(lenRecv);
	allHotVerticesScore.assign(&recvArray_score[0], &recvArray_score[lenRecv]);
	
	// Instance Index 信息构建
	int indexTemp = 0;
	InstanceIndexStart.push_back(indexTemp);
	for(int i = 0; i < numprocs - 1; i++){
		indexTemp += recvcounts[i];
		InstanceIndexStart.push_back(indexTemp);
		InstanceIndexEnd.push_back(indexTemp);
	}
	indexTemp += recvcounts[numprocs-1];
	InstanceIndexEnd.push_back(indexTemp);

	if(debug){
		if(procid == 0){
			for(int i = 0; i < InstanceIndexStart.size(); i++){
				cout << InstanceIndexStart[i] << endl;
			}
			cout << endl;
			for(int i = 0; i < InstanceIndexEnd.size(); i++){
				cout << InstanceIndexEnd[i] << endl;
			}
		}
	}

	// Partition Index 信息传输构建
	sendcount = PartitionIndexLen.size();
	for(int i = 0; i < numprocs; i++){
		recvcounts[i] = numparts[i];
	}
	lenRecv = allparts;
	int recvArray_PartitionIndex[lenRecv];
	displs[0] = 0;
	for(int i = 1; i < numprocs; i++){
		displs[i] = displs[i-1] + recvcounts[i-1];
	}
	MPI_Allgatherv(PartitionIndexLen.data(), sendcount, MPI_INT, recvArray_PartitionIndex, recvcounts, displs, MPI_INT, MPI_COMM_WORLD);
	PartitionIndexLen.clear();
	PartitionIndexLen.reserve(lenRecv);
	PartitionIndexLen.assign(&recvArray_PartitionIndex[0], &recvArray_PartitionIndex[lenRecv]);

	indexTemp = 0;
	PartitionIndexStart.push_back(indexTemp);
	for(int i = 0; i < lenRecv - 1; i++){
		indexTemp += PartitionIndexLen[i];
		PartitionIndexStart.push_back(indexTemp);
		PartitionIndexEnd.push_back(indexTemp);
	}
	indexTemp += PartitionIndexLen[lenRecv-1];
	PartitionIndexEnd.push_back(indexTemp);
}

// 计算冷边的分配
void InstancePartitions::computeEdgesMatchPartitions(){
	// coldEdges2Partition.clear();
	// 对每一条边进行处理
	int all = 0, change = 0;
	for(int i = 0; i < partitions.size(); i++){
		for(int j = 0; j < partitions[i]->coldEdges.size(); j++){
			Edge eTemp = partitions[i]->coldEdges[j];
			double matchScores[allparts];
			int part = 0;
			bool isChanged = false;
			for(int k = 0; k < allparts; k++){
				matchScores[k] = computerMatchScore(eTemp, k);
				if(matchScores[k] > matchScores[part]){
					isChanged = true;
					part = k;
				}
			}
			all++;
			if(isChanged || matchScores[0] > 0){
				coldEdges2Partition[eTemp] = part;
				change++;
			}
			else{
				coldEdges2Partition[eTemp] = i + startpart;
			}
		}
	}

	if(procid == 1){
		map<Edge, int>::iterator iter;
		for(iter = coldEdges2Partition.begin(); iter != coldEdges2Partition.end(); iter++){
			// cout << iter->second << endl;
		}
		// cout << "Task: " << procid << ". all cold edges: " << all << ". need to change: " << change << endl;
	}
}

// 计算一条边与partition的match score
// 可丰富策略
double InstancePartitions::computerMatchScore(Edge e, int part){
	double score = 0.0;
	uint32_t src = e.src.ver;
	uint32_t dst = e.dst.ver;
	for(int i = PartitionIndexStart[part]; i < PartitionIndexEnd[part]; i++){
		if(src == allHotVertices[i]){
			score += allHotVerticesScore[i];
		}
		if(dst == allHotVertices[i]){
			score += allHotVerticesScore[i];
		}
	}
	return score;
}

// 对冷边进行分发
void InstancePartitions::distributeAllColdEdges(){
	vector<Edge> coldEdges;
	map<Edge, int>::iterator iter;
	for(iter = coldEdges2Partition.begin(); iter != coldEdges2Partition.end(); iter++){
		coldEdges.push_back(iter->first);
	}
	QuickSortEdgePart(coldEdges, coldEdges2Partition, 0, coldEdges.size()-1);  // 将冷边按partition序排列
	// 全局更新交换边信息
	uint32_t sendArray[coldEdges.size()*3];
	for(int i = 0; i < coldEdges.size(); i++){
		sendArray[i*3] = coldEdges[i].src.ver;
		sendArray[i*3+1] = coldEdges[i].dst.ver;
		sendArray[i*3+2] = coldEdges2Partition[coldEdges[i]];
	}
	
	int sendCounts[numprocs] = {0};
	int endparts[numprocs];
	endparts[0] = numparts[0];
	for(int i = 1; i < numprocs; i++){
		endparts[i] = numparts[i] + endparts[i-1];
	}
	int index = 0;
	int k = 0;
	for(int i = 0; i < coldEdges.size(); i++){
		
		if(coldEdges2Partition[coldEdges[i]] < endparts[k]){
			index++;
		}
		else{
			sendCounts[k] = index * 3;
			k++;
			i--;                         // 刚才不满足条件的这个边要重新考虑
			index = 0;
		}
	}
	sendCounts[k] = index * 3;
	
	// for(int i = 0; i < numprocs; i++){
	// 	if(procid == i){
	// 		cout << "Task: " << procid <<" ";
	// 		for(int j = 0; j < numprocs; j++){
	// 			cout << sendCounts[j] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	else{
	// 		sleep(1);
	// 	}
	// }

	int recvCounts[numprocs] = {0};
	MPI_Alltoall(sendCounts, 1, MPI_INT, recvCounts, 1, MPI_INT, MPI_COMM_WORLD);
	
	// MPI_Barrier(MPI_COMM_WORLD);
	// for(int i = 0; i < numprocs; i++){
	// 	if(procid == i){
	// 		cout << "Task: " << procid <<" ";
	// 		for(int j = 0; j < numprocs; j++){
	// 			cout << recvCounts[j] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	else{
	// 		sleep(1);
	// 	}
	// }

	int sdispls[numprocs];
	sdispls[0] = 0;
	for(int i = 1; i < numprocs; i++){
		sdispls[i] = sdispls[i-1] + sendCounts[i-1];
	}

	int rdispls[numprocs];
	rdispls[0] = 0;
	for(int i = 1; i < numprocs; i++){
		rdispls[i] = rdispls[i-1] + recvCounts[i-1];
	}

	int lenRecv = 0;
	for(int i = 0; i < numprocs; i++){
		lenRecv += recvCounts[i];
	}

	uint32_t recvArray[lenRecv];
	MPI_Alltoallv(sendArray, sendCounts, sdispls, MPI_INT, recvArray, recvCounts, rdispls, MPI_INT, MPI_COMM_WORLD);

	// if(procid == 0){
	// 	for(int i = 0; i < lenRecv; i+=3){
	// 		cout << recvArray[i] << " " << recvArray[i+1] << " " << recvArray[i+2] << endl;
	// 	}
	// }
	updateAllEdges(recvArray, lenRecv);
}

// 更新边信息
void InstancePartitions::updateAllEdges(uint32_t* updateEdges, int len){
	// 目前edges最前面的就是冷边
	// 后期可在选出冷边时就将其删除
	vector<Edge> edgesTemp;
	for(int i = 0; i < partitions.size(); i++){
		edgesTemp.clear();
		edgesTemp.assign(partitions[i]->edges.begin() + partitions[i]->coldEdges.size(), partitions[i]->edges.end());
		partitions[i]->edges.clear();
		partitions[i]->edges.assign(edgesTemp.begin(), edgesTemp.end());
	}
	// 将新边分发
	for(int i = 0; i < len; i+=3){
		Edge e;
		e.src.ver = updateEdges[i];
		e.dst.ver = updateEdges[i+1];
		partitions[updateEdges[i+2]-startpart]->edges.push_back(e);
	}
}

// partition全局状态更新
void InstancePartitions::updateAllPartitions(){
	// 更新点集与点的局部度数
	for(int i = 0; i < partitions.size(); i++){
		partitions[i]->getVerticesAndDegree();
	}
	// 更新指标
	getVRF();
	getBalance();
	// 清空全部中间信息
	for(int i = 0; i < partitions.size(); i++){
		partitions[i]->hotVertices.clear();
		partitions[i]->coldVertices.clear();
		partitions[i]->hotEdges.clear();
		partitions[i]->coldEdges.clear();
		partitions[i]->vertexScore.clear();
		partitions[i]->edgeSocre.clear();
	}
	allHotVertices.clear();
	allHotVerticesScore.clear();
	InstanceIndexStart.clear();
	InstanceIndexEnd.clear();
	PartitionIndexStart.clear();
	PartitionIndexEnd.clear();
	PartitionIndexLen.clear();
	coldEdges2Partition.clear();  // 这个很关键
}

// 只要边发生变化，则执行该操作(每次迭代之后必须要更新)
void Partition::getVerticesAndDegree(){
	set<uint32_t> t;
	vertices.clear();
	vertexPartDegree.clear();
	for(int i = 0; i < edges.size(); i++){
		t.insert(edges[i].src.ver);
		t.insert(edges[i].dst.ver);

		if(vertexPartDegree.count(edges[i].src.ver) == 0){
			vertexPartDegree[edges[i].src.ver] = 1;
		}
		else{
			vertexPartDegree[edges[i].src.ver] += 1;
		}
		if(vertexPartDegree.count(edges[i].dst.ver) == 0){
			vertexPartDegree[edges[i].dst.ver] = 1;
		}
		else{
			vertexPartDegree[edges[i].dst.ver] += 1;
		}
	}
	vertices = t;
}

// 获取每个点的score (这里可以丰富策略)
void Partition::getVertexScore(InstancePartitions* ins_partition){
	set<uint32_t>::iterator iter;
	for(iter = vertices.begin(); iter != vertices.end(); iter++){
		vertexScore[*iter] = vertexPartDegree[*iter] / 1.0 / ins_partition->vertexDegree[*iter];
	}
}

// 获取热点信息
void Partition::getHotColdVertices(InstancePartitions* ins_partition, double hot, double cold){
	getVertexScore(ins_partition);
	vector<uint32_t> verticesTemp;
	std::copy(vertices.begin(), vertices.end(), std::back_inserter(verticesTemp)); // 直接将set copy 到 vector
	QuickSortVertex(verticesTemp, vertexScore, 0, verticesTemp.size()-1);
	hotVertices.assign(verticesTemp.end() - verticesTemp.size() * hot, verticesTemp.end());
	coldVertices.assign(verticesTemp.begin(), verticesTemp.begin() + verticesTemp.size() * cold);
	// cout << "Task: " << procid << " hotVertices: " << hotVertices.size() << " coldVertices: " << coldVertices.size() << endl;
}

// 获取每一条边的score（这里可以丰富策略）
void Partition::getEdgeScore(){
	vector<Edge>::iterator iter;
	for(iter = edges.begin(); iter != edges.end(); iter++){
		edgeSocre[*iter] = vertexScore[iter->src.ver] + vertexScore[iter->dst.ver];
	}
}

// 获取冷边信息
void Partition::getColdEdges(double cold){
	getEdgeScore();
	QuickSortEdge(edges, edgeSocre, 0, edges.size()-1);
	coldEdges.assign(edges.begin(), edges.begin() + edges.size() * cold);
	// cout << "Task: " << procid << " coldEdges: " << coldEdges.size() << endl;
}

// 市场价值模型
void Partition::getSellEdge(InstancePartitions* ins_partition, int startDegree, int endDegree, double threshold){
	// 每次交换到最后面而不是删除，方便统一操作
	int endEdges = edges.size() - 1;
	for(int i = 0; i <= endEdges; i++){
		uint32_t src = edges[i].src.ver;
		uint32_t dst = edges[i].dst.ver;
		uint32_t ver;
		if(ins_partition->vertexDegree[src] < ins_partition->vertexDegree[dst]){
			if(ins_partition->vertexDegree[src] > 1){
				ver = src;
			}
			else{
				ver = dst;
			}
		}
		else{
			if(ins_partition->vertexDegree[dst] > 1){
				ver = dst;
			}
			else{
				ver = src;
			}
		}
		if(ins_partition->vertexDegree[ver] >= startDegree && ins_partition->vertexDegree[ver] <= endDegree){
			if(vertexPartDegree[ver] / 1.0 / ins_partition->vertexDegree[ver] <= threshold){
				swap(edges[endEdges], edges[i]);
				i--;
				endEdges--;
			} 
		}
	}
	sellEdge.clear();
	sellEdge.assign(edges.begin() + endEdges + 1, edges.end());
	edges.erase(edges.begin() + endEdges + 1, edges.end());
	getVerticesAndDegree();
	// for(int i = 0; i < sellEdge.size(); i++){
	// 	money += 1.0/ins_partition->vertexDegree[sellEdge[i].src.ver];
	// 	money += 1.0/ins_partition->vertexDegree[sellEdge[i].dst.ver];
	// }
	money += sellEdge.size();
}

// for debug
void Partition::printHotVertices(){
	vector<uint32_t>::iterator iter;
	cout << "HotVertices of this partition is: " << endl;
	for(iter = hotVertices.begin(); iter != hotVertices.end(); iter++){
		cout << *iter << "\t\t" << vertexScore[*iter] << endl;
	}
}

void Partition::printColdEdges(){
	vector<Edge>::iterator iter;
	cout << "ColdEdges of this partition is: " << endl;
	for(iter = coldEdges.begin(); iter != coldEdges.end(); iter++){
		cout << iter->src.ver << "\t\t" << iter->dst.ver << "\t\t" << edgeSocre[*iter] << endl;
	}
}

void Partition::printAllVertices(){
	vector<uint32_t> verticesTemp;
	std::copy(vertices.begin(), vertices.end(), std::back_inserter(verticesTemp)); // 直接将set copy 到 vector
	QuickSortVertex(verticesTemp, vertexScore, 0, verticesTemp.size()-1);
	cout << "AllVertices of this partition is: " << endl;
	vector<uint32_t>::iterator iter;
	for(iter = verticesTemp.begin(); iter != verticesTemp.end(); iter++){
		cout << *iter << "\t\t" << vertexScore[*iter] << endl;
	}
}

void Partition::printAllEdges(){
	cout << "AllEdges of this partition is: " << endl;
	vector<Edge>::iterator iter;
	for(iter = edges.begin(); iter != edges.end(); iter++){
		cout << iter->src.ver << "\t\t" << iter->dst.ver << "\t\t" << edgeSocre[*iter] << endl;
	}
}

void InstancePartitions::print4debug(int part){
	int pid = -1, pidpart = -1;
	if(part >= startpart && part < startpart + numparts[procid]){
		pid = procid;
		pidpart = part - startpart;
	}
	if(procid == pid){
		cout << "Task: " << pid << " partition: " << pidpart << endl;
		Partition* partition = partitions[pidpart];
		// partition->printAllVertices();
		partition->printHotVertices();
		// partition->printAllEdges();
		partition->printColdEdges();
	}
}