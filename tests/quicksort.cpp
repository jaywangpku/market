#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <ctime>

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
    
    for(int i = 0; i < vertices.size(); i++){
        cout << vertices[i] << " ";
    }
    cout << endl;
    
    QuickSort(vertices, vertexScore, start, startTemp-1);
    QuickSort(vertices, vertexScore, startTemp+1, end);
}

int main()
{
    vector<uint32_t> vertices;
    map<uint32_t, double> vertexScore;
    
    vertices.push_back(9);
    vertices.push_back(7);
    vertices.push_back(11);
    vertices.push_back(13);
    vertices.push_back(4);
    vertices.push_back(8);
    vertices.push_back(1);
    vertices.push_back(19);
    vertices.push_back(2);
    vertices.push_back(90);
    
    vertexScore[9] = 100;
    vertexScore[7] = 99;
    vertexScore[11] = 98;
    vertexScore[13] = 97;
    vertexScore[4] = 96;
    vertexScore[8] = 110;
    vertexScore[1] = 111;
    vertexScore[19] = 112;
    vertexScore[2] = 113;
    vertexScore[90] = 114;
    
    QuickSort(vertices, vertexScore, 0, vertices.size()-1);
    for(int i = 0; i < vertices.size(); i++){
        cout << vertices[i] << endl;
    }
    return 0;
}
