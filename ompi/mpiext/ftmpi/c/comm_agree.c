/*
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2015-2020 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ompi_config.h"

#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/group/group.h"
#include "ompi/proc/proc.h"
#include "ompi/op/op.h"

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

#if OMPI_BUILD_MPI_PROFILING
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_Comm_agree = PMPIX_Comm_agree
#endif
#define MPIX_Comm_agree PMPIX_Comm_agree
#endif

static const char FUNC_NAME[] = "MPIX_Comm_agree";


int MPIX_Comm_agree(MPI_Comm comm, int *flag)
{
    int rc = MPI_SUCCESS;
    ompi_group_t* acked;

    /* Argument checking */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        }
        if (NULL == flag) {
            rc = MPI_ERR_ARG;
        }
        OMPI_ERRHANDLER_CHECK(rc, comm, rc, FUNC_NAME);
    }

    ompi_comm_failure_get_acked_internal( comm, &acked );

    rc = comm->c_coll->coll_agree( flag,
                                   1,
                                   &ompi_mpi_int.dt,
                                   &ompi_mpi_op_band.op,
                                   &acked, false, /* Acked failures are ignored */
                                   (ompi_communicator_t*)comm,
                                   comm->c_coll->coll_agree_module);
    OBJ_RELEASE( acked );
    OMPI_ERRHANDLER_RETURN(rc, comm, rc, FUNC_NAME);
}


int MPIX_Comm_agree_failed(int *flag, int *num_agreed_failed, MPI_Comm comm)
{
    int rc = MPI_SUCCESS;

    /* Argument checking */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        }
        if (NULL == flag) {
            rc = MPI_ERR_ARG;
        }
        OMPI_ERRHANDLER_CHECK(rc, comm, rc, FUNC_NAME);
    }

    ompi_group_t* failed_group;
    opal_mutex_lock(&ompi_group_afp_mutex);
    ompi_group_intersection(comm->c_remote_group, ompi_group_all_failed_procs, failed_group);
    opal_mutex_unlock(&ompi_group_afp_mutex);

    do {
        rc = comm->c_coll->coll_agree( flag,
                                   1,
                                   &ompi_mpi_int.dt,
                                   &ompi_mpi_op_band.op,
                                   &failed_group, true,
                                   (ompi_communicator_t*)comm,
                                   comm->c_coll->coll_agree_module);
    } while(MPI_ERR_PROC_FAILED == rc);
    *num_agreed_failed = ompi_group_size(failed_group);
    OBJ_RELEASE( failed_group );

    OMPI_ERRHANDLER_RETURN(rc, comm, rc, FUNC_NAME);
}

