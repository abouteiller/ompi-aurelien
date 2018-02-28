! -*- f90 -*-
!
! $COPYRIGHT$
!

subroutine MPIX_Comm_revoke_f08(comm, ierror)
  use :: mpi_f08_types, only : MPI_Comm
  implicit none
  interface
     subroutine MPIX_Comm_revoke_f(comm, ierror) &
          BIND(C, name="MPIX_Comm_revoke_f")
       implicit none
       INTEGER, INTENT(IN) :: comm
       INTEGER, INTENT(OUT) :: ierror
     end subroutine MPIX_Comm_revoke_f
  end interface
  TYPE(MPI_Comm), INTENT(IN) :: comm
  INTEGER, OPTIONAL, INTENT(OUT) :: ierror
  integer :: c_ierror

  call MPIX_Comm_revoke_f(comm%MPI_VAL, c_ierror)
  if (present(ierror)) ierror = c_ierror

end subroutine MPIX_Comm_revoke_f08
