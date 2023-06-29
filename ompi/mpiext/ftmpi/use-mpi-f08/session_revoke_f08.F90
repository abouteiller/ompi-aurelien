! -*- f90 -*-
!
! Copyright (c) 2023      The University of Tennessee and the University
!                         of Tennessee Research Foundation.  All rights
!                         reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!

subroutine MPIX_Session_revoke_f08(session, ierror)
  use :: mpi_f08_types, only : MPI_Session
  implicit none
  interface
     subroutine ompix_session_revoke_f(session, ierror) &
          BIND(C, name="ompix_session_revoke_f")
       implicit none
       INTEGER, INTENT(IN) :: session
       INTEGER, INTENT(OUT) :: ierror
     end subroutine ompix_session_revoke_f
  end interface
  TYPE(MPI_Session), INTENT(IN) :: session
  INTEGER, OPTIONAL, INTENT(OUT) :: ierror
  integer :: c_ierror

  call ompix_session_revoke_f(session%MPI_VAL, c_ierror)
  if (present(ierror)) ierror = c_ierror

end subroutine MPIX_Session_revoke_f08
