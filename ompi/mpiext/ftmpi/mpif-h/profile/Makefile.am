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

include $(top_srcdir)/Makefile.ompi-rules
#
# OMPI_PROFILING_DEFINES flag s enabled when we want our MPI_* symbols
# to be replaced by PMPI_*. In other words, this flag decides
# whether "profile/defines.h" is included or not. "profile/defines.h"
# replaces all MPI_* symbols with PMPI_* symbols. In this directory
# we definately need it to be 1.
#
AM_CPPFLAGS = -DOMPI_BUILD_MPI_PROFILING=1 -DOMPI_COMPILING_FORTRAN_WRAPPERS=1

headers =
noinst_LTLIBRARIES =
nodist_libpmpiext_ftmpi_mpifh_la_SOURCES =

if OMPI_BUILD_FORTRAN_MPIFH_BINDINGS

headers += pmpiext_ftmpi_mpifh.h

nodist_libpmpiext_ftmpi_mpifh_la_SOURCES += \
    $(headers) \
    pcomm_revoke_f.c \
    pcomm_is_revoked_f.c \
    pcomm_shrink_f.c \
    pcomm_failure_ack_f.c \
    pcomm_failure_get_acked_f.c \
    pcomm_agree_f.c \
    pcomm_iagree_f.c

if BUILD_PMPI_FORTRAN_MPIFH_BINDINGS_LAYER
noinst_LTLIBRARIES += libpmpiext_ftmpi_mpifh.la
endif

#
# Sym link in the sources from the real MPI directory
#
$(nodist_libpmpiext_ftmpi_mpifh_la_SOURCES):
	if test ! -r $@ ; then \
		pname=`echo $@ | cut -b '2-'` ; \
		$(LN_S) $(top_srcdir)/ompi/mpiext/ftmpi/mpif-h/$$pname $@ ; \
	fi

ompidir = $(includedir)/openmpi/ompi/mpiext/ftmpi/mpif-h
ompi_HEADERS = $(headers)

# These files were created by targets above

MAINTAINERCLEANFILES = $(nodist_libpmpiext_ftmpi_mpifh_la_SOURCES)
endif
