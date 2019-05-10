#include "io.h"
using namespace std;

// 加载格式为uint32_t的数据
int load_edges_uint32(InstancePartitions* ins_partition, char* file_name)
{
    FILE* input = fopen(file_name, "rb");
    if(input == NULL){
        LOG(ERROR) << "Task " << procid << " open input file error.";
        return 1;
    }
    fseek(input, 0L, SEEK_END);
    uint32_t file_size = ftell(input);
    fseek(input, 0L, SEEK_SET);

    // 获取每个进程需要读取的数据的指针区间
    uint32_t nedges_global = file_size / (2 * sizeof(uint32_t));
    ins_partition->nedges_global = nedges_global;
    uint32_t read_offset_start = 
          procid * 2 * sizeof(uint32_t) * (nedges_global / numprocs);
    uint32_t read_offset_end =
          (procid + 1) * 2 * sizeof(uint32_t) * (nedges_global / numprocs);
    if(procid == numprocs - 1){
        read_offset_end = 2 * sizeof(uint32_t) * nedges_global;
    }
    uint32_t nedges = (read_offset_end - read_offset_start) / 8;

    // LOG(INFO) << "Task " << procid << ", read_offset_start " << read_offset_start \
    // << ", read_offset_end " << read_offset_end << ", nedges_global " << nedges_global \
    // << ", nedges: " << nedges << endl;

    // 每个进程获取边集数据信息
    fseek(input, read_offset_start, SEEK_SET);
    uint32_t* gen_edges_read = (uint32_t*)malloc(2 * nedges * sizeof(uint32_t));
    if(gen_edges_read == NULL){
        LOG(ERROR) << "Task " <<procid << " malloc error.";
        return 1;
    }
    fread(gen_edges_read, nedges, 2 * sizeof(uint32_t), input);

    // 每个进程读取边信息，初始化相关量
    for(int i = 0, j = ins_partition->numparts[procid]; i < nedges * 2; i += 2, j++){
        Edge edge;
        edge.src.ver = (uint32_t)gen_edges_read[i];
        edge.dst.ver = (uint32_t)gen_edges_read[i+1];
        ins_partition->partitions[j%ins_partition->numparts[procid]]->edges.push_back(edge);

        // 初始化度相关信息
        if(ins_partition->vertexDegree.count(edge.src.ver) == 0){
            ins_partition->vertexDegree[edge.src.ver] = 1;
        }
        else{
            ins_partition->vertexDegree[edge.src.ver] += 1;
        }
        if(ins_partition->vertexDegree.count(edge.dst.ver) == 0){
            ins_partition->vertexDegree[edge.dst.ver] = 1;
        }
        else{
            ins_partition->vertexDegree[edge.dst.ver] += 1;
        }
    }
    free(gen_edges_read);
    fclose(input);
    return 0;
}

// 输出全部的边信息用于debug，现已不用
void printAllEdges4Debug(InstancePartitions* ins_partition)
{
    for(int i = 0; i < numprocs; i++){
        if(procid == i){
            for(int j = 0; j < ins_partition->partitions.size(); j++){
                Partition *t = ins_partition->partitions[j];
                for(int k = 0; k < t->edges.size(); k++){
                    cout << "Task: " << procid << " partitions: " << j << " edge: " \
                    << t->edges[k].src.ver << " " << t->edges[k].dst.ver << endl;
                }
            }
        }
        else{
            sleep(5);
        }
    }
}