/*
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
#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"
#include "ompi/mpiext/ftmpi/mpif-h/prototypes_mpi.h"

#if OMPI_BUILD_MPI_PROFILING
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak PMPIX_COMM_AGREE = ompi_comm_agree_f
#pragma weak pmpix_comm_agree = ompi_comm_agree_f
#pragma weak pmpix_comm_agree_ = ompi_comm_agree_f
#pragma weak pmpix_comm_agree__ = ompi_comm_agree_f

#pragma weak PMPIX_Comm_agree_f = ompi_comm_agree_f
#pragma weak PMPIX_Comm_agree_f08 = ompi_comm_agree_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(PMPIX_COMM_AGREE,
                        pmpix_comm_agree,
                        pmpix_comm_agree_,
                        pmpix_comm_agree__,
                        pompix_comm_agree_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
                        (comm, flag, ierr))
#endif
#endif

#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_COMM_AGREE = ompi_comm_agree_f
#pragma weak mpix_comm_agree = ompi_comm_agree_f
#pragma weak mpix_comm_agree_ = ompi_comm_agree_f
#pragma weak mpix_comm_agree__ = ompi_comm_agree_f

#pragma weak MPIX_Comm_agree_f = ompi_comm_agree_f
#pragma weak MPIX_Comm_agree_f08 = ompi_comm_agree_f

#else /* No weak symbols */
#if ! OMPI_BUILD_MPI_PROFILING
OMPI_GENERATE_F77_BINDINGS(MPIX_COMM_AGREE,
                        mpix_comm_agree,
                        mpix_comm_agree_,
                        mpix_comm_agree__,
                        ompix_comm_agree_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
                        (comm, flag, ierr))
#else
#define ompix_comm_agree_f pompix_comm_agree_f
#endif
#endif

void ompix_comm_agree_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr)
{
    MPI_Comm c_comm = PMPI_Comm_f2c(*comm);
    OMPI_LOGICAL_NAME_DECL(flag)

    *ierr = OMPI_INT_2_FINT(PMPIX_Comm_agree(c_comm,
                                             OMPI_LOGICAL_SINGLE_NAME_CONVERT(flag)));
}
