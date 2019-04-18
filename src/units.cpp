#include "units.h"
using namespace std;

void QuickSort(vector<uint32_t>& vertices, map<uint32_t, double>& vertexScore, int start, int end){
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
    QuickSort(vertices, vertexScore, start, startTemp-1);
    QuickSort(vertices, vertexScore, startTemp+1, end);
}