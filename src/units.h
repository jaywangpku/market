#ifndef UNITS_H
#define UNITS_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <stdint.h>
#include <algorithm>
#include <mpi.h>
#include <glog/logging.h>
#include <cmath>
#include <random>
#include <ctime>

using namespace std;

void QuickSort(vector<uint32_t>& vertices, map<uint32_t, double>& vertexScore, int start, int end);

#endif