#include "market.h"

using namespace std;

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
    uint32_t numparts[numprocs];
    uint32_t numpartsTemp = allparts / numprocs;
    for(int i = 0; i < numprocs - 1; i++){
        numparts[i] = numpartsTemp;
    }
    numparts[numprocs - 1] = allparts - numpartsTemp * (numprocs - 1);

    InstancePartitions* ins_partition = new InstancePartitions(allparts, numparts);
	int ret = load_edges_uint32(ins_partition, argv[1]);
    if(ret){
    	LOG(INFO) << "load edges error.";
    	return 1;
    }

    // 查看partitions是否正确
    if(debug){
        for(int i = 0; i < numprocs; i++){
            if(procid == i){
                cout << "Task: " << procid << endl;
                cout << "allparts: " << ins_partition->allparts << endl;
                cout << "numparts: ";
                for(int j = 0; j < ins_partition->numparts.size(); j++){
                    cout << numparts[j] << " ";
                }
                cout << endl;
                cout << "startpart: " << ins_partition->startpart << endl;
            }
            else{
                sleep(1);
            }
        }
        // printAllEdges4Debug(ins_partition);
    }

    ins_partition->InstanceInit();
    
    for(int i = 0; i < 100; i++){
        if(procid ==0){
            cout << "VRF: " << ins_partition->VRF << endl;
            cout << "Balance_RSD: " << ins_partition->balance_RSD << endl;
            cout << "Balance_MAX_MIN: " << ins_partition->balance_MAX_MIN << endl;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        int alledges = 0;
        for(int i = 0; i < ins_partition->partitions.size(); i++){
            alledges += ins_partition->partitions[i]->edges.size();
        }
        for(int j = 0; j < numprocs; j++){
            if(procid == j){
                cout << alledges << " ";
            }
            else{
                sleep(1);
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        cout << endl;
        ins_partition->InstanceIteration();
    }
    
    MPI_Finalize();
    google::ShutdownGoogleLogging();
	return 0;
}