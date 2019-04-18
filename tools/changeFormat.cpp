#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <cstdio>
#include <fstream>

using namespace std;

int main(int argc, char* argv[])
{
	if(argc < 3){
		cout << "Input error!" << endl;
		cout << "Input should be: " << endl;
		cout << "./app inputfile outputfile" << endl;
		return 1;
	}
	else{
		cout << "Input file is: " << argv[1] << endl;
		cout << "Output file is: " << argv[2] << endl;
	}
	
	ifstream input;
	ofstream output;
	input.open(argv[1]);
	output.open(argv[2], ios::binary);

	uint32_t src, dst;
	while(input >> src >> dst){
		output.write((char *)&src, sizeof(uint32_t));
		output.write((char *)&dst, sizeof(uint32_t));
	}
	return 0;
}