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
#pragma weak PMPIX_COMM_IAGREE = pompi_comm_iagree_f
#pragma weak pmpix_comm_iagree = pompi_comm_iagree_f
#pragma weak pmpix_comm_iagree_ = pompi_comm_iagree_f
#pragma weak pmpix_comm_iagree__ = pompi_comm_iagree_f

#pragma weak PMPIX_Comm_iagree_f = pompi_comm_iagree_f
#pragma weak PMPIX_Comm_iagree_f08 = pompi_comm_iagree_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(PMPIX_COMM_IAGREE,
                        pmpix_comm_iagree,
                        pmpix_comm_iagree_
                        pmpix_comm_iagree__,
                        pompi_comm_iagree_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr),
                        (comm, flag, request, ierr))
#endif
#define ompi_comm_iagree_f pompi_comm_iagree_f

#else /* PMPI */
#if OPAL_HAVE_WEAK_SYMBOLS
#pragma weak MPIX_COMM_IAGREE = ompi_comm_iagree_f
#pragma weak mpix_comm_iagree = ompi_comm_iagree_f
#pragma weak mpix_comm_iagree_ = ompi_comm_iagree_f
#pragma weak mpix_comm_iagree__ = ompi_comm_iagree_f

#pragma weak MPIX_Comm_iagree_f = ompi_comm_iagree_f
#pragma weak MPIX_Comm_iagree_f08 = ompi_comm_iagree_f

#else /* No weak symbols */
OMPI_GENERATE_F77_BINDINGS(MPIX_COMM_IAGREE,
                        mpix_comm_iagree,
                        mpix_comm_iagree_
                        mpix_comm_iagree__,
                        ompi_comm_iagree_f,
                        (MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr),
                        (comm, flag, request, ierr))
#endif
#endif /* PMPI */

#include "ompi/communicator/communicator.h"
#include "ompi/mpi/fortran/base/fint_2_int.h"
#include "ompi/mpiext/ftmpi/c/mpiext_ftmpi_c.h"

void ompi_comm_iagree_f(MPI_Fint *comm, ompi_fortran_logical_t *flag, MPI_Fint *request, MPI_Fint *ierr)
{
    MPI_Comm c_comm = MPI_Comm_f2c(*comm);
    MPI_Request c_req;
    OMPI_LOGICAL_NAME_DECL(flag)

    *ierr = OMPI_INT_2_FINT(PMPIX_Comm_iagree(c_comm,
                                              OMPI_LOGICAL_SINGLE_NAME_CONVERT(flag),
                                              &c_req));

    if (MPI_SUCCESS == OMPI_FINT_2_INT(*ierr)) {
        *request = MPI_Request_c2f(c_req);
    }
}
