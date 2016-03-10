/*
 * Copyright (c) 2016      The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
#include "opal/mca/base/mca_base_var.h"

#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/mca/pml/pml.h"

static int comm_heartbeat_recv_cb_type = -1;
static int comm_heartbeat_request_cb_type = -1;

static double comm_heartbeat_period = 5e-5;
static double comm_heartbeat_timeout = 1e-6;

typedef struct fd_heartbeat_t {
    ompi_comm_rbcast_message_t super;
    int from;
} ompi_comm_heartbeat_message_t;

typedef struct {
    ompi_communicator_t* comm;
    int hb_observing; /* the rank of the process we observe */
    int hb_observer; /* the rank of the process that observes us */
    double hb_period; /* the time spacing between heartbeat emission (eta) */
    double hb_timeout; /* the timeout before we start suspecting observed process as dead (delta) */
    double hb_sstamp; /* the date at which the last hb emission was done */
    double hb_rstamp; /* the date of the last hb reception */
} comm_detector_t;

static comm_detector_t comm_world_detector;

static int fd_heartbeat_send(ompi_communicator_t* comm);
static int fd_heartbeat_recv_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg);
static int fd_heartbeat_request(ompi_communicator_t* comm);
static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg);
static int fd_heartbeat_timeout_cb(ompi_communicator_t* comm);

int ompi_comm_init_failure_detector(void) {
    int ret;
    bool detect = true;

    (void) mca_base_var_register ("ompi", "mpi", "ft", "detector",
                                  "Use the OMPI heartbeat based failure detector, or disable it and use only RTE and in-band detection (slower)",
                                  MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                  OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_READONLY, &detect);
    (void) mca_base_var_register ("ompi", "mpi", "ft", "detector_period",
                                  "Period of heartbeat emission (s)",
                                  MCA_BASE_VAR_TYPE_DOUBLE, NULL, 0, 0,
                                  OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_READONLY, &comm_heartbeat_period);
    (void) mca_base_var_register ("ompi", "mpi", "ft", "detector_timeout",
                                  "Timeout before we start suspecting a process after the last heartbeat reception (must be larger than 3*ompi_mpi_ft_detector_period)",
                                  MCA_BASE_VAR_TYPE_DOUBLE, NULL, 0, 0,
                                  OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_READONLY, &comm_heartbeat_timeout);
    if( !detect || !ompi_ftmpi_enabled ) return OMPI_SUCCESS;

    /* using rbcast to transmit messages (cb must always return the noforward 'false' flag) */
    /* registering the cb types */
    ret = ompi_comm_rbcast_register_cb_type((ompi_comm_rbcast_cb_t)fd_heartbeat_recv_cb);
    if( 0 > ret ) goto cleanup;
    comm_heartbeat_recv_cb_type = ret;
    ret = ompi_comm_rbcast_register_cb_type((ompi_comm_rbcast_cb_t)fd_heartbeat_request_cb);
    if( 0 > ret ) goto cleanup;
    comm_heartbeat_request_cb_type = ret;

    /* setting up the default detector on comm_world */
    int rank, np;
    comm_world_detector.comm = &ompi_mpi_comm_world.comm;
    np = ompi_comm_size(comm_world_detector.comm);
    rank = ompi_comm_rank(comm_world_detector.comm);
    comm_world_detector.hb_observing = (np+rank-1) % np;
    comm_world_detector.hb_observer = (np+rank+1) % np;
    comm_world_detector.hb_period = comm_heartbeat_period;
    comm_world_detector.hb_timeout = comm_heartbeat_timeout;
    comm_world_detector.hb_sstamp = PMPI_Wtime();
    comm_world_detector.hb_rstamp = PMPI_Wtime();

    return OMPI_SUCCESS;

  cleanup:
    ompi_comm_finalize_failure_detector();
    return ret;
}

int ompi_comm_finalize_failure_detector(void) {
    int ret;

    if( -1 != comm_heartbeat_recv_cb_type ) ompi_comm_rbcast_unregister_cb_type(comm_heartbeat_recv_cb_type);
    if( -1 != comm_heartbeat_request_cb_type ) ompi_comm_rbcast_unregister_cb_type(comm_heartbeat_request_cb_type);
    comm_heartbeat_recv_cb_type = comm_heartbeat_request_cb_type = -1;

    return OMPI_SUCCESS;
}


