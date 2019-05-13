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

// 每个进程迭代优化
bool InstancePartitions::InstanceIteration(){
	for(int i = 0; i < numparts[procid]; i++){
		partitions[i]->getHotColdVertices(this, 0.1, 0.1);
		partitions[i]->getColdEdges(0.1);
	}
	// print4debug(32);
	getAllHotVertices();
	computeEdgesMatchPartitions();
}

// 每个进程获取到全部的热点数据信息
void InstancePartitions::getAllHotVertices(){
	allHotVertices.clear();
	allHotVerticesScore.clear();
	InstanceIndexStart.clear();
	InstanceIndexEnd.clear();
	PartitionIndexStart.clear();
	PartitionIndexEnd.clear();
	PartitionIndexLen.clear();
	
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
	map<Edge, int> coldEdges2Partition;
	


	
}

// 只要边发生变化，则执行该操作(每次迭代之后必须要更新)
void Partition::getVerticesAndDegree(){
	set<uint32_t> t;
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