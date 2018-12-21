! -*- f90 -*-
!
! $COPYRIGHT$
!

subroutine MPIX_Comm_agree_f08(comm, flag, ierror)
  use :: mpi_f08_types, only : MPI_Comm
  implicit none
  interface
     subroutine MPIX_Comm_agree_f(comm, flag, ierror) &
          BIND(C, name="MPIX_Comm_agree_f")
       implicit none
       INTEGER, INTENT(IN) :: comm
       INTEGER, INTENT(INOUT) :: flag
       INTEGER, INTENT(OUT) :: ierror
     end subroutine MPIX_Comm_agree_f
  end interface
  TYPE(MPI_Comm), INTENT(IN) :: comm
  INTEGER, INTENT(INOUT) :: flag
  INTEGER, OPTIONAL, INTENT(OUT) :: ierror
  integer :: c_ierror

  call MPIX_Comm_agree_f(comm%MPI_VAL, flag, c_ierror)
  if (present(ierror)) ierror = c_ierror

end subroutine MPIX_Comm_agree_f08
