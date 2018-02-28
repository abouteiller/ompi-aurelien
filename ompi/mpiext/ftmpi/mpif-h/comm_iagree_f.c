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
#include "ompi/request/request.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"

#if OMPI_PROFILE_LAYER
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_Comm_iagree_f = PMPIX_Comm_iagree_f
#endif
#define MPIX_Comm_iagree_f PMPIX_Comm_iagree_f
#endif

#include "ompi/mpiext/ftmpi/mpif-h/f77_mangle.h"
F77_STAMP_FN(MPIX_Comm_iagree_f,
             mpix_comm_iagree,
             MPIX_COMM_IAGREE,
             (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr),
             (comm, flag, request, ierr))
F77_STAMP_FN(MPIX_Comm_iagree_f,
             ompi_comm_iagree,
             OMPI_COMM_IAGREE,
             (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr),
             (comm, flag, request, ierr))

#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

void MPIX_Comm_iagree_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    MPI_Request c_req;
    OMPI_LOGICAL_NAME_DECL(flag)

    *ierr = OMPI_INT_2_FINT(MPIX_Comm_iagree(c_comm,
                                             OMPI_LOGICAL_SINGLE_NAME_CONVERT(flag),
                                             &c_req));

    if (MPI_SUCCESS == OMPI_FINT_2_INT(*ierr)) {
        *request = MPI_Request_c2f(c_req);
    }
}
