/*
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2010-2019 The University of Tennessee and the University
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
#pragma weak PMPIX_COMM_IS_REVOKED = pompi_comm_is_revoked_f
#pragma weak pmpix_comm_is_revoked = pompi_comm_is_revoked_f
#pragma weak pmpix_comm_is_revoked_ = pompi_comm_is_revoked_f
#pragma weak pmpix_comm_is_revoked__ = pompi_comm_is_revoked_f

#pragma weak PMPIX_Comm_is_revoked_f = pompi_comm_is_revoked_f
#pragma weak PMPIX_Comm_is_revoked_f08 = pompi_comm_is_revoked_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(PMPIX_COMM_IS_REVOKED,
                        pmpix_comm_is_revoked,
                        pmpix_comm_is_revoked_,
                        pmpix_comm_is_revoked__,
                        pompi_comm_is_revoked_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
                        (comm, flag, ierr))
#endif
#define ompi_comm_is_revoked_f pompi_comm_is_revoked_f

#else /* PMPI */
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_COMM_IS_REVOKED = ompi_comm_is_revoked_f
#pragma weak mpix_comm_is_revoked = ompi_comm_is_revoked_f
#pragma weak mpix_comm_is_revoked_ = ompi_comm_is_revoked_f
#pragma weak mpix_comm_is_revoked__ = ompi_comm_is_revoked_f

#pragma weak MPIX_Comm_is_revoked_f = ompi_comm_is_revoked_f
#pragma weak MPIX_Comm_is_revoked_f08 = ompi_comm_is_revoked_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(MPIX_COMM_IS_REVOKED,
                        mpix_comm_is_revoked,
                        mpix_comm_is_revoked_,
                        mpix_comm_is_revoked__,
                        ompi_comm_is_revoked_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
                        (comm, flag, ierr))
#endif
#endif /* PMPI */

#include "ompi/communicator/communicator.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"
#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"
void ompi_comm_is_revoked_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr);

void ompi_comm_is_revoked_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);

    OMPI_LOGICAL_NAME_DECL(flag)

    *ierr = OMPI_INT_2_FINT(PMPIX_Comm_is_revoked(c_comm,
                                                  OMPI_LOGICAL_SINGLE_NAME_CONVERT(flag)));
}
