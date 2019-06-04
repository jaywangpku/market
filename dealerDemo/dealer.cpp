#include "dealer.h"

int main(int argc, char *argv[])
{
	Graph g;
	int allparts = atoi(argv[2]);
	for(int i = 0; i < allparts; i++){
		subGraph subgraph;
		g.subGraphs.push_back(subgraph);
	}
	// load_graph(argv[1], g);
	load_graph_prepartition(argv[1], g);
	
	cout << getVRF(g) << endl; 

	// 代理商策略
	// for(int i = 0; i < 1000; i+=10){
	// 	getSellEdge(g, i, i + 10, 0.3);
	// 	arrangeInternalMarket(g);
	// 	buyEdges(g);

	// 	int alledges = 0;
	// 	for(int i = 0; i < g.subGraphs.size(); i++){
	// 		alledges += g.subGraphs[i].edges.size();
	// 	}


	// 	cout <<alledges<<" "<< getVRF(g) <<" "<< getBalance(g)<< endl; 
	// }

	for(int i = 0; i < 100; i+=1){
			getSellEdge(g, i+0, i+1, 1);
			arrangeInternalMarket(g);
			buyEdges(g);
			clearance(g);
			int alledges = 0;
			for(int i = 0; i < g.subGraphs.size(); i++){
				alledges += g.subGraphs[i].edges.size();
			}
			cout <<alledges<<" "<< getVRF(g) <<" "<< getBalance(g)<< endl; 
	}

	return 0;
}