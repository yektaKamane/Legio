#include <mpi.h>
#include <mpi-ext.h>
#include <stdio.h>
#include <signal.h>
#include "comm_manipulation.h"
#include "configuration.h"
#include "adv_comm.h"
#include "multicomm.h"


int VERBOSE = 1;

char errstr[MPI_MAX_ERROR_STRING];
int len;
Multicomm *cur_comms;

int MPI_Init(int* argc, char *** argv)
{
    int rc = PMPI_Init(argc, argv);
    initialization();
    return rc;
    /*
    int provided;
    return MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &provided);
    */
}

int MPI_Init_thread(int* argc, char *** argv, int required, int* provided)
{
    int rc = PMPI_Init_thread(argc, argv, required, provided);
    initialization();
    //kalive_thread();
    return rc;
}

int MPI_Finalize()
{
    MPI_Barrier(MPI_COMM_WORLD);
    //kill_kalive_thread();
    finalization();
    //return PMPI_Finalize();
    return MPI_SUCCESS;
}

int MPI_Abort(MPI_Comm comm, int errorcode)
{
    int rc, flag;
    cur_comms->part_of(comm, &flag);
    AdvComm* translated = cur_comms->translate_into_complex(comm);
    if(flag)
        rc = PMPI_Abort(translated->get_comm(), errorcode);
    else
        rc = PMPI_Abort(comm, errorcode);
    
    print_info("abort", comm, rc);

    return rc;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Comm_dup(translated->get_comm(), newcomm);
        else
            rc = PMPI_Comm_dup(comm, newcomm);
        
        print_info("comm_dup", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
            {
                MPI_Comm_set_errhandler(*newcomm, MPI_ERRORS_RETURN);
                bool result = add_comm(*newcomm);
                if(result)
                    return rc;
            }
        }
        else
            return rc;
    }
}

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        if(flag)
        {
            int rank;
            MPI_Group_rank(group, &rank);
            rank = (rank != MPI_UNDEFINED);
            MPI_Comm temp;
            rc = MPI_Comm_split(comm, rank, 0, &temp);
            if(rank)
                *newcomm = temp;
            else
            {
                MPI_Comm_disconnect(&temp);
                *newcomm = MPI_COMM_NULL;
            }
            return rc;
        }
        else
            return PMPI_Comm_create(comm, group, newcomm);
    }
}
/*
int MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm *newcomm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
        {
            MPI_Group new_group, failed_group;
            MPIX_Comm_failure_get_acked(comm, &failed_group);
            MPI_Group_difference(group, failed_group, &new_group);
            rc = PMPI_Comm_create_group(translated->get_comm(), new_group, tag, newcomm);
        }
        else
            rc = PMPI_Comm_create_group(comm, group, tag, newcomm);
        
        print_info("comm_create_group", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
            {
                MPI_Comm_set_errhandler(*newcomm, MPI_ERRORS_RETURN);
                cur_comms->add_comm(*newcomm);
                return rc;
            }
        }
        else
            return rc;
    }
}
*/

int MPI_Comm_disconnect(MPI_Comm * comm)
{
    std::function<int(MPI_Comm*)> func = [](MPI_Comm * a){return PMPI_Comm_disconnect(a);};
    cur_comms->remove(*comm, func);
    func(comm);
    return MPI_SUCCESS;
}

int MPI_Comm_free(MPI_Comm* comm)
{
    std::function<int(MPI_Comm*)> func = [](MPI_Comm * a){return PMPI_Comm_free(a);};
    cur_comms->remove(*comm, func);
    func(comm);
    return MPI_SUCCESS;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm* newcomm)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Comm_split(translated->get_comm(), color, key, newcomm);
        else
            rc = PMPI_Comm_split(comm, color, key, newcomm);
        
        print_info("comm_split", comm, rc);

        if(flag)
        {
            agree_and_eventually_replace(&rc, translated);
            if(rc == MPI_SUCCESS)
            {
                MPI_Comm_set_errhandler(*newcomm, MPI_ERRORS_RETURN);
                bool result = add_comm(*newcomm);
                if(result)
                    return rc;
            }
        }
        else
            return rc;
    }
}

int MPI_Comm_set_info(MPI_Comm comm, MPI_Info info)
{
    while(1)
    {
        int rc, flag;
        cur_comms->part_of(comm, &flag);
        AdvComm* translated = cur_comms->translate_into_complex(comm);
        if(flag)
            rc = PMPI_Comm_set_info(translated->get_comm(), info);
        else
            rc = PMPI_Comm_set_info(comm, info);
        
        print_info("comm_set_info", comm, rc);

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


int MPI_Comm_get_info(MPI_Comm comm, MPI_Info * info_used)
{
    int rc, flag;
    cur_comms->part_of(comm, &flag);
    AdvComm* translated = cur_comms->translate_into_complex(comm);
    if(flag)
        rc = PMPI_Comm_get_info(translated->get_comm(), info_used);
    else
        rc = PMPI_Comm_get_info(comm, info_used);
    
    print_info("comm_get_info", comm, rc);

    return rc;
}