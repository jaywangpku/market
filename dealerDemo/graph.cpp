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

void load_graph_prepartition(char* filename, Graph& g)
{
	ifstream input;
	input.open(filename);
	
	int src;
	int dst;
	int part;
	while(input >> src >> dst >> part)
	{
		Edge e;
		e.src = src;
		e.dst = dst;

		g.subGraphs[part].edges.push_back(e);
		g.subGraphs[part].vertices.insert(e.src);
		g.subGraphs[part].vertices.insert(e.dst);
		if(g.subGraphs[part].vertex2SubDegree.count(e.src) == 0){
			g.subGraphs[part].vertex2SubDegree[e.src] = 1;
		}
		else{
			g.subGraphs[part].vertex2SubDegree[e.src] += 1;
		}
		if(g.subGraphs[part].vertex2SubDegree.count(e.dst) == 0){
			g.subGraphs[part].vertex2SubDegree[e.dst] = 1;
		}
		else{
			g.subGraphs[part].vertex2SubDegree[e.dst] += 1;
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
	cout << "maxSize " << maxSize << " minSize " << minSize << endl;
	return (maxSize - minSize) / 1.0 / (g.allEdges.size() / g.subGraphs.size());
}

void getVerticesAndDegree(subGraph& subg){
	subg.vertices.clear();
	subg.vertex2SubDegree.clear();
	for(int i = 0; i < subg.edges.size(); i++){
		subg.vertices.insert(subg.edges[i].src);
		subg.vertices.insert(subg.edges[i].dst);
		if(subg.vertex2SubDegree.count(subg.edges[i].src) == 0){
			subg.vertex2SubDegree[subg.edges[i].src] = 1;
		}
		else{
			subg.vertex2SubDegree[subg.edges[i].src] += 1;
		}
		if(subg.vertex2SubDegree.count(subg.edges[i].dst) == 0){
			subg.vertex2SubDegree[subg.edges[i].dst] = 1;
		}
		else{
			subg.vertex2SubDegree[subg.edges[i].dst] += 1;
		}
	}
}

void getSellEdge(Graph& g, int startDegree, int endDegree, double threshold){
	g.internalMarket.clear();
	for(int i = 0; i < g.subGraphs.size(); i++){
		int endEdges = g.subGraphs[i].edges.size() - 1;
		for(int j = 0; j <= endEdges; j++){
			int src = g.subGraphs[i].edges[j].src;
			int dst = g.subGraphs[i].edges[j].dst;
			int ver;
			if(g.vertex2AllDegree[src] < g.vertex2AllDegree[dst]){
				if(g.vertex2AllDegree[src] > 1){
					ver = src;
				}
				else{
					ver = dst;
				}
			}
			else{
				if(g.vertex2AllDegree[dst] > 1){
					ver = dst;
				}
				else{
					ver = src;
				}
			}
			if(g.vertex2AllDegree[ver] >= startDegree && g.vertex2AllDegree[ver] <= endDegree){
				if(g.subGraphs[i].vertex2SubDegree[ver] / 1.0 / g.vertex2AllDegree[ver] <= threshold){
					swap(g.subGraphs[i].edges[endEdges], g.subGraphs[i].edges[j]);
				    j--;
				    endEdges--;
				}
			}
		}
		g.subGraphs[i].money += g.subGraphs[i].edges.size() - endEdges - 1;
		// cout << i <<" "<< g.subGraphs[i].money << endl;
		// cout << i << " " << g.subGraphs[i].money << " " << g.subGraphs[i].edges.size() << " ";
		g.internalMarket.insert(g.internalMarket.end(), g.subGraphs[i].edges.begin() + endEdges + 1, g.subGraphs[i].edges.end());
		g.subGraphs[i].edges.erase(g.subGraphs[i].edges.begin() + endEdges + 1, g.subGraphs[i].edges.end());
		// cout << g.subGraphs[i].edges.size() << " ";
		getVerticesAndDegree(g.subGraphs[i]);
		// cout << g.subGraphs[i].vertices.size() << " " << g.subGraphs[i].vertex2SubDegree.size() << " " << endl;
	}
}

void arrangeInternalMarket(Graph& g){
	g.vertex2Edgesets.clear();
	g.internalMarket.insert(g.internalMarket.end(), g.leftover.begin(), g.leftover.end());
	for(int i = 0; i < g.internalMarket.size(); i++){
		int src = g.internalMarket[i].src;
		int dst = g.internalMarket[i].dst;
		int ver = g.vertex2AllDegree[src] < g.vertex2AllDegree[dst] ? src : dst;
		if(g.vertex2Edgesets.count(ver) == 0){
			EdgeSet edgeset;
			edgeset.edges.push_back(g.internalMarket[i]);
			edgeset.coreVertex = ver;
			g.vertex2Edgesets[ver] = edgeset;
		}
		else{
			g.vertex2Edgesets[ver].edges.push_back(g.internalMarket[i]);
		}
		g.vertex2Edgesets[ver].vertices.insert(src);
		g.vertex2Edgesets[ver].vertices.insert(dst);
	}
}

void buyEdges(Graph& g){
	g.leftover.clear();
	map<int, EdgeSet>::iterator iter;
	// cout << g.vertex2Edgesets.size() << endl;
	int m = 0, n = 0, k = 0;
	for(iter = g.vertex2Edgesets.begin(); iter != g.vertex2Edgesets.end(); iter++){
		bool selled = false;
		// 第一种情况，不完整小度点被购买
		for(int i = 0; i < g.subGraphs.size(); i++){
			if(g.subGraphs[i].vertices.find(iter->first) != g.subGraphs[i].vertices.end() && \
				g.subGraphs[i].money >= iter->second.edges.size()){

			
				g.subGraphs[i].edges.insert(g.subGraphs[i].edges.end(), iter->second.edges.begin(), iter->second.edges.end());
				g.subGraphs[i].money -= iter->second.edges.size();
				selled = true;
				break;
			}
		}
		// 第二种情况，完整小度点整体打包出售
		if(!selled && iter->second.edges.size() == g.vertex2AllDegree[iter->first]){
			// 选点交集最大的子图
			int part = -1;
			int maxt = 0;
			for(int i = 0; i < g.subGraphs.size(); i++){
				set<int> t;
				set_intersection(g.subGraphs[i].vertices.begin(), g.subGraphs[i].vertices.end(), \
					iter->second.vertices.begin(), iter->second.vertices.end(), inserter(t, t.begin()));
				if(t.size() > maxt && g.subGraphs[i].money >= iter->second.edges.size()){
					maxt = t.size();
					part = i;
				}
			}
			if(part >= 0 && g.subGraphs[part].money >= iter->second.edges.size()){
				g.subGraphs[part].edges.insert(g.subGraphs[part].edges.end(), iter->second.edges.begin(), iter->second.edges.end());
				g.subGraphs[part].money -= iter->second.edges.size();
				selled = true;
			}
		}
		// 第三种情况，留着下一轮卖
		if(!selled){
			for(int i = 0; i < iter->second.edges.size(); i++){
				g.leftover.push_back(iter->second.edges[i]);
			}
		}
	}
	for(int i = 0; i < g.subGraphs.size(); i++){
		getVerticesAndDegree(g.subGraphs[i]);
	}
}

// 清仓，以边为单位来卖
void clearance(Graph& g){
	// 第一轮贪心分配
	// for(int i = 0; i < g.subGraphs.size(); i++){
	// 	if(g.subGraphs[i].money > 0){
	// 		for(int j = 0; j < g.leftover.size(); j++){

	// 		}
	// 	}
	// }
	// 第二轮随机分配
	for(int i = 0; i < g.subGraphs.size(); i++){
		while(g.subGraphs[i].money > 0){
			g.subGraphs[i].edges.push_back(g.leftover[g.leftover.size()-1]);
			g.subGraphs[i].money -= 1;
			g.leftover.pop_back();
		}
	}
	for(int i = 0; i < g.subGraphs.size(); i++){
		getVerticesAndDegree(g.subGraphs[i]);
	}
}

// 获取需要重新经过greedy的边
void getReGreedyEdges(Graph& g, double k){
	g.reGreedyEdges.clear();
	for(int i = 0; i < g.subGraphs.size(); i++){
		random_shuffle(g.subGraphs[i].edges.begin(), g.subGraphs[i].edges.end());
		g.reGreedyEdges.insert(g.reGreedyEdges.end(), g.subGraphs[i].edges.begin() + g.subGraphs[i].edges.size()*k, g.subGraphs[i].edges.end());
		int edgesNUM = g.subGraphs[i].edges.size();
		g.subGraphs[i].edges.erase(g.subGraphs[i].edges.begin() + g.subGraphs[i].edges.size()*k, g.subGraphs[i].edges.end());
		g.subGraphs[i].money += edgesNUM - g.subGraphs[i].edges.size();
		getVerticesAndDegree(g.subGraphs[i]);
	}
}

// 从集合中选出最小的k个
set<int> sellectKsmallPart(Graph& g, set<int> parts, int k){
	map<int, int> tm; // 装载过程就直接按照key排好序了
	set<int>::iterator iter;
	for(iter = parts.begin(); iter != parts.end(); iter++){
		tm[g.subGraphs[*iter].edges.size()] = *iter;
	}
	set<int> ans;
	map<int, int>::iterator it = tm.begin();
	for(int i = 0; i < k; i++){
		ans.insert(it->second);
		it++;
	}
	return ans;
}

// 贪心随机分配过程
void greedySingleRandom(Graph& g){
	random_shuffle(g.reGreedyEdges.begin(), g.reGreedyEdges.end());
	// 对每一条边采用贪心分配策略
	for(int i = 0; i < g.reGreedyEdges.size(); i++){
		int src = g.reGreedyEdges[i].src;
		int dst = g.reGreedyEdges[i].dst;
		set<int> srcPart;
		set<int> dstPart;
		for(int j = 0; j < g.subGraphs.size(); j++){
			if(g.subGraphs[j].vertices.count(src) > 0){
				srcPart.insert(j);
			}
			if(g.subGraphs[j].vertices.count(dst) > 0){
				dstPart.insert(j);
			}
		}
		
		set<int> parts;
		// src 和 dst 均没出现过
		// 可以留下来放在之后再分配 ?
		if(srcPart.size() == 0 && dstPart.size() == 0){
			set<int> partTemp;
			for(int i = 0; i < g.subGraphs.size(); i++){
				partTemp.insert(i);
			}
			parts = sellectKsmallPart(g, partTemp, 1);
		}
		// src没有出现过dst出现过
		else if(srcPart.size() == 0 && dstPart.size() != 0){
			parts = sellectKsmallPart(g, dstPart, 1);
		}
		// src出现过dst没有出现过
		else if(srcPart.size() != 0 && dstPart.size() == 0){
			parts = sellectKsmallPart(g, srcPart, 1);
		}
		// src和dst均出现过
		else{
			set<int> intersection;
			set<int> convergence;
			set_intersection(srcPart.begin(), srcPart.end(), dstPart.begin(), dstPart.end(), inserter(intersection, intersection.begin()));
			set_union(srcPart.begin(), srcPart.end(), dstPart.begin(), dstPart.end(), inserter(convergence, convergence.begin()));
			// 有交集
			if(intersection.size() > 0){
				parts = sellectKsmallPart(g, intersection, 1);
			}
			// 无交集
			else{
				// 放到src对应的part中
				if(g.vertex2AllDegree[src] < g.vertex2AllDegree[dst]){
					parts = sellectKsmallPart(g, srcPart, 1);
				}
				// 放到dst对应的part中
				else{
					parts = sellectKsmallPart(g, dstPart, 1);
				}
			}
		}
		int part = *parts.begin();
		g.subGraphs[part].edges.push_back(g.reGreedyEdges[i]);
		g.subGraphs[part].vertices.insert(src);
		g.subGraphs[part].vertices.insert(dst);
		if(g.subGraphs[part].vertex2SubDegree.count(src) == 0){
			g.subGraphs[part].vertex2SubDegree[src] = 1;
		}
		else{
			g.subGraphs[part].vertex2SubDegree[src] += 1;
		}
		if(g.subGraphs[part].vertex2SubDegree.count(dst) == 0){
			g.subGraphs[part].vertex2SubDegree[dst] = 1;
		}
		else{
			g.subGraphs[part].vertex2SubDegree[dst] += 1;
		}
	}
	g.reGreedyEdges.clear();
}