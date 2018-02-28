! -*- f90 -*-
!
! Copyright (c) 2009-2012 Cisco Systems, Inc.  All rights reserved.
! Copyright (c) 2009-2012 Los Alamos National Security, LLC.
!                         All rights reserved.
! $COPYRIGHT$
!

subroutine MPIX_Comm_failure_ack_f08(comm, ierror)
  use :: mpi_f08_types, only : MPI_Comm
  implicit none
  interface
     subroutine MPIX_Comm_failure_ack_f(comm, ierror) &
          BIND(C, name="MPIX_Comm_failure_ack_f")
       implicit none
       INTEGER, INTENT(IN) :: comm
       INTEGER, INTENT(OUT) :: ierror
     end subroutine MPIX_Comm_failure_ack_f
  end interface
  TYPE(MPI_Comm), INTENT(IN) :: comm
  INTEGER, OPTIONAL, INTENT(OUT) :: ierror
  integer :: c_ierror

  call MPIX_Comm_failure_ack_f(comm%MPI_VAL, c_ierror)
  if (present(ierror)) ierror = c_ierror

end subroutine MPIX_Comm_failure_ack_f08
