! -*- f90 -*-
!
! $COPYRIGHT$


interface MPIX_Comm_agree
subroutine MPIX_Comm_agree_f08(comm,flag,ierror)
   use :: mpi_f08_types, only : MPI_Comm
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   INTEGER, INTENT(INOUT) :: flag
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_agree_f08
end interface MPIX_Comm_agree

interface MPIX_Comm_failure_ack
subroutine MPIX_Comm_failure_ack_f08(comm,ierror)
   use :: mpi_f08_types, only : MPI_Comm
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_failure_ack_f08
end interface MPIX_Comm_failure_ack

interface MPIX_Comm_failure_get_acked
subroutine MPIX_Comm_failure_get_acked_f08(comm,failedgrp,ierror)
   use :: mpi_f08_types, only : MPI_Comm, MPI_Group
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   TYPE(MPI_Group), INTENT(OUT) :: failedgrp
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_failure_get_acked_f08
end interface MPIX_Comm_failure_get_acked

interface MPIX_Comm_iagree
subroutine MPIX_Comm_iagree_f08(comm,flag,request,ierror)
   use :: mpi_f08_types, only : MPI_Comm, MPI_Request
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   INTEGER, INTENT(INOUT), ASYNCHRONOUS :: flag ! should use OMPI_ASYNCHRONOUS
   TYPE(MPI_Request), INTENT(OUT) :: request
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_iagree_f08
end interface MPIX_Comm_iagree

interface MPIX_Comm_revoke
subroutine MPIX_Comm_revoke_f08(comm,ierror)
   use :: mpi_f08_types, only : MPI_Comm
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_revoke_f08
end interface MPIX_Comm_revoke

interface MPIX_Comm_shrink
subroutine MPIX_Comm_shrink_f08(comm,newcomm,ierror)
   use :: mpi_f08_types, only : MPI_Comm
   implicit none
   TYPE(MPI_Comm), INTENT(IN) :: comm
   TYPE(MPI_Comm), INTENT(OUT) :: newcomm
   INTEGER, OPTIONAL, INTENT(OUT) :: ierror
end subroutine MPIX_Comm_shrink_f08
end interface MPIX_Comm_shrink

