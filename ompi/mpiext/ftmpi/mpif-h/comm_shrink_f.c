/*
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * Copyright (c) 2010-2016 The University of Tennessee and the University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "ompi_config.h"

#include "ompi/communicator/communicator.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"

#if OMPI_PROFILE_LAYER
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_Comm_shrink_f = PMPIX_Comm_shrink_f
#endif
#define MPIX_Comm_shrink_f PMPIX_Comm_shrink_f
#endif

#include "ompi/mpiext/ftmpi/mpif-h/f77_mangle.h"
F77_STAMP_FN(MPIX_Comm_shrink_f,
             mpix_comm_shrink,
             MPIX_COMM_SHRINK,
             (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr),
             (comm, newcomm, ierr))
F77_STAMP_FN(MPIX_Comm_shrink_f,
             ompi_comm_shrink,
             OMPI_COMM_SHRINK,
             (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr),
             (comm, newcomm, ierr))

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

void MPIX_Comm_shrink_f(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *ierr)
{
    MPI_Comm c_newcomm;
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_shrink(c_comm,
                                             &c_newcomm));
    if (MPI_SUCCESS == OMPI_FINT_2_INT(*ierr)) {
        *newcomm = MPI_Comm_c2f(c_newcomm);
    }
}
