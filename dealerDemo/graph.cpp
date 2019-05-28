#include "graph.h"

void load_graph(char* filename, Graph& g)
{
	ifstream input;
	input.open(filename);
	
	int src;
	int dst;
	int i = 0;
	while(input >> src >> dst)
	{
		Edge e;
		e.src = src;
		e.dst = dst;

		g.subGraphs[i].edges.push_back(e);
		g.subGraphs[i].vertices.insert(e.src);
		g.subGraphs[i].vertices.insert(e.dst);
		if(g.subGraphs[i].vertex2SubDegree.count(e.src) == 0){
			g.subGraphs[i].vertex2SubDegree[e.src] = 1;
		}
		else{
			g.subGraphs[i].vertex2SubDegree[e.src] += 1;
		}
		if(g.subGraphs[i].vertex2SubDegree.count(e.dst) == 0){
			g.subGraphs[i].vertex2SubDegree[e.dst] = 1;
		}
		else{
			g.subGraphs[i].vertex2SubDegree[e.dst] += 1;
		}

		g.allEdges.push_back(e);
		g.allVertices.insert(e.src);
		g.allVertices.insert(e.dst);
		if(g.vertex2AllDegree.count(e.src) == 0){
			g.vertex2AllDegree[e.src] = 1;
		}
		else{
			g.vertex2AllDegree[e.src] += 1;
		}
		if(g.vertex2AllDegree.count(e.dst) == 0){
			g.vertex2AllDegree[e.dst] = 1;
		}
		else{
			g.vertex2AllDegree[e.dst] += 1;
		}

		i = (i+1)%g.subGraphs.size();
	}
}

double getVRF(Graph& g){
	int vertices = 0;
	for(int i = 0; i < g.subGraphs.size(); i++){
		vertices += g.subGraphs[i].vertices.size();
	}
	return vertices / 1.0 / g.allVertices.size();
}

double getBalance(Graph& g){
	int maxSize = 0;
	int minSize = 100000000;
	for(int i = 0; i < g.subGraphs.size(); i++){
		if(maxSize < g.subGraphs[i].edges.size()){
			maxSize = g.subGraphs[i].edges.size();
		}
		if(minSize > g.subGraphs[i].edges.size()){
			minSize = g.subGraphs[i].edges.size();
		}
	}
	return (maxSize - minSize) / 1.0 / (g.allEdges.size() / g.subGraphs.size());
}

