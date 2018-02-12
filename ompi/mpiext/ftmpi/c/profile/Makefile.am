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

#
# OMPI_BUILD_MPI_PROFILING flag is enabled when we want our MPI_* symbols
# to be replaced by PMPI_*.

AM_CPPFLAGS = -DOMPI_BUILD_MPI_PROFILING=1 -DOMPI_COMPILING_FORTRAN_WRAPPERS=1

noinst_LTLIBRARIES = libmpiext_ftmpi_c_pmpi.la

nodist_libmpiext_ftmpi_c_pmpi_la_SOURCES = \
	pcomm_revoke.c \
	pcomm_shrink.c \
	pcomm_failure_ack.c \
	pcomm_failure_get_acked.c \
	pcomm_agree.c \
	pcomm_iagree.c

#
# Sym link in the sources from the real MPI directory
#
$(nodist_libmpiext_ftmpi_c_pmpi_la_SOURCES):
	$(OMPI_V_LN_S) if test ! -r $@ ; then \
		pname=`echo $@ | cut -b '2-'` ; \
		$(LN_S) $(top_srcdir)/ompi/mpiext/ftmpi/c/$$pname $@ ; \
	fi

# Conditionally install the header files

# These files were created by targets above

MAINTAINERCLEANFILES = $(nodist_libmpiext_ftmpi_c_pmpi_la_SOURCES)
