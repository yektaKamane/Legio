#include "mpi.h"
#include "mpi-ext.h"
#include "supported_comm.h"
#include "complex_comm.h"
#include "respawned_multicomm.h"
#include <functional>
#include <chrono>
#include <string>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <sstream>

SupportedComm::SupportedComm(MPI_Comm alias) {
    alias = alias;
}

RespawnedSupportedComm::RespawnedSupportedComm(MPI_Comm alias, std::vector<int> failed_ranks)
{
    alias = alias;
    failed_ranks = failed_ranks;
}

int RespawnedSupportedComm::size() {
    int size;
    MPI_Comm_size( get_alias() , &size);

    return size + failed_ranks.size();
}



int RespawnedSupportedComm::size() {
    int rank, i;
    int size = RespawnedSupportedComm::size();
    MPI_Comm_rank(get_alias(), &rank);

    // Reconstruct the original rank by iterating over the ranks of the original comunicator
    for (i = 0; i < size; i++) {
        if (std::find(failed_ranks.begin(), failed_ranks.end(), i) != failed_ranks.end()) {
            // Rank is failed, skip it
        }
        else {
            rank--;
        }
        if (rank == 0)
            break;
    }

    return i;
}


int RespawnedSupportedComm::get_failed_ranks_before(int rank) {
    return std::count_if(failed_ranks.begin(), failed_ranks.end(), [rank] (int failed_rank) {
        return failed_rank < rank;
    });
}