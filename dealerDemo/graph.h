#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>

using namespace std;

struct Edge{
	int src;
	int dst;
};

struct subGraph{
	vector<Edge> edges;
	set<int> vertices;
	map<int, int> vertex2SubDegree; // 局部度信息

};

struct Graph{
	vector<subGraph> subGraphs;

	vector<Edge> allEdges;
	set<int> allVertices;
	map<int, int> vertex2AllDegree; // 全局度信息

};

void load_graph(char* filename, Graph& g);

double getVRF(Graph& g);
double getBalance(Graph& g);

#endif