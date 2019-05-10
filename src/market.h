#ifndef MARKET_H
#define MARKET_H

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

int procid;
int numprocs;

bool debug = false;

#endif