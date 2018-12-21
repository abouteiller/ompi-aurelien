! -*- fortran -*-
! Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
! Copyright (c) 2010-2016 The University of Tennessee and the University
!                         of Tennessee research foundation.  All rights
!                         reserved.
! $COPYRIGHT$
!
! Additional copyrights may follow
!
! $HEADER$
!

! Include the parameters for this extension
! Included from config/ompi_ext.m4 into mpif90-ext.f90
! include '../mpiext/ftmpi/mpif-h/mpiext_ftmpi_mpifh.h'

!
! Communicators
!
interface MPIX_Comm_revoke
    subroutine mpix_comm_revoke(comm, ierr)
      integer, intent(IN) :: comm
      integer, intent(OUT) :: ierr
    end subroutine mpix_comm_revoke
end interface MPIX_Comm_revoke

interface MPIX_Comm_shrink
    subroutine mpix_comm_shrink(comm, newcomm, ierr)
      integer, intent(IN) :: comm
      integer, intent(OUT) :: newcomm, ierr
    end subroutine mpix_comm_shrink
end interface MPIX_Comm_shrink

interface MPIX_Comm_failure_ack
    subroutine mpix_comm_failure_ack(comm, ierr)
      integer, intent(IN) :: comm
      integer, intent(OUT) :: ierr
    end subroutine mpix_comm_failure_ack
end interface MPIX_Comm_failure_ack

interface MPIX_Comm_failure_get_acked
    subroutine mpix_comm_failure_get_acked(comm, failedgrp, ierr)
        integer, intent(IN) :: comm
        integer, intent(OUT) :: failedgrp, ierr
    end subroutine mpix_comm_failure_get_acked
end interface MPIX_Comm_failure_get_acked

interface MPIX_Comm_agree
    subroutine mpix_comm_agree(comm, ierr)
      integer, intent(IN) :: comm
      integer, intent(OUT) :: ierr
    end subroutine mpix_comm_agree
end interface MPIX_Comm_agree

interface MPIX_Comm_iagree
    subroutine mpix_comm_iagree(comm, request, ierr)
      integer, intent(IN) :: comm
      integer, intent(OUT) :: request, ierr
    end subroutine mpix_comm_iagree
end interface MPIX_Comm_iagree


!
! Validation: Windows
! Todo
!


!
! Validation: File Handles
! Todo
!


!
