#
# Copyright (c) 2011-2012 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2017      Research Organization for Information Science
#                         and Technology (RIST). All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

AM_FCFLAGS = $(OMPI_FC_MODULE_FLAG)$(top_builddir)/ompi/mpi/fortran/use-mpi-f08/mod \
             -I$(top_srcdir) $(FCFLAGS_f90)

noinst_LTLIBRARIES =

if OMPI_BUILD_FORTRAN_USEMPIF08_BINDINGS

# If we are, build the convenience libtool library that will be
# slurped up into libmpi_usempif08.la.
noinst_LTLIBRARIES += libmpiext_ftmpi_usempif08.la

noinst_HEADERS = mpiext_ftmpi_usempif08.h

# Sources for the convenience libtool library.
libmpiext_ftmpi_usempif08_la_SOURCES = comm_agree_f08.F90 \
                                       comm_failure_ack_f08.F90 \
                                       comm_failure_get_acked_f08.F90 \
                                       comm_iagree_f08.F90 \
                                       comm_revoke_f08.F90 \
                                       comm_shrink_f08.F90
libmpiext_ftmpi_usempif08_la_LDFLAGS = -module -avoid-version
endif
