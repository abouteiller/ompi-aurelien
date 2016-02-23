/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2016 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart, 
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2008      Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2010-2012 Oak Ridge National Labs.  All rights reserved.
 * $COPYRIGHT$
 * 
 * Additional copyrights may follow
 * 
 * $HEADER$
 *
 * These symbols are in a file by themselves to provide nice linker
 * semantics.  Since linkers generally pull in symbols by object
 * files, keeping these symbols as the only symbols in this file
 * prevents utility programs such as "ompi_info" from having to import
 * entire components just to query their version and parameters.
 */

#include "ompi_config.h"
#include "ompi/mca/coll/coll.h"
#include "ompi/mca/coll/ftbasic/coll_ftbasic.h"
#include "ompi/mca/coll/ftbasic/coll_ftbasic_agreement.h"


/*
 * Public string showing the coll ompi_ftbasic component version number
 */
const char *mca_coll_ftbasic_component_version_string =
    "Open MPI ftbasic collective MCA component version " OMPI_VERSION;

/*
 * Global variables
 */
int mca_coll_ftbasic_priority  = 0;
mca_coll_ftbasic_agreement_method_t mca_coll_ftbasic_cur_agreement_method = COLL_FTBASIC_EARLY_RETURNING;
int mca_coll_ftbasic_cur_era_topology = 1;
int mca_coll_ftbasic_era_rebuild = 0;

/*
 * Local function
 */
static int ftbasic_register(void);
static int ftbasic_close(void);
/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

const mca_coll_base_component_2_0_0_t mca_coll_ftbasic_component = {

    /* First, the mca_component_t struct containing meta information
     * about the component itself */

    {
     MCA_COLL_BASE_VERSION_2_0_0,

     /* Component name and version */
     "ftbasic",
     OMPI_MAJOR_VERSION,
     OMPI_MINOR_VERSION,
     OMPI_RELEASE_VERSION,

     /* Component open and close functions */
     NULL,
     ftbasic_close,
     NULL,
     ftbasic_register
    },
    {
        /* The component is checkpoint ready */
        MCA_BASE_METADATA_PARAM_CHECKPOINT
    },

    /* Initialization / querying functions */

    mca_coll_ftbasic_init_query,
    mca_coll_ftbasic_comm_query
};

static int
ftbasic_close(void)
{
    if( mca_coll_ftbasic_cur_agreement_method ==  COLL_FTBASIC_EARLY_RETURNING ) {
        return mca_coll_ftbasic_agreement_era_finalize();
    }
    return OMPI_SUCCESS;
}

static int
ftbasic_register(void)
{
    int value;

    /* Use a low priority, but allow other components to be lower */
    mca_coll_ftbasic_priority = 30;
    (void) mca_base_component_var_register(&mca_coll_ftbasic_component.collm_version,
                                           "priority", "Priority of the ftbasic coll component",
                                           MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                           OPAL_INFO_LVL_6,
                                           MCA_BASE_VAR_SCOPE_READONLY,
                                           &mca_coll_ftbasic_priority);

    value = 0;
    (void) mca_base_component_var_register(&mca_coll_ftbasic_component.collm_version,
                                           "agreement", "Agreement algorithm 0: Early Returning Concensus (era); 1: Early Concensus Termination (eta)",
                                           MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                           OPAL_INFO_LVL_6,
                                           MCA_BASE_VAR_SCOPE_READONLY,
                                           &value);
    switch(value) {
    case 0:
        mca_coll_ftbasic_cur_agreement_method = COLL_FTBASIC_EARLY_TERMINATION;
        opal_output_verbose(6, ompi_ftmpi_output_handle,
                            "%s ftbasic:register) Agreement Algorithm - Early Terminating Consensus Algorithm",
                            OMPI_NAME_PRINT(OMPI_PROC_MY_NAME) );
        break;
    default:  /* Includes the valid case 1 */
        mca_coll_ftbasic_cur_agreement_method = COLL_FTBASIC_EARLY_RETURNING;
        opal_output_verbose(6, ompi_ftmpi_output_handle,
                            "%s ftbasic:register) Agreement Algorithm - Early Returning Consensus Algorithm",
                            OMPI_NAME_PRINT(OMPI_PROC_MY_NAME) );
        break;
    }

    mca_coll_ftbasic_cur_era_topology = 1;
    (void) mca_base_component_var_register(&mca_coll_ftbasic_component.collm_version,
                                           "era_topology", "ERA topology 1: binary tree; 2: star tree; 3: string tree",
                                           MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                           OPAL_INFO_LVL_6,
                                           MCA_BASE_VAR_SCOPE_READONLY,
                                           &mca_coll_ftbasic_cur_era_topology);

    /* TODO: add an adaptative rebuilding strategy */
    mca_coll_ftbasic_era_rebuild = 0; /* by default do not rebuild, master-worker application patterns can benefit greatly from rebuilding... */
    (void) mca_base_component_var_register(&mca_coll_ftbasic_component.collm_version,
                                           "era_rebuild", "ERA rebuild/rebalance the tree in a first post-failure agreement 0: no rebalancing; 1: rebalance all the time",
                                           MCA_BASE_VAR_TYPE_INT, NULL, 0, 0,
                                           OPAL_INFO_LVL_6,
                                           MCA_BASE_VAR_SCOPE_READONLY,
                                           &mca_coll_ftbasic_era_rebuild);


    return OMPI_SUCCESS;
}


static void
mca_coll_ftbasic_module_construct(mca_coll_ftbasic_module_t *module)
{
    module->is_intercomm = false;

    module->mccb_reqs = NULL;
    module->mccb_num_reqs = 0;

    module->mccb_statuses = NULL;
    module->mccb_num_statuses = 0;

    module->mccb_coll_tag_agreement = MCA_COLL_BASE_TAG_AGREEMENT;

    /* This object is managed by the agreement operation selected */
    module->agreement_info = NULL;
}

static void
mca_coll_ftbasic_module_destruct(mca_coll_ftbasic_module_t *module)
{

    /* Finalize the agreement function */
    if( ompi_ftmpi_enabled && !module->is_intercomm ) {
        mca_coll_ftbasic_agreement_finalize(module);
    }

    module->is_intercomm = false;

    /* This object is managed by the agreement operation selected */
    module->agreement_info = NULL;

    if (NULL != module->mccb_reqs) {
        free(module->mccb_reqs);
        module->mccb_reqs = NULL;
    }

    if( NULL != module->mccb_statuses ) {
        free(module->mccb_statuses);
        module->mccb_statuses = NULL;
    }
}


OBJ_CLASS_INSTANCE(mca_coll_ftbasic_module_t,
                   mca_coll_base_module_t,
                   mca_coll_ftbasic_module_construct,
                   mca_coll_ftbasic_module_destruct);
