// mpitest.cpp
#include <iostream>
#include <vector>
#include <map>
#include <mpi.h>

using namespace std;

int main(int argc, char *argv[])
{
	int myid, numprocs;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	MPI_Status status;
	double startTime = 0;
	double endTime = 0;
	startTime = MPI_Wtime();

	// 广播
	// MPI_Bcast
	/*
	int value = 1;
	while(value > 0){
		if(myid == 0){
			cin >> value;
		}
		MPI_Bcast(&value, 1, MPI_INT, 0, MPI_COMM_WORLD);
		cout << "Task: " << myid << " value: " << value << endl;
	}
	*/


	// 收集
	// MPI_Gather
	/*
	int sendArray[10];
	int *recvArray = (int*)malloc(numprocs*10*sizeof(int));
	for(int i = 0; i < 10; i++){
		sendArray[i] = myid * 10 + i;
	}
	MPI_Gather(sendArray, 10, MPI_INT, recvArray, 10, MPI_INT, 0, MPI_COMM_WORLD);
	if(myid == 0){
		for(int i = 0; i < 10*numprocs; i++){
			cout << "Task: " << myid << " value: " << recvArray[i] << endl;
		}
	}
	*/

	// MPI_Gatherv
	/*
	int sendArray[myid+1];
	for(int i = 0; i < myid+1; i++){
		sendArray[i] = myid * 10 + i;
	}
	int *recvArray = (int*)malloc(10*sizeof(int));
	int recvCounts[4];
	recvCounts[0] = 1;
	recvCounts[1] = 2;
	recvCounts[2] = 3;
	recvCounts[3] = 4;

	int displs[4];
	displs[0] = 0;
	for(int i = 1; i < 4; i++){
		displs[i] = displs[i-1] + recvCounts[i-1];
	}

	MPI_Gatherv(sendArray, myid+1, MPI_INT, recvArray, recvCounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
	if(myid == 0){
		for(int i = 0; i < 10; i++){
			cout << "Task: " << myid << " value: " << recvArray[i] << endl;
		}
	}
	*/

	// 散发
	// MPI_Scatter
	/*
	int sendArray[40];
	for(int i = 0; i < 40; i++){
		sendArray[i] = i + 1;
	}
	int recvArray[10];
	MPI_Scatter(sendArray, 10, MPI_INT, recvArray, 10, MPI_INT, 0, MPI_COMM_WORLD);
	for(int i = 0; i < 10; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	*/

	// MPI_Scatterv
	/*
	int sendArray[10];
	for(int i = 0; i < 10; i++){
		sendArray[i] = i + 1;
	}
	int sendCounts[4];
	sendCounts[0] = 1;
	sendCounts[1] = 2;
	sendCounts[2] = 3;
	sendCounts[3] = 4;

	int displs[4];
	displs[0] = 0;
	for(int i = 1; i < 4; i++){
		displs[i] = displs[i-1] + sendCounts[i-1];
	}

	int recvArray[myid+1];
	MPI_Scatterv(sendArray, sendCounts, displs, MPI_INT, recvArray, myid+1, MPI_INT, 0, MPI_COMM_WORLD);
	for(int i = 0; i < myid+1; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	*/

	// 组收集
	// MPI_Allgather
	/*
	int sendArray[10];
	for(int i = 0; i < 10; i++){
		sendArray[i] = myid * 10 + i;
	}
	int recvArray[40];
	MPI_Allgather(sendArray, 10, MPI_INT, recvArray, 10, MPI_INT, MPI_COMM_WORLD);
	for(int i = 0; i < 40; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	*/
	

	// MPI_Allgatherv
	
	int sendArray[myid+1];
	for(int i = 0; i < myid+1; i++){
		sendArray[i] = myid * 10 + i;
	}
	int recvCounts[4];
	recvCounts[0] = 1;
	recvCounts[1] = 2;
	recvCounts[2] = 3;
	recvCounts[3] = 4;
	int recvArray[10];
	int displs[4];
	displs[0] = 0;
	for(int i = 1; i < 4; i++){
		displs[i] = displs[i-1] + recvCounts[i-1];
	}

	MPI_Allgatherv(sendArray, myid+1, MPI_INT, recvArray, recvCounts, displs, MPI_INT, MPI_COMM_WORLD);

	for(int i = 0; i < 10; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	

	// 全互换
	// MPI_Alltoall
	/*
	int sendArray[40];
	for(int i = 0; i < 40; i++){
		sendArray[i] = myid;
	}
	int recvArray[40];
	MPI_Alltoall(sendArray, 10, MPI_INT, recvArray, 10, MPI_INT, MPI_COMM_WORLD);
	for(int i = 0; i < 40; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	*/

	// MPI_Alltoallv
	/*
	int sendArray[(myid+1) * 4];
	for(int i = 0; i < (myid+1) * 4; i++){
		sendArray[i] = myid;
	}
	int sendCounts[4];
	for(int i = 0; i < 4; i++){
		sendCounts[i] = myid + 1;
	}
	int sdispls[4];
	sdispls[0] = 0;
	for(int i = 1; i < 4; i++){
		sdispls[i] = sdispls[i-1] + sendCounts[i-1];
	}
	int recvArray[10];
	int recvCounts[4];
	recvCounts[0] = 1;
	recvCounts[1] = 2;
	recvCounts[2] = 3;
	recvCounts[3] = 4;
	int rdispls[4];
	rdispls[0] = 0;
	for(int i = 1; i < 4; i++){
		rdispls[i] = rdispls[i-1] + recvCounts[i-1];
	}

	MPI_Alltoallv(sendArray, sendCounts, sdispls, MPI_INT, recvArray, recvCounts, rdispls, MPI_INT, MPI_COMM_WORLD);

	for(int i = 0; i < 10; i++){
		cout << "Task: " << myid << " value: " << recvArray[i] << endl;
	}
	*/

	// 规约-最大值
	// MPI_Reduce
	/*
	int value = myid;
	int getValue;
	MPI_Reduce(&value, &getValue, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
	if(myid == 0){
		cout << "Task: " << myid << " value: " << value << " getValue: " << getValue << endl;
	}
	*/

	// MPI_Allreduce
	/*
	int value = myid;
	int getValue;
	MPI_Allreduce(&value, &getValue, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
	cout << "Task: " << myid << " value: " << value << " getValue: " << getValue << endl;
	*/

	// 规约-求和
	// MPI_Reduce
	/*
	int value[4];
	for(int i = 0; i < 4; i++){
		value[i] = myid * 10 + i;
	}
	int getValue[4];
	MPI_Reduce(value, getValue, 4, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if(myid == 0){
		for(int i = 0; i < 4; i++){
			cout << "Task: " << myid << " value: " << value[i] << " getValue: " << getValue[i] << endl;
		}
	}
	*/

	// MPI_Allreduce
	/*
	int value[4];
	for(int i = 0; i < 4; i++){
		value[i] = myid * 10 + i;
	}
	int getValue[4];
	MPI_Allreduce(value, getValue, 4, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	for(int i = 0; i < 4; i++){
		cout << "Task: " << myid << " value: " << value[i] << " getValue: " << getValue[i] << endl;
	}
	*/

	// MPI_Reduce_scatter
	/*
	int value[4];
	for(int i = 0; i < 4; i++){
		value[i] = myid * 10 + i;
	}
	int getValue[4];
	for(int i = 0; i < 4; i++){
		getValue[i] = -1;
	}
	int recvCounts[4];
	for(int i = 0; i < 4; i++){
		recvCounts[i] = 1;
	}
	MPI_Reduce_scatter(value, getValue, recvCounts, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	for(int i = 0; i < 4; i++){
		cout << "Task: " << myid << " value: " << value[i] << " getValue: " << getValue[i] << endl;
	}
	*/

	// MPI_Scan
	/*
	int value[4];
	for(int i = 0; i < 4; i++){
		value[i] = myid * 10 + i;
	}
	int getValue[4];
	for(int i = 0; i < 4; i++){
		getValue[i] = -1;
	}
	MPI_Scan(value, getValue, 4, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	for(int i = 0; i < 4; i++){
		cout << "Task: " << myid << " value: " << value[i] << " getValue: " << getValue[i] << endl;
	}
	*/
	
	

	MPI_Barrier(MPI_COMM_WORLD);
	endTime = MPI_Wtime();
	if(myid == 0){
		cout << "Tasks over" << endl;
		cout << "Cost time: " << endTime - startTime << " s." << endl;
	}
	MPI_Finalize();
	return 0;
}
