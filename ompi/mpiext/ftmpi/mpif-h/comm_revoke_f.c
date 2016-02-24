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

#include "ompi/mpiext/ftmpi/mpif-h/ftmpi_f77_support.h"

F77_STAMP_FN(MPIX_Comm_revoke_f,
             mpix_comm_revoke,
             MPIX_COMM_REVOKE,
             (MPI_Fint *comm, MPI_Fint *ierr),
             (comm, ierr))
F77_STAMP_FN(MPIX_Comm_revoke_f,
             ompi_comm_revoke,
             OMPI_COMM_REVOKE,
             (MPI_Fint *comm, MPI_Fint *ierr),
             (comm, ierr))

#if OMPI_PROFILE_LAYER && ! OPAL_HAVE_WEAK_SYMBOLS
#include "ompi/mpiext/ftmpi/mpif-h/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"
#include "ompi/communicator/communicator.h"

static void MPIX_Comm_revoke_f(MPI_Fint *comm, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_revoke(c_comm));
}
