#include "units.h"
using namespace std;

void QuickSortVertex(vector<uint32_t>& vertices, map<uint32_t, double>& vertexScore, int start, int end){
    if(start >= end){
        return;
    }
	int rand = start; 
    uint32_t temp = vertices[start];
    int startTemp = start;
    int endTemp = end;
    bool flag = false;
    while(startTemp < endTemp){
        if(flag){
            while(vertexScore[vertices[startTemp]] <= vertexScore[temp] && startTemp < endTemp){
                startTemp++;
            }
            if(startTemp < endTemp){
                vertices[rand] = vertices[startTemp];
                rand = startTemp;
            }
            flag = false;
        }
        else{
            while(vertexScore[vertices[endTemp]] >= vertexScore[temp] && startTemp < endTemp){
                endTemp--;
            }
            if(startTemp < endTemp){
                vertices[rand] = vertices[endTemp];
                rand = endTemp;
            }
            flag = true;
        }
    }
    vertices[startTemp] = temp;
    QuickSortVertex(vertices, vertexScore, start, startTemp-1);
    QuickSortVertex(vertices, vertexScore, startTemp+1, end);
}

void QuickSortEdge(vector<Edge>& edges, map<Edge, double>& edgeScore, int start, int end){
    if(start >= end){
        return;
    }
    int rand = start; 
    Edge temp = edges[start];
    int startTemp = start;
    int endTemp = end;
    bool flag = false;
    while(startTemp < endTemp){
        if(flag){
            while(edgeScore[edges[startTemp]] <= edgeScore[temp] && startTemp < endTemp){
                startTemp++;
            }
            if(startTemp < endTemp){
                edges[rand] = edges[startTemp];
                rand = startTemp;
            }
            flag = false;
        }
        else{
            while(edgeScore[edges[endTemp]] >= edgeScore[temp] && startTemp < endTemp){
                endTemp--;
            }
            if(startTemp < endTemp){
                edges[rand] = edges[endTemp];
                rand = endTemp;
            }
            flag = true;
        }
    }
    edges[startTemp] = temp;
    QuickSortEdge(edges, edgeScore, start, startTemp-1);
    QuickSortEdge(edges, edgeScore, startTemp+1, end);
}