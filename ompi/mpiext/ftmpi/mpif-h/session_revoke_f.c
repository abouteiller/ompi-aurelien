/*
 * Copyright (c) 2023      The University of Tennessee and the University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ompi_config.h"

#include "ompi/mpi/fortran/mpif-h/bindings.h"
#include "ompi/mpi/fortran/base/constants.h"
#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"
#include "ompi/mpiext/ftmpi/mpif-h/prototypes_mpi.h"

#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak PMPIX_SESSION_REVOKE = ompix_session_revoke_f
#pragma weak pmpix_session_revoke = ompix_session_revoke_f
#pragma weak pmpix_session_revoke_ = ompix_session_revoke_f
#pragma weak pmpix_session_revoke__ = ompix_session_revoke_f
#pragma weak PMPIX_Session_revoke_f = ompix_session_revoke_f
#pragma weak PMPIX_Session_revoke_f08 = ompix_session_revoke_f

#pragma weak MPIX_SESSION_REVOKE = ompix_session_revoke_f
#pragma weak mpix_session_revoke = ompix_session_revoke_f
#pragma weak mpix_session_revoke_ = ompix_session_revoke_f
#pragma weak mpix_session_revoke__ = ompix_session_revoke_f
#pragma weak MPIX_Session_revoke_f = ompix_session_revoke_f
#pragma weak MPIX_Session_revoke_f08 = ompix_session_revoke_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(PMPIX_SESSION_REVOKE,
                        pmpix_session_revoke,
                        pmpix_session_revoke_,
                        pmpix_session_revoke__,
                        ompix_session_revoke_f,
                        (MPI_Fint *session, MPI_Fint *ierr),
                        (session, ierr))

OMPI_GENERATE_F77_BINDINGS(MPIX_SESSION_REVOKE,
                        mpix_session_revoke,
                        mpix_session_revoke_,
                        mpix_session_revoke__,
                        ompix_session_revoke_f,
                        (MPI_Fint *session, MPI_Fint *ierr),
                        (session, ierr))
#endif

void ompix_session_revoke_f(MPI_Fint *session, MPI_Fint *ierr)
{
    MPI_Session c_session = PMPI_Session_f2c(*session);

    *ierr = OMPI_INT_2_FINT(PMPIX_Session_revoke(c_session));
}

