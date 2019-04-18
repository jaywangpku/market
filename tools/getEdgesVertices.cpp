#include <iostream>
#include <fstream>
#include <set>
#include <stdint.h>

using namespace std;

int main(int argc, char* argv[])
{
	ifstream input;
	input.open(argv[1]);
	
	uint64_t src, dst;
	set<uint64_t> vertices;
	uint64_t edges = 0;
	while(input >> src >> dst){
		edges++;
		vertices.insert(src);
		vertices.insert(dst);
	}
	cout << "Input file: " << argv[1] << endl;
	cout << "Vertices: " << vertices.size() << endl;
	cout << "Edges: " << edges << endl;

	return 0;
}