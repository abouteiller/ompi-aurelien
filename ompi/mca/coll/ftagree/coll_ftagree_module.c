/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2019 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */

#include "ompi_config.h"
#include "opal/util/bit_ops.h"
#include "ompi/mca/coll/coll.h"
#include "ompi/mca/coll/base/base.h"

#include "coll_ftagree.h"
#include "coll_ftagree_era.h"

#if OPAL_ENABLE_FT_MPI
static int
mca_coll_ftagree_noft(void *contrib,
                      int dt_count,
                      struct ompi_datatype_t *dt,
                      struct ompi_op_t *op,
                      struct ompi_group_t **group, bool update_grp,
                      struct ompi_communicator_t* comm,
                      struct mca_coll_base_module_2_3_0_t *module)
{
    return comm->c_coll->coll_allreduce(MPI_IN_PLACE, contrib, dt_count, dt, op,
                                       comm, comm->c_coll->coll_allreduce_module);
}

static int
mca_coll_ftagree_inoft(void *contrib,
                       int dt_count,
                       struct ompi_datatype_t *dt,
                       struct ompi_op_t *op,
                       struct ompi_group_t **group, bool update_grp,
                       struct ompi_communicator_t* comm,
                       ompi_request_t **request,
                       struct mca_coll_base_module_2_3_0_t *module)
{
    return comm->c_coll->coll_iallreduce(MPI_IN_PLACE, contrib, dt_count, dt, op,
                                        comm, request, comm->c_coll->coll_iallreduce_module);
}

#endif /* OPAL_ENABLE_FT_MPI */

/*
 * Initial query function that is invoked during MPI_INIT, allowing
 * this component to disqualify itself if it doesn't support the
 * required level of thread support.
 */
int
mca_coll_ftagree_init_query(bool enable_progress_threads,
                            bool enable_mpi_threads)
{
    if( mca_coll_ftagree_algorithm == COLL_FTAGREE_EARLY_RETURNING ) {
        return mca_coll_ftagree_era_init();
    }

    return OMPI_SUCCESS;
}


/*
 * Invoked when there's a new communicator that has been created.
 * Look at the communicator and decide which set of functions and
 * priority we want to return.
 */
mca_coll_base_module_t *
mca_coll_ftagree_comm_query(struct ompi_communicator_t *comm,
                            int *priority)
{
    int size;
    mca_coll_ftagree_module_t *ftagree_module;

    ftagree_module = OBJ_NEW(mca_coll_ftagree_module_t);
    if (NULL == ftagree_module) return NULL;

    *priority = mca_coll_ftagree_priority;

    /*
     * Allocate the data that hangs off the communicator
     * Intercommunicators not currently supported
     */
    if( ompi_ftmpi_enabled ) {
        if (OMPI_COMM_IS_INTER(comm)) {
            size = ompi_comm_remote_size(comm)+ompi_comm_size(comm);
        } else {
            size = ompi_comm_size(comm);
        }
        ftagree_module->mccb_num_reqs = size * 2;
        ftagree_module->mccb_reqs = (ompi_request_t**)
            malloc(sizeof(ompi_request_t *) * ftagree_module->mccb_num_reqs);

        ftagree_module->mccb_num_statuses = size * 2; /* x2 for alltoall */
        ftagree_module->mccb_statuses = (ompi_status_public_t*)
            malloc(sizeof(ompi_status_public_t) * ftagree_module->mccb_num_statuses);
    } else {
        ftagree_module->mccb_num_reqs = 0;
        ftagree_module->mccb_reqs = NULL;
        ftagree_module->mccb_num_statuses = 0;
        ftagree_module->mccb_statuses = NULL;
    }

    /*
     * Choose whether to use [intra|inter], and [linear|log]-based
     * algorithms.
     */
    ftagree_module->super.coll_module_enable = mca_coll_ftagree_module_enable;
    ftagree_module->super.ft_event = mca_coll_ftagree_ft_event;

    /* agreement is a reduction with a bitwise OR */
    ftagree_module->super.coll_agreement  = mca_coll_ftagree_noft;
    ftagree_module->super.coll_iagreement = mca_coll_ftagree_inoft;
    /* This component does not provide any base collectives,
     * just the FT collectives.
     * Other function pointers are zeroed by the module constructor.
     */

    /*
     * Agreement operation setup
     * Intercommunicators not currently supported
     */
    if( ompi_ftmpi_enabled ) {
        /* Choose the correct operations */
        switch( mca_coll_ftagree_algorithm ) {
        case COLL_FTAGREE_NOFT:
            break;
        case COLL_FTAGREE_EARLY_TERMINATION:
            if( !OMPI_COMM_IS_INTER(comm) ) {
                ftagree_module->super.coll_agreement  = mca_coll_ftagree_eta_intra;
            }
            break;
        default: /* Manages the COLL_FTAGREE_EARLY_RETURNING as default case too */
            /* Init the agreement function */
            mca_coll_ftagree_era_comm_init(comm, ftagree_module);
            if( OMPI_COMM_IS_INTER(comm) ) {
                ftagree_module->super.coll_agreement  = mca_coll_ftagree_era_inter;
            } else {
                ftagree_module->super.coll_agreement  = mca_coll_ftagree_era_intra;
                ftagree_module->super.coll_iagreement = mca_coll_ftagree_iera_intra;
            }
            break;
        }
    }

    return &(ftagree_module->super);
}


/*
 * Init module on the communicator
 */
int
mca_coll_ftagree_module_enable(mca_coll_base_module_t *module,
                             struct ompi_communicator_t *comm)
{
    /* All done */
    return OMPI_SUCCESS;
}


int mca_coll_ftagree_ft_event(int state)
{

    /* Nothing to do for checkpoint */

    return OMPI_SUCCESS;
}