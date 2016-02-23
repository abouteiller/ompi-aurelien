#
# Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
# Copyright (c) 2016      The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# $COPYRIGHT$
# 
# Additional copyrights may follow
# 
# $HEADER$
#

SUBDIRS = profile

headers =
f77_sources =
if OMPI_BUILD_FORTRAN_MPIFH_BINDINGS
headers += \
	mpiext_ftmpi_mpifh.h \
	ftmpi_f77_support.h

f77_sources += \
	comm_revoke_f.c \
	comm_shrink_f.c \
	comm_failure_ack_f.c \
	comm_failure_get_acked_f.c \
	comm_agree_f.c \
	comm_iagree_f.c

AM_CPPFLAGS = -DOMPI_PROFILE_LAYER=0 -DOMPI_COMPILING_FORTRAN_WRAPPERS=1

f77_lib = libmpiext_ftmpi_mpifh.la
f77_lib_sources = $(f77_sources)

libmpiext_ftmpi_mpifh_la_SOURCES = $(f77_lib_sources)
libmpiext_ftmpi_mpifh_la_LIBADD  = $(lib_sources)
libmpiext_ftmpi_mpifh_la_LDFLAGS = -module -avoid-version
if BUILD_PMPI_FORTRAN_MPIFH_BINDINGS_LAYER
  libmpiext_ftmpi_mpifh_la_LIBADD += profile/libpmpiext_ftmpi_mpifh.la
endif
# These files were created by targets above

MAINTAINERCLEANFILES = $(nodist_libmpiext_ftmpi_mpifh_la_SOURCES)
endif

noinst_LTLIBRARIES = $(f77_lib)

# Conditionally install the header files
if WANT_INSTALL_HEADERS
ompidir = $(includedir)/openmpi/ompi/mpiext/ftmpi/mpif-h
ompi_HEADERS = $(headers)
endif
