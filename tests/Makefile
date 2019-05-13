all:
	mpicxx -c -std=c++11 mpitest.cpp
	mpicxx -o mpitest mpitest.o

run:
	mpirun -np 4 ./mpitest

clean:
	rm *.o mpitest
	