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

headers = \
	mpiext_ftmpi_c.h

sources = \
	comm_revoke.c \
	comm_shrink.c \
	comm_failure_ack.c \
	comm_failure_get_acked.c \
	comm_agree.c \
	comm_iagree.c

AM_CPPFLAGS = -DOMPI_PROFILE_LAYER=0 -DOMPI_COMPILING_FORTRAN_WRAPPERS=1

lib = libmpiext_ftmpi_c.la
lib_sources = $(sources)

libmpiext_ftmpi_c_la_SOURCES = $(lib_sources)
libmpiext_ftmpi_c_la_LDFLAGS = -module -avoid-version

noinst_LTLIBRARIES = $(lib)

ompidir = $(ompiincludedir)/ompi/mpiext/ftmpi/c
ompi_HEADERS = \
        $(headers)

MAINTAINERCLEANFILES = $(nodist_libmpiext_ftmpi_c_la_SOURCES)
