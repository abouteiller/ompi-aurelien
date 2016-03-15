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
#include "opal/mca/timer/base/base.h"

#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/mca/pml/pml.h"

typedef struct {
    ompi_communicator_t* comm;
    int hb_observing; /* the rank of the process we observe */
    int hb_observer; /* the rank of the process that observes us */
    double hb_period; /* the time spacing between heartbeat emission (eta) */
    double hb_timeout; /* the timeout before we start suspecting observed process as dead (delta) */
    double hb_sstamp; /* the date at which the last hb emission was done */
    double hb_rstamp; /* the date of the last hb reception */
    opal_event_t fd_event; /* to trigger timeouts with opal_events */
} comm_detector_t;

static comm_detector_t comm_world_detector;


typedef struct fd_heartbeat_t {
    ompi_comm_rbcast_message_t super;
    int from;
} ompi_comm_heartbeat_message_t;

static int fd_heartbeat_send(comm_detector_t* detector);
static int fd_heartbeat_recv_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg);
static int fd_heartbeat_request(comm_detector_t* detector);
static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg);

static double comm_heartbeat_period = 5e-5;
static double comm_heartbeat_timeout = 1e-3;
static opal_event_base_t* fd_event_base = NULL;
static void fd_event_cb(int fd, short flags, void* pdetector);

static int comm_heartbeat_recv_cb_type = -1;
static int comm_heartbeat_request_cb_type = -1;


int ompi_comm_start_detector(ompi_communicator_t* comm);

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

    // TODO: if using threads, change the event base
    fd_event_base = opal_sync_event_base;
    /* setting up the default detector on comm_world */
    return ompi_comm_start_detector(&ompi_mpi_comm_world.comm);

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

int ompi_comm_start_detector(ompi_communicator_t* comm) {
    if( comm != &ompi_mpi_comm_world.comm ) return OMPI_SUCCESS; /* TODO: not implemented for other comms yet */

    int rank, np;
    comm_world_detector.comm = comm;
    np = ompi_comm_size(comm);
    rank = ompi_comm_rank(comm);
    comm_world_detector.hb_observing = (np+rank-1) % np;
    comm_world_detector.hb_observer = (np+rank+1) % np;
    comm_world_detector.hb_period = comm_heartbeat_period;
    comm_world_detector.hb_timeout = comm_heartbeat_timeout;
    comm_world_detector.hb_sstamp = 0;
    comm_world_detector.hb_rstamp = PMPI_Wtime()+1e3*comm_heartbeat_timeout;

    opal_event_evtimer_set(fd_event_base, &comm_world_detector.fd_event, fd_event_cb, &comm_world_detector);
    fd_event_cb(-1, 0, &comm_world_detector); /* start the events */
    return OMPI_SUCCESS;
}

static int fd_heartbeat_send(comm_detector_t* detector) {
    assert( -1 != comm_heartbeat_recv_cb_type /* initialized */);
    ompi_communicator_t* comm = detector->comm;
    if( comm != &ompi_mpi_comm_world.comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    detector->hb_sstamp = PMPI_Wtime();
    OPAL_OUTPUT_VERBOSE((9, ompi_ftmpi_output_handle,
                         "%s %s: Sending heartbeat to %d on communicator %3d:%d stamp %g",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, detector->hb_observer, comm->c_contextid, comm->c_epoch, detector->hb_sstamp ));

    /* rearm the timer for next time */
    struct timeval tv;
    tv.tv_sec = (int)detector->hb_period;
    tv.tv_usec = (int)((detector->hb_period-(double)tv.tv_sec)*1e6);
    opal_event_add(&detector->fd_event, &tv);

    /* send the heartbeat */
    ompi_comm_heartbeat_message_t msg;
    msg.super.cid = comm->c_contextid;
    msg.super.epoch = comm->c_epoch;
    msg.super.type = comm_heartbeat_recv_cb_type;
    msg.from = comm->c_my_rank;
    ompi_proc_t* proc = ompi_comm_peer_lookup(comm, detector->hb_observer);
    return ompi_comm_rbcast_send_msg(proc, &msg.super, sizeof(msg));
}

static int fd_heartbeat_recv_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg) {
    assert( &ompi_mpi_comm_world.comm == comm );
    comm_detector_t* detector = &comm_world_detector;

    if( msg->from != detector->hb_observing ) {
        OPAL_OUTPUT_VERBOSE((2, ompi_ftmpi_output_handle,
                             "%s %s: Received heartbeat from %d on communicator %3d:%d but I am now monitoring %d -- ignored.",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, detector->hb_observing ));
    }
    else {
        double stamp = PMPI_Wtime();
        double grace = detector->hb_timeout - (stamp - detector->hb_rstamp);
        OPAL_OUTPUT_VERBOSE((9, ompi_ftmpi_output_handle,
                             "%s %s: Recveived heartbeat from %d on communicator %3d:%d at timestamp %g (remained %e of %e before suspecting)",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, stamp, grace, detector->hb_timeout ));
        if( grace > 0.0 ) {
            detector->hb_rstamp = stamp;
        }
        // else complain? This is indicative of wrong assumptions on the
        // timeout and possibly imperfect detector suspecting live processes
    }
    return false; /* never forward on the rbcast */
}


