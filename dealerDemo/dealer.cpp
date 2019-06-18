#include "dealer.h"

int main(int argc, char *argv[])
{
	srand((unsigned int)time(0));

	Graph g;
	int allparts = atoi(argv[2]);
	for(int i = 0; i < allparts; i++){
		subGraph subgraph;
		g.subGraphs.push_back(subgraph);
	}
	load_graph(argv[1], g);
	// load_graph_prepartition(argv[1], g);
	
	cout << getVRF(g) << endl;
	
	int k = 0;
	while(1){
		getReGreedyEdges(g, 0.8);
		greedySingleRandom(g);
		int alledges = 0;
		for(int i = 0; i < g.subGraphs.size(); i++){
			alledges += g.subGraphs[i].edges.size();
		}
		cout << ++k << " " << getBalance(g) << " " << getVRF(g) << endl << endl; 
	}

	return 0;
}
