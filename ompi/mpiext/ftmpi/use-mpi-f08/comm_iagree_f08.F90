! -*- f90 -*-
!
! $COPYRIGHT$
!

#include "ompi/mpi/fortran/configure-fortran-output.h"

subroutine MPIX_Comm_iagree_f08(comm, flag, request, ierror)
  use :: mpi_f08_types, only : MPI_Comm, MPI_Request
  implicit none
  interface
     subroutine MPIX_Comm_iagree_f(comm, flag, request, ierror) &
          BIND(C, name="MPIX_Comm_iagree_f")
       implicit none
       INTEGER, INTENT(IN) :: comm
       INTEGER, INTENT(INOUT) :: flag
       INTEGER, INTENT(OUT) :: request
       INTEGER, INTENT(OUT) :: ierror
     end subroutine MPIX_Comm_iagree_f
  end interface
  TYPE(MPI_Comm), INTENT(IN) :: comm
  INTEGER, INTENT(INOUT) OMPI_ASYNCHRONOUS :: flag
  TYPE(MPI_Request), INTENT(OUT) :: request
  INTEGER, OPTIONAL, INTENT(OUT) :: ierror
  integer :: c_ierror

  call MPIX_Comm_iagree_f(comm%MPI_VAL, flag, request%MPI_VAL, c_ierror)
  if (present(ierror)) ierror = c_ierror

end subroutine MPIX_Comm_iagree_f08
