all:
	g++ -c -std=c++11 dealer.cpp
	g++ -c -std=c++11 graph.cpp

	g++ -o dealer dealer.o graph.o

run:
	./dealer ../datasets/Wiki-Vote.txt 100

runhdrf:
	./dealer ../datasets/Wiki-Vote-hdrf.txt 100