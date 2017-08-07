/*
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2014-2016 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ompi_config.h"

#include "ompi/mpi/c/bindings.h"
#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/request/request.h"
#include "ompi/proc/proc.h"
#include "ompi/op/op.h"

#if OPAL_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak MPIX_Comm_iagree = PMPIX_Comm_iagree
#endif

#if OMPI_PROFILE_LAYER
#include "ompi/mpiext/ftmpi/c/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

static const char FUNC_NAME[] = "MPIX_Comm_iagree";


int MPIX_Comm_iagree(MPI_Comm comm, int *flag, MPI_Request *request)
{
    int rc = MPI_SUCCESS;
    ompi_group_t* acked;

    /* Argument checking */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        } else if (request == NULL) {
            rc = MPI_ERR_REQUEST;
        }
        OMPI_ERRHANDLER_CHECK(rc, comm, rc, FUNC_NAME);
    }

    ompi_comm_failure_get_acked_internal( comm, &acked );
    rc = comm->c_coll->coll_iagreement( flag,
                                        1,
                                        &ompi_mpi_int.dt,
                                        &ompi_mpi_op_band.op,
                                        &acked, false,
                                        (ompi_communicator_t*)comm,
                                        request,
                                        comm->c_coll->coll_iagreement_module);
    OMPI_ERRHANDLER_RETURN(rc, comm, rc, FUNC_NAME);
}

int OMPI_Comm_iagree(MPI_Comm comm, int *flag, MPI_Request *request)
{
    return MPIX_Comm_iagree(comm, flag, request);
}