static int fd_heartbeat_request(comm_detector_t* detector) {
    assert( -1 != comm_heartbeat_request_cb_type /* initialized */);
    ompi_communicator_t* comm = detector->comm;
    if( &ompi_mpi_comm_world.comm != comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    /* already observing a live process, so nothing to do. */
    if( ompi_comm_is_proc_active(comm, detector->hb_observing, OMPI_COMM_IS_INTER(comm)) ) return OMPI_SUCCESS;

    int ret;
    int nb = ompi_comm_size(comm);
    int rank;

    for( rank = (nb+detector->hb_observing-1) % nb;
         rank != comm->c_my_rank;
         rank = (nb+rank-1) % nb ) {
        if( !ompi_comm_is_proc_active(comm, rank, OMPI_COMM_IS_INTER(comm)) ) continue;

        detector->hb_rstamp = PMPI_Wtime();
        OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                             "%s %s: Sending observe request to %d on communicator %3d:%d stamp %g",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, rank, comm->c_contextid, comm->c_epoch, detector->hb_rstamp ));

        ompi_comm_heartbeat_message_t msg;
        msg.super.cid = comm->c_contextid;
        msg.super.epoch = comm->c_epoch;
        msg.super.type = comm_heartbeat_request_cb_type;
        msg.from = comm->c_my_rank;
        ompi_proc_t* proc = ompi_comm_peer_lookup(comm, rank);
        ret = ompi_comm_rbcast_send_msg(proc, &msg.super, sizeof(msg));
        if( OMPI_SUCCESS == ret ) break;
    }
    /* if everybody else is dead, then it's a success */
    detector->hb_observing = rank;
    detector->hb_rstamp = PMPI_Wtime()+detector->hb_timeout; /* we add one timeout slack to account for the send time */
    return OMPI_SUCCESS;
}

static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg) {
    assert( &ompi_mpi_comm_world.comm == comm );
    comm_detector_t* detector = &comm_world_detector;

#if OPAL_ENABLE_DEBUG
    int nb, rr, ro;
    nb = ompi_comm_size(comm);
    rr = (nb-comm->c_my_rank+msg->from) % nb; /* translate msg->from in circular space so that myrank==0 */
    ro = (nb-comm->c_my_rank+detector->hb_observer) % nb; /* same for the observer rank */
    if( rr < ro ) {
        opal_output_verbose(1, ompi_ftmpi_output_handle,
                             "%s %s: Received heartbeat request from %d on communicator %3d:%d but I am monitored by %d -- this is stall information, ignoring.",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, detector->hb_observer );
        return false; /* never forward on the rbcast */
    }
#endif
    OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                         "%s %s: Recveived heartbeat request from %d on communicator %3d:%d",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch));
    detector->hb_observer = msg->from;
    opal_event_del(&detector->fd_event);
    fd_heartbeat_send(detector);
    return false; /* never forward on the rbcast */
}

static void fd_event_cb(int fd, short flags, void* pdetector) {
    double stamp = PMPI_Wtime();
    comm_detector_t* detector = pdetector;;
    struct timeval tv;

    OPAL_OUTPUT_VERBOSE((10, ompi_ftmpi_output_handle,
                        "%s %s: evtimer triggered at stamp %g, send grace is %e, recv grace is %e",
                        OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                        stamp,
                        detector->hb_period - (stamp - detector->hb_sstamp),
                        detector->hb_timeout - (stamp - detector->hb_rstamp)));

    if( (stamp - detector->hb_sstamp) >= detector->hb_period) {
        fd_heartbeat_send(detector);
    }
    else {
        tv.tv_sec = (int)detector->hb_period;
        tv.tv_usec = (int)((detector->hb_period-(double)tv.tv_sec)*1e6);
        opal_event_add(&detector->fd_event, &tv);
    }

    if( (stamp - detector->hb_rstamp) > detector->hb_timeout ) {
        /* this process is now suspected dead. */
        ompi_proc_t* proc = ompi_comm_peer_lookup(detector->comm, detector->hb_observing);
        /* mark this process dead and forward */
        ompi_errmgr_mark_failed_peer_cause_heartbeat(proc);
        /* change the observed proc */
        fd_heartbeat_request(detector);
    }
}
