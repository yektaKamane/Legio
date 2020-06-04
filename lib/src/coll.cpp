#include <mpi.h>
#include <mpi-ext.h>
#include <stdio.h>
#include <signal.h>
#include "comm_manipulation.h"
#include "configuration.h"
#include "adv_comm.h"
#include "multicomm.h"

extern Multicomm *cur_comms;
extern int VERBOSE;
extern char errstr[MPI_MAX_ERROR_STRING];
extern int len;

int MPI_Barrier(MPI_Comm comm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Barrier(translated->get_comm());
        else
            rc = PMPI_Barrier(comm);

        print_info("barrier", comm, rc);

        if(rc == MPI_SUCCESS || !flag)
            return rc;
        else
            replace_comm(translated);
    }
}

int MPI_Bcast(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
        {
            int root_rank;
            translate_ranks(root, translated, &root_rank);
            if(root_rank == MPI_UNDEFINED)
            {
                HANDLE_BCAST_FAIL(translated->get_comm());
            }
            rc = PMPI_Bcast(buffer, count, datatype, root_rank, translated->get_comm());
        }
        else
            rc = PMPI_Bcast(buffer, count, datatype, root, comm);
        bcast_handling:

        print_info("bcast", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
                return rc;
        }
        else
            return rc;
    }
}

int MPI_Allreduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, translated->get_comm());
        else
            rc = PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);

        print_info("allreduce", comm, rc);

        if(rc == MPI_SUCCESS || !flag)
            return rc;
        else
            replace_comm(translated);
    }
}

int MPI_Reduce(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
        {
            int root_rank;
            translate_ranks(root, translated, &root_rank);
            if(root_rank == MPI_UNDEFINED)
            {
                HANDLE_REDUCE_FAIL(translated.get_comm());
            }
            rc = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root_rank, translated->get_comm());
        }
        else
            rc = PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
        reduce_handling:

        print_info("reduce", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
                return rc;
        }
        else
            return rc;
    }
}

int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    while(1)
    {
        int rc, actual_root, total_size, fake_rank, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        MPI_Comm actual_comm;
        MPI_Comm_size(comm, &total_size);
        MPI_Comm_rank(comm, &fake_rank);
        if(flag)
        {
            actual_comm = translated->get_comm();
            translate_ranks(root, translated, &actual_root);
            if(actual_root == MPI_UNDEFINED)
            {
                HANDLE_GATHER_FAIL(actual_comm);
            }
        }
        else
        {
            actual_comm = comm;
            actual_root = root;
        }

        PERFORM_GATHER(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, actual_root, actual_comm, total_size, fake_rank, comm);

        gather_handling:

        print_info("gather", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
                return rc;
        }
        else
            return rc;
    }
}

int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)
{
    while(1)
    {
        int rc, actual_root, total_size, fake_rank, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        MPI_Comm actual_comm;
        MPI_Comm_size(comm, &total_size);
        MPI_Comm_rank(comm, &fake_rank);
        if(flag)
        {
            actual_comm = translated->get_comm();
            translate_ranks(root, translated, &actual_root);
            if(actual_root == MPI_UNDEFINED)
            {
                HANDLE_SCATTER_FAIL(actual_comm);
            }
        }
        else
        {
            actual_comm = comm;
            actual_root = root;
        }

        PERFORM_SCATTER(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, actual_root, actual_comm, total_size, fake_rank, comm);

        scatter_handling:
        
        print_info("scatter", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
                return rc;
        }
        else
            return rc;
    }
}

int MPI_Scan(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Scan(sendbuf, recvbuf, count, datatype, op, translated->get_comm());
        else
            rc = PMPI_Scan(sendbuf, recvbuf, count, datatype, op, comm);
        
        print_info("scan", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
                return rc;
        }
        else
            return rc;
    }
}