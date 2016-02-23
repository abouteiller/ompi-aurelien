/*
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2013-2016 The University of Tennessee and The University
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
#include "ompi/proc/proc.h"
#include "ompi/errhandler/errhandler.h"

#if OPAL_HAVE_WEAK_SYMBOLS && OMPI_PROFILE_LAYER
#pragma weak MPIX_Comm_failure_ack = PMPIX_Comm_failure_ack
#endif

#if OMPI_PROFILE_LAYER
#include "ompi/mpiext/ftmpi/c/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

static const char FUNC_NAME[] = "MPIX_Comm_failure_ack";


int MPIX_Comm_failure_ack(MPI_Comm comm)
{
    int rc = MPI_SUCCESS;

    /* Argument checking */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_comm_invalid(comm)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        }
        OMPI_ERRHANDLER_CHECK(rc, comm, rc, FUNC_NAME);
    }

    rc = ompi_comm_failure_ack_internal( (ompi_communicator_t*)comm );
    OMPI_ERRHANDLER_RETURN(rc, comm, rc, FUNC_NAME);
}

int OMPI_Comm_failure_ack(MPI_Comm comm)
{
    return MPIX_Comm_failure_ack(comm);
}

