#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <ctime>

using namespace std;

struct Edge{
	int src;
	int dst;
};

struct EdgeSet{
	vector<Edge> edges;
	set<int> vertices;
	int coreVertex;
};

struct subGraph{
	vector<Edge> edges;
	// 边集改变，这两个要更新
	set<int> vertices;
	map<int, int> vertex2SubDegree; // 局部度信息

	int money = 10;

};

struct Graph{
	vector<subGraph> subGraphs;

	vector<Edge> allEdges;
	set<int> allVertices;
	map<int, int> vertex2AllDegree; // 全局度信息

	vector<Edge> internalMarket;
	map<int, EdgeSet> vertex2Edgesets;   // 按边集组织的内部市场
	vector<Edge> leftover;               // 没有卖完的边

	vector<Edge> reGreedyEdges;          // 需要重新经过greedy的边

};

void load_graph(char* filename, Graph& g);
void load_graph_prepartition(char* filename, Graph& g);

double getVRF(Graph& g);
double getBalance(Graph& g);

void getVerticesAndDegree(subGraph& subg);
void getSellEdge(Graph& g, int startDegree, int endDegree, double threshold);
void arrangeInternalMarket(Graph& g);
void buyEdges(Graph& g);
void clearance(Graph& g);

void getReGreedyEdges(Graph& g, double k);
void greedySingleRandom(Graph& g);
set<int> sellectKsmallPart(Graph& g, set<int> parts, int k);

#endif