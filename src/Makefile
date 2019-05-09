all:
	# mpicxx -o market market.cpp -L /usr/local/lib -lglog -I /usr/local/include/glog
	mpicxx -c -std=c++11 Partition.cpp
	mpicxx -c -std=c++11 market.cpp
	mpicxx -c -std=c++11 units.cpp
	mpicxx -c -std=c++11 io.cpp

	mpicxx -o market market.o Partition.o io.o units.o -L /usr/local/lib -lglog -I /usr/local/include/glog

run:
	mpirun -np 4 ./market ../datasets/wiki-vote.dat 100

clean:
	rm *.o market ../log/* ans.txt
	