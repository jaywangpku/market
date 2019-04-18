#include <iostream>
#include <stdint.h>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <set>
#include <cstdio>
#include <map>

using namespace std;

int main(int argc, char* argv[])
{
	FILE* input = fopen(argv[1], "rb");

	fseek(input, 0L, SEEK_END);
    uint32_t file_size = ftell(input);
    fseek(input, 0L, SEEK_SET);

    uint32_t nedges = file_size / 8;
    uint32_t* gen_edges_read = (uint32_t*)malloc(2 * nedges * sizeof(uint32_t));
    fread(gen_edges_read, nedges, 2 * sizeof(uint32_t), input);

    int edges = 0;
    set<uint32_t> vertices;
    map<uint32_t, uint32_t> vertexDegree;
    for(int i = 0; i < nedges * 2; i += 2){
    	uint32_t src = (uint32_t)gen_edges_read[i];
    	uint32_t dst = (uint32_t)gen_edges_read[i+1];
    	vertices.insert(src);
    	vertices.insert(dst);
        edges++;
        if(vertexDegree.count(src) == 0){
        	vertexDegree[src] = 1;
        }
        else{
        	vertexDegree[src] += 1;
        }

        // 6110 just include src but not dst
        if(vertexDegree.count(dst) == 0){
        	vertexDegree[dst] = 1;
        }
        else{
        	vertexDegree[dst] += 1;
        }

    }

    cout << "Edges: " << edges << endl;
    cout << "Vertices: " << vertices.size() << endl;

    // 6110 just include src but not dst
    cout << "Map size: " << vertexDegree.size() << endl;

	return 0;
}