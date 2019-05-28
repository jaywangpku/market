#include "dealer.h"

int main(int argc, char *argv[])
{
	Graph g;
	int allparts = atoi(argv[2]);
	for(int i = 0; i < allparts; i++){
		subGraph subgraph;
		g.subGraphs.push_back(subgraph);
	}
	load_graph(argv[1], g);

	// 代理商策略
	getSellEdge(g, 1, 10, 0.3);
	arrangeInternalMarket(g);
	buyEdges(g);





	return 0;
}