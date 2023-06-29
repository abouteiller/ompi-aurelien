/*
 * Copyright (c) 2023      The University of Tennessee and The University
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

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

#if OMPI_BUILD_MPI_PROFILING
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_Session_revoke = PMPIX_Session_revoke
#endif
#define MPIX_Session_revoke PMPIX_Session_revoke
#endif

static const char FUNC_NAME[] = "MPIX_Session_revoke";


int MPIX_Session_revoke(MPI_Session session)
{
    int rc = MPI_SUCCESS;

    /* Argument checking */
    if (MPI_PARAM_CHECK) {
        OMPI_ERR_INIT_FINALIZE(FUNC_NAME);
        if (ompi_instance_invalid(session)) {
            return OMPI_ERRHANDLER_INVOKE(MPI_COMM_WORLD, MPI_ERR_COMM, FUNC_NAME);
        }
        OMPI_ERRHANDLER_CHECK(rc, session, rc, FUNC_NAME);
    }

    rc = ompi_instance_revoke_comms(session);
    OMPI_ERRHANDLER_RETURN(rc, session, rc, FUNC_NAME);
}

