all:
	# mpicxx -o market market.cpp -L /usr/local/lib -lglog -I /usr/local/include/glog
	mpicxx -c units.cpp
	mpicxx -c io.cpp
	mpicxx -c Partition.cpp
	mpicxx -c market.cpp

	mpicxx -o market market.o Partition.o io.o units.o -L /usr/local/lib -lglog -I /usr/local/include/glog

run:
	mpirun -np 4 ./market ../datasets/wiki-vote.dat 100

clean:
	rm *.o market ../log/* ans.txt
	