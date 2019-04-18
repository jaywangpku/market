all:
	g++ -o changeFormat changeFormat.cpp
	g++ -o getEdgesVertices getEdgesVertices.cpp
	g++ -o getDegree getDegree.cpp

RUNchangeFormat:
	./changeFormat ../datasets/Wiki-Vote.txt ../datasets/wiki-vote.dat

RUNgetEdgesVertices:
	./getEdgesVertices ../datasets/Wiki-Vote.txt

RUNgetDegree:
	./getDegree ../datasets/wiki-vote.dat

clean:
	rm changeFormat getEdgesVertices getDegree