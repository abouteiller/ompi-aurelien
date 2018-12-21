/*
 * Copyright (c) 2010-2018 The University of Tennessee and the University
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

#if OMPI_BUILD_MPI_PROFILING
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak PMPIX_COMM_REVOKE = pompi_comm_revoke_f
#pragma weak pmpix_comm_revoke = pompi_comm_revoke_f
#pragma weak pmpix_comm_revoke_ = pompi_comm_revoke_f
#pragma weak pmpix_comm_revoke__ = pompi_comm_revoke_f

#pragma weak PMPIX_Comm_revoke_f = pompi_comm_revoke_f
#pragma weak PMPIX_Comm_revoke_f08 = pompi_comm_revoke_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(PMPIX_COMM_REVOKE,
                        pmpix_comm_revoke,
                        pmpix_comm_revoke_,
                        pmpix_comm_revoke__,
                        pompi_comm_revoke_f,
                        (MPI_Fint *comm, MPI_Fint *ierr),
                        (comm, ierr))
#endif
#define ompi_comm_revoke_f pompi_comm_revoke_f

#else /* PMPI */
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_COMM_REVOKE = ompi_comm_revoke_f
#pragma weak mpix_comm_revoke = ompi_comm_revoke_f
#pragma weak mpix_comm_revoke_ = ompi_comm_revoke_f
#pragma weak mpix_comm_revoke__ = ompi_comm_revoke_f

#pragma weak MPIX_Comm_revoke_f = ompi_comm_revoke_f
#pragma weak MPIX_Comm_revoke_f08 = ompi_comm_revoke_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(MPIX_COMM_REVOKE,
                        mpix_comm_revoke,
                        mpix_comm_revoke_,
                        mpix_comm_revoke__,
                        ompi_comm_revoke_f,
                        (MPI_Fint *comm, MPI_Fint *ierr),
                        (comm, ierr))
#endif
#endif /* PMPI */

#include "ompi/communicator/communicator.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"
#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

void ompi_comm_revoke_f(MPI_Fint *comm, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);

    *ierr = OMPI_INT_2_FINT(PMPIX_Comm_revoke(c_comm));
}
