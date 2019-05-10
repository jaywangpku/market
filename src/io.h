#ifndef IO_H
#define IO_H

#include <iostream>
#include <mpi.h>
#include <omp.h>
#include <stdint.h>
#include <string>
#include <cstdlib>
#include <glog/logging.h>
#include <unistd.h>
#include <cstdio>

#include "Partition.h"

using namespace std;

extern int procid, numprocs;
extern bool debug;

class InstancePartitions;

int load_edges_uint32(InstancePartitions* ins_partition, char* file_name);

void printAllEdges4Debug(InstancePartitions* ins_partition);

#endif