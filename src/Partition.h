#ifndef PARTITION_H
#define PARTITION_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stdint.h>
#include <algorithm>
#include <mpi.h>
#include <glog/logging.h>
#include <cmath>

#include "units.h"

using namespace std;

class Vertex;
class Edge;
class Partition;
class InstancePartitions;

class Vertex{
public:
	uint32_t ver;
	// uint32_t degree;
};

class Edge{
public:
	Vertex src;
	Vertex dst;

	bool operator<(const Edge& e) const{
		if(this->src.ver < e.src.ver){
			return true;
		}else if(this->src.ver == e.src.ver){
			return this->dst.ver < e.dst.ver;
		}else{
			return false;
		}
	}
};

class Partition{
public:
	void getVerticesAndDegree();
	
	void getVertexScore(InstancePartitions* ins_partition);
	void getHotColdVertices(InstancePartitions* ins_partition, double hot, double cold); // 热点与冷点比例
	
	void getEdgeScore();
	void getColdEdges(double cold); // 冷边所占比例

public:
	vector<Edge> edges;
	set<uint32_t> vertices;

	vector<uint32_t> hotVertices;
	vector<uint32_t> coldVertices;
	vector<Edge> hotEdges;
	vector<Edge> coldEdges;

	map<uint32_t, uint32_t> vertexPartDegree; // 局部度信息
	map<uint32_t, double> vertexScore;        // 点的score
	map<Edge, double> edgeSocre;              // 边的score
};

// 每一个进程所拥有的全部partition 类
class InstancePartitions{
public:
	InstancePartitions(uint32_t allparts, uint32_t numparts);
	void getAllVerticesDegree();
	void getVRF();
	void getBalance();

	void InstanceInit();

	bool InstanceIteration(); 
	bool InstanceIteration(int times); 

public:
	vector<Partition*> partitions;
	uint32_t allparts;
	uint32_t numparts;
	uint32_t nedges_global;               // 全部边数
	uint32_t nvertices_global;            // 全部点数
	map<uint32_t, uint32_t> vertexDegree; // 每个进程保留全部点度信息

	double balance_RSD = 0;
	double balance_MAX_MIN = 0;
	double VRF = 0;
};

#endif