static int fd_heartbeat_send(ompi_communicator_t* comm) {
    assert( -1 != comm_heartbeat_recv_cb_type /* initialized */);
    if( comm != comm_world_detector.comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    comm_world_detector.hb_sstamp = PMPI_Wtime();
    OPAL_OUTPUT_VERBOSE((9, ompi_ftmpi_output_handle,
                         "%s %s: Sending heartbeat to %d on communicator %3d:%d stamp %g",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, comm_world_detector.hb_observer, comm->c_contextid, comm->c_epoch, comm_world_detector.hb_sstamp ));

    ompi_comm_heartbeat_message_t msg;
    msg.super.cid = comm->c_contextid;
    msg.super.epoch = comm->c_epoch;
    msg.super.type = comm_heartbeat_recv_cb_type;
    msg.from = comm->c_my_rank;
    ompi_proc_t* proc = ompi_comm_peer_lookup(comm, comm_world_detector.hb_observer);
    return ompi_comm_rbcast_send_msg(proc, &msg.super, sizeof(msg));
}

static int fd_heartbeat_recv_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg) {
    if( msg->from != comm_world_detector.hb_observing ) {
        OPAL_OUTPUT_VERBOSE((2, ompi_ftmpi_output_handle,
                             "%s %s: Received heartbeat from %d on communicator %3d:%d but I am now monitoring %d -- ignored.",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, comm_world_detector.hb_observing ));
    }
    else {
        double stamp = PMPI_Wtime();
        double grace = comm_world_detector.hb_timeout - (stamp - comm_world_detector.hb_rstamp);
        OPAL_OUTPUT_VERBOSE((9, ompi_ftmpi_output_handle,
                             "%s %s: Recveived heartbeat from %d on communicator %3d:%d at timestamp %g (remained %g of %g before suspecting)",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, stamp, grace, comm_world_detector.hb_timeout ));
        if( grace > 0.0 ) {
            comm_world_detector.hb_rstamp = stamp;
        }
        // else complain? This is indicative of wrong assumptions on the
        // timeout and possibly imperfect detector suspecting live processes
    }
    return false; /* never forward on the rbcast */
}


static int fd_heartbeat_request(ompi_communicator_t* comm) {
    assert( -1 != comm_heartbeat_request_cb_type /* initialized */);
    if( comm != comm_world_detector.comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    if( ompi_comm_is_proc_active(comm, comm_world_detector.hb_observing, OMPI_COMM_IS_INTER(comm)) ) return OMPI_SUCCESS;

    int ret;
    int nb = ompi_comm_size(comm);
    int rank = (nb+comm_world_detector.hb_observing-1) % nb;

    do {
        comm_world_detector.hb_rstamp = PMPI_Wtime();
        OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                             "%s %s: Sending observe request to %d on communicator %3d:%d stamp %g",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, rank, comm->c_contextid, comm->c_epoch, comm_world_detector.hb_rstamp ));

        ompi_comm_heartbeat_message_t msg;
        msg.super.cid = comm->c_contextid;
        msg.super.epoch = comm->c_epoch;
        msg.super.type = comm_heartbeat_request_cb_type;
        msg.from = comm->c_my_rank;
        ompi_proc_t* proc = ompi_comm_peer_lookup(comm, rank);
        ret = ompi_comm_rbcast_send_msg(proc, &msg.super, sizeof(msg));
    } while( OMPI_SUCCESS != ret && rank != comm->c_my_rank );
    /* if everybody else is dead, then it's a success */
    comm_world_detector.hb_observing = rank;
    comm_world_detector.hb_rstamp = PMPI_Wtime()+comm_world_detector.hb_timeout; /* we add one timeout slack to account for the send time */
    return OMPI_SUCCESS;
}

static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg) {
#if OPAL_ENABLE_DEBUG
    int nb, rr, ro;
    nb = ompi_comm_size(comm);
    rr = (nb-comm->c_my_rank+msg->from) % nb; /* translate msg->from in circular space so that myrank==0 */
    ro = (nb-comm->c_my_rank+comm_world_detector.hb_observer) % nb; /* same for the observer rank */
    if( rr < ro ) {
        opal_output_verbose(1, ompi_ftmpi_output_handle,
                             "%s %s: Received heartbeat request from %d on communicator %3d:%d but I am monitored by %d -- this is stall information, ignoring.",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, comm_world_detector.hb_observer );
        return false; /* never forward on the rbcast */
    }
#endif
    OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                         "%s %s: Recveived heartbeat request from %d on communicator %3d:%d",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch));
    comm_world_detector.hb_observer = msg->from;
    fd_heartbeat_send(comm);
    // schedule next
    return false; /* never forward on the rbcast */
}


static int fd_heartbeat_timeout_cb(ompi_communicator_t* comm) {
    double stamp = PMPI_Wtime();


}

