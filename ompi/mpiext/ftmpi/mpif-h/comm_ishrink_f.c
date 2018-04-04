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

#include "ompi/communicator/communicator.h"
#include "ompi/request/request.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"

#include "ompi/mpiext/ftmpi/mpif-h/f77_mangle.h"
F77_STAMP_FN(MPIX_Comm_ishrink_f,
             mpix_comm_ishrink,
             MPIX_COMM_ishrink,
             (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr),
             (comm, newcomm, request, ierr))
F77_STAMP_FN(MPIX_Comm_ishrink_f,
             ompi_comm_ishrink,
             OMPI_COMM_ishrink,
             (MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr),
             (comm, newcomm, request, ierr))

#if OMPI_PROFILE_LAYER && ! OPAL_HAVE_WEAK_SYMBOLS
#include "ompi/mpiext/ftmpi/mpif-h/profile/defines.h"
#endif

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

static void MPIX_Comm_ishrink_f(MPI_Fint *comm, MPI_Fint *newcomm, MPI_Fint *request, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    MPI_Request c_req;
    MPI_Comm c_newcomm;

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_ishrink(c_comm,
                                             &c_newcomm,
                                             &c_req));

    if (MPI_SUCCESS == OMPI_FINT_2_INT(*ierr)) {
        *request = MPI_Request_c2f(c_req);
        *newcomm = MPI_Comm_c2f(c_newcomm);
    }
    else {
        *newcomm = MPI_Comm_c2f(&ompi_mpi_comm_null.comm);
    }
}
