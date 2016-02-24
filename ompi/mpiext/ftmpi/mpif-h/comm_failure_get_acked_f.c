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

F77_STAMP_FN(MPIX_Comm_failure_get_acked_f,
             mpix_comm_failure_get_acked,
             MPIX_COMM_FAILURE_GET_ACKED,
             (MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr),
             (comm, group, ierr))
F77_STAMP_FN(MPIX_Comm_failure_get_acked_f,
             ompi_comm_failure_get_acked,
             OMPI_COMM_FAILURE_GET_ACKED,
             (MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr),
             (comm, group, ierr))

#if OMPI_PROFILE_LAYER && ! OPAL_HAVE_WEAK_SYMBOLS
#include "ompi/mpiext/ftmpi/mpif-h/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"
#include "ompi/communicator/communicator.h"

static void MPIX_Comm_failure_get_acked_f(MPI_Fint *comm, MPI_Fint *group, MPI_Fint *ierr)
{
    MPI_Group c_group;
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_failure_get_acked(c_comm,
                                                        &c_group));
    if (MPI_SUCCESS == OMPI_FINT_2_INT(*ierr)) {
        *group = MPI_Group_c2f (c_group);
    }
}
