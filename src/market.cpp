#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <stdint.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <glog/logging.h>
#include <unistd.h>

#include "Partition.h"
#include "io.h"

using namespace std;

int procid, numprocs;

int main(int argc, char* argv[])
{
	google::InitGoogleLogging(argv[0]);
	FLAGS_logtostderr = false;
	FLAGS_alsologtostderr = true;
	FLAGS_log_prefix = false;
	FLAGS_log_dir = "../log";
	google::SetStderrLogging(google::GLOG_INFO);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    uint32_t allparts = atoi(argv[2]);
    uint32_t numparts = allparts / numprocs;
    if(procid == numprocs - 1){
    	numparts = allparts - numparts * (numprocs - 1);
    }
    // LOG(INFO) << "Task " << procid << " has parts " << numparts;

    InstancePartitions* ins_partition = new InstancePartitions(allparts, numparts);
	int ret = load_edges_uint32(ins_partition, argv[1]);
    if(ret){
    	LOG(INFO) << "load edges error.";
    	return 1;
    }

    // print4debug(ins_partition);
    ins_partition->InstanceInit();

    ins_partition->InstanceIteration();
    
    
    
    MPI_Finalize();
    google::ShutdownGoogleLogging();
	return 0;
}