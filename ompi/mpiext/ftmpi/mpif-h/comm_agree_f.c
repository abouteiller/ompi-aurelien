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

#include "ompi/mpiext/ftmpi/mpif-h/f77_mangle.h"
F77_STAMP_FN(MPIX_Comm_agree_f,
             mpix_comm_agree,
             MPIX_COMM_AGREE,
             (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
             (comm, flag, ierr))
F77_STAMP_FN(MPIX_Comm_agree_f,
             ompi_comm_agree,
             OMPI_COMM_AGREE,
             (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr),
             (comm, flag, ierr))

#if OMPI_PROFILE_LAYER && ! OPAL_HAVE_WEAK_SYMBOLS
#include "ompi/mpiext/ftmpi/mpif-h/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

static void MPIX_Comm_agree_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    OMPI_LOGICAL_NAME_DECL(flag)

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_agree(c_comm,
                                            OMPI_LOGICAL_SINGLE_NAME_CONVERT(flag)));
}
