#include "Partition.h"
using namespace std;

extern int procid, numprocs;

InstancePartitions::InstancePartitions(uint32_t allparts, uint32_t numparts){
	this->allparts = allparts;
	this->numparts = numparts;
	for(int i = 0; i < numparts; i++){
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
	int numPartitionVertices[numparts];
	for(int i = 0; i < numparts; i++){
		numPartitionVertices[i] = partitions[i]->vertices.size();
	}
	int numInstanceVertices = 0;
	for(int i = 0; i < numparts; i++){
		numInstanceVertices += numPartitionVertices[i];
	}
	int sumVertices;
	MPI_Allreduce(&numInstanceVertices, &sumVertices, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	VRF = sumVertices / 1.0 / nvertices_global;
	// LOG(INFO) << "VRF: " << VRF << endl;
}

void InstancePartitions::getBalance(){
	int numPartitionEdges[numparts];
	for(int i = 0; i < numparts; i++){
		numPartitionEdges[i] = partitions[i]->edges.size();
	}
	int recvCounts[numprocs];
	MPI_Allgather(&numparts, 1, MPI_INT, recvCounts, 1, MPI_INT, MPI_COMM_WORLD);
	int rdisps[numprocs];
	rdisps[0] = 0;
	for(int i = 1; i < numprocs; i++){
		rdisps[i] = rdisps[i-1] + recvCounts[i-1];
	}
	int numAllPartitionsEdges[allparts];
	MPI_Allgatherv(numPartitionEdges, numparts, MPI_INT, \
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
	for(int i = 0; i < numparts; i++){
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
	for(int i = 0; i < numparts; i++){
		partitions[i]->getHotColdVertices(this, 0.3, 0.3);
		partitions[i]->getColdEdges(0.3);
	}
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
