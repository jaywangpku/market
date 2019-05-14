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

#include "Partition.h"

using namespace std;

// 两个头文件互相包含，需要加前置声明解决
class Vertex;
class Edge;
class Partition;
class InstancePartitions;

void QuickSortVertex(vector<uint32_t>& vertices, map<uint32_t, double>& vertexScore, int start, int end);
void QuickSortEdge(vector<Edge>& edges, map<Edge, double>& edgeScore, int start, int end);
void QuickSortEdgePart(vector<Edge>& edges, map<Edge, int>& edgePart, int start, int end);

#endif