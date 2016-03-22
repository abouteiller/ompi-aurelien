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
#include "opal/threads/threads.h"

#include "ompi/runtime/params.h"
#include "ompi/communicator/communicator.h"
#include "ompi/mca/pml/pml.h"
#include "ompi/mca/bml/bml.h"
#include "ompi/mca/bml/base/base.h"

typedef struct {
    ompi_communicator_t* comm;
    opal_event_t fd_event; /* to trigger timeouts with opal_events */
    int hb_observing; /* the rank of the process we observe */
    int hb_observer; /* the rank of the process that observes us */
    double hb_rstamp; /* the date of the last hb reception */
    double hb_timeout; /* the timeout before we start suspecting observed process as dead (delta) */
    double hb_period; /* the time spacing between heartbeat emission (eta) */
    double hb_sstamp; /* the date at which the last hb emission was done */
    /* caching for RDMA put heartbeats */
    mca_bml_base_btl_t *hb_rdma_bml_btl;
    int hb_rdma_rank; /* my rank */
    mca_btl_base_registration_handle_t* hb_rdma_rank_lreg;
    volatile int hb_rdma_flag; /* set to -1 when read locally, set to observing sets it to its rank through rdma */
    mca_btl_base_registration_handle_t* hb_rdma_flag_lreg;
    uint64_t hb_rdma_raddr; /* write-to remote flag address */
    mca_btl_base_registration_handle_t* hb_rdma_rreg;
} comm_detector_t;

static comm_detector_t comm_world_detector;


typedef struct fd_heartbeat_t {
    ompi_comm_rbcast_message_t super;
    int from;
} ompi_comm_heartbeat_message_t;

typedef struct fd_heartbeat_req_t {
    ompi_comm_rbcast_message_t super;
    int from;
    uint64_t rdma_raddr;
    char rdma_rreg[0];
} ompi_comm_heartbeat_req_t;

static int fd_heartbeat_request(comm_detector_t* detector);
static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_req_t* msg);
static int fd_heartbeat_rdma_put(comm_detector_t* detector);
static int fd_heartbeat_send(comm_detector_t* detector);
static int fd_heartbeat_recv_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_message_t* msg);

static int comm_detector_use_rdma_hb = true;
static double comm_heartbeat_period = 5e-3;
static double comm_heartbeat_timeout = 15e-3;
static opal_event_base_t* fd_event_base = NULL;
static void fd_event_cb(int fd, short flags, void* pdetector);

#if OMPI_ENABLE_THREAD_MULTIPLE
static bool comm_detector_use_thread = true;
static bool fd_thread_active = false;
static opal_thread_t fd_thread;
static void* fd_progress(opal_object_t* obj);
#endif /* OMPI_ENABLE_THREAD_MULTIPLE */

static int comm_heartbeat_recv_cb_type = -1;
static int comm_heartbeat_request_cb_type = -1;

/* rdma btl alignment */
#define ALIGNMENT_MASK(x) ((x) ? (x) - 1 : 0)

int ompi_comm_start_detector(ompi_communicator_t* comm);

int ompi_comm_init_failure_detector(void) {
    int ret;
    bool detect = true;
    fd_event_base = opal_sync_event_base;

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
    (void) mca_base_var_register ("ompi", "mpi", "ft", "detector_rdma_heartbeat",
                                  "Use rdma put to deposit heartbeats into the observer memory",
                                  MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                  OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_READONLY, &comm_detector_use_rdma_hb);
#if OMPI_ENABLE_THREAD_MULTIPLE
    (void) mca_base_var_register ("ompi", "mpi", "ft", "detector_thread",
                                  "Delegate failure detector to a separate thread",
                                  MCA_BASE_VAR_TYPE_BOOL, NULL, 0, 0,
                                  OPAL_INFO_LVL_9, MCA_BASE_VAR_SCOPE_READONLY, &comm_detector_use_thread);
#endif /* OMPI_ENABLE_THREAD_MULTIPLE */
    if( !detect || !ompi_ftmpi_enabled ) return OMPI_SUCCESS;

    /* using rbcast to transmit messages (cb must always return the noforward 'false' flag) */
    /* registering the cb types */
    ret = ompi_comm_rbcast_register_cb_type((ompi_comm_rbcast_cb_t)fd_heartbeat_recv_cb);
    if( 0 > ret ) goto cleanup;
    comm_heartbeat_recv_cb_type = ret;
    ret = ompi_comm_rbcast_register_cb_type((ompi_comm_rbcast_cb_t)fd_heartbeat_request_cb);
    if( 0 > ret ) goto cleanup;
    comm_heartbeat_request_cb_type = ret;

#if OMPI_ENABLE_THREAD_MULTIPLE
    if( comm_detector_use_thread ) {
        fd_event_base = opal_event_base_create();
        if( NULL == fd_event_base ) {
            fd_event_base = opal_sync_event_base;
            ret = OMPI_ERR_OUT_OF_RESOURCE;
            goto cleanup;
        }
        opal_event_use_threads();
        opal_set_using_threads(true);
        OBJ_CONSTRUCT(&fd_thread, opal_thread_t);
        fd_thread.t_run = fd_progress;
        fd_thread.t_arg = NULL;
        ret = opal_thread_start(&fd_thread);
        if( OPAL_SUCCESS != ret ) goto cleanup;
        opal_atomic_rmb();
        while(!fd_thread_active);
        return OMPI_SUCCESS;
    }
    else
#endif /* OMPI_ENABLE_THREAD_MULTIPLE*/
    {
        opal_progress_event_users_increment();

        /* setting up the default detector on comm_world */
        return ompi_comm_start_detector(&ompi_mpi_comm_world.comm);
    }
  cleanup:
    ompi_comm_finalize_failure_detector();
    return ret;
}

int ompi_comm_finalize_failure_detector(void) {
    int ret;

#if OMPI_ENABLE_THREAD_MULTIPLE
    if( fd_thread_active ) {
        void* tret;
        fd_thread_active = false;
        opal_event_base_loopbreak(fd_event_base);
        opal_thread_join(&fd_thread, &tret);
    }
    if( opal_sync_event_base != fd_event_base ) opal_event_base_free(fd_event_base);
#endif /* MPI_ENABLE_THREAD_MULTIPLE */
    if( -1 != comm_heartbeat_recv_cb_type ) ompi_comm_rbcast_unregister_cb_type(comm_heartbeat_recv_cb_type);
    if( -1 != comm_heartbeat_request_cb_type ) ompi_comm_rbcast_unregister_cb_type(comm_heartbeat_request_cb_type);
    comm_heartbeat_recv_cb_type = comm_heartbeat_request_cb_type = -1;
    return OMPI_SUCCESS;
}

int ompi_comm_start_detector(ompi_communicator_t* comm) {
    if( comm != &ompi_mpi_comm_world.comm ) return OMPI_SUCCESS; /* TODO: not implemented for other comms yet */
    comm_detector_t* detector = &comm_world_detector;

    int rank, np;
    detector->comm = comm;
    np = ompi_comm_size(comm);
    rank = ompi_comm_rank(comm);
    detector->hb_observing = (np+rank-1) % np;
    detector->hb_observer = (np+rank+1) % np;
    detector->hb_period = comm_heartbeat_period;
    detector->hb_timeout = comm_heartbeat_timeout;
    detector->hb_sstamp = 0.;
    detector->hb_rstamp = PMPI_Wtime()+(double)np; /* give some slack for MPI_Init */

    detector->hb_rdma_bml_btl = NULL;
    detector->hb_rdma_rank = rank;
    detector->hb_rdma_rank_lreg = NULL;
    detector->hb_rdma_flag_lreg = NULL;
    detector->hb_rdma_flag = -1;
    detector->hb_rdma_raddr = 0;
    detector->hb_rdma_rreg = NULL;

    if( comm_detector_use_rdma_hb ) {
        detector->hb_rdma_flag = -3;
        fd_heartbeat_request(detector);
    }

    opal_event_set(fd_event_base, &detector->fd_event, -1, OPAL_EV_TIMEOUT | OPAL_EV_PERSIST, fd_event_cb, detector);
    struct timeval tv;
    tv.tv_sec = (int)detector->hb_period;
    tv.tv_usec = (-tv.tv_sec + detector->hb_period) * 1e6;
    opal_event_add(&detector->fd_event, &tv);
    return OMPI_SUCCESS;
}

static int fd_heartbeat_request(comm_detector_t* detector) {
    assert( -1 != comm_heartbeat_request_cb_type /* initialized */);
    ompi_communicator_t* comm = detector->comm;
    if( &ompi_mpi_comm_world.comm != comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    if( -3 != detector->hb_rdma_flag /* initialization */
     && ompi_comm_is_proc_active(comm, detector->hb_observing, OMPI_COMM_IS_INTER(comm)) ) {
        /* already observing a live process, so nothing to do. */
        return OMPI_SUCCESS;
    }

    int ret;
    int nb = ompi_comm_size(comm);
    int rank;
    size_t regsize = 0;

    for( rank = (nb+detector->hb_observing) % nb;
         rank != comm->c_my_rank;
         rank = (nb+rank-1) % nb ) {
        ompi_proc_t* proc = ompi_comm_peer_lookup(comm, rank);
        assert( NULL != proc );
        if( !ompi_proc_is_active(proc) ) continue;

        detector->hb_rstamp = PMPI_Wtime();
        OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                             "%s %s: Sending observe request to %d on communicator %3d:%d stamp %g",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, rank, comm->c_contextid, comm->c_epoch, detector->hb_rstamp ));

        if( comm_detector_use_rdma_hb ) {
            mca_bml_base_endpoint_t* endpoint = mca_bml_base_get_endpoint(proc);
            assert( NULL != endpoint );
            mca_bml_base_btl_t *bml_btl = mca_bml_base_btl_array_get_index(&endpoint->btl_rdma, 0);
            assert( NULL != bml_btl );

            /* register mem for the flag and cache the reg key */
            if( NULL != bml_btl->btl->btl_register_mem ) {
                if( NULL != detector->hb_rdma_flag_lreg ) {
                    mca_bml_base_deregister_mem(bml_btl, detector->hb_rdma_flag_lreg);
                }
                assert( !((size_t)&detector->hb_rdma_flag & ALIGNMENT_MASK(bml_btl->btl->btl_put_alignment)) );
                mca_bml_base_register_mem(bml_btl, (void*)&detector->hb_rdma_flag, sizeof(int),
                        MCA_BTL_REG_FLAG_LOCAL_WRITE | MCA_BTL_REG_FLAG_REMOTE_WRITE, &detector->hb_rdma_flag_lreg);
                assert( NULL != detector->hb_rdma_flag_lreg );
                regsize = bml_btl->btl->btl_registration_handle_size;
            }
        }

        ompi_comm_heartbeat_req_t* msg = calloc(sizeof(*msg)+regsize, 1);
        msg->super.cid = comm->c_contextid;
        msg->super.epoch = comm->c_epoch;
        msg->super.type = comm_heartbeat_request_cb_type;
        msg->from = comm->c_my_rank;
        if( comm_detector_use_rdma_hb ) {
            /* send the rdma addr and registration key to the observed */
            memcpy(&msg->rdma_rreg[0], detector->hb_rdma_flag_lreg, regsize);
            msg->rdma_raddr = (uint64_t)&detector->hb_rdma_flag;
        }
        ret = ompi_comm_rbcast_send_msg(proc, &msg->super, sizeof(*msg)+regsize);
        free(msg);
        if( OMPI_SUCCESS == ret ) break;
        /* mark this process dead and forward */
        ompi_errmgr_mark_failed_peer_cause_heartbeat(proc);
    }
    /* if everybody else is dead, then it's a success */
    detector->hb_observing = rank;
    detector->hb_rstamp = PMPI_Wtime()+detector->hb_timeout; /* we add one timeout slack to account for the send time */
    return OMPI_SUCCESS;
}

static int fd_heartbeat_request_cb(ompi_communicator_t* comm, ompi_comm_heartbeat_req_t* msg) {
    assert( &ompi_mpi_comm_world.comm == comm );
    comm_detector_t* detector = &comm_world_detector;

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
    OPAL_OUTPUT_VERBOSE((1, ompi_ftmpi_output_handle,
                         "%s %s: Recveived heartbeat request from %d on communicator %3d:%d",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch));
//  OPAL_MUTEX_LOCK(&detector_mutex);
    detector->hb_observer = msg->from;
    detector->hb_sstamp = 0.;

    if( comm_detector_use_rdma_hb ) {
        ompi_communicator_t* comm = detector->comm;
        ompi_proc_t* proc = ompi_comm_peer_lookup(comm, msg->from);
        assert( NULL != proc );
        mca_bml_base_endpoint_t* endpoint = mca_bml_base_get_endpoint(proc);
        assert( NULL != endpoint );
        mca_bml_base_btl_t *bml_btl = mca_bml_base_btl_array_get_index(&endpoint->btl_rdma, 0);
        assert( NULL != bml_btl );

        /* registration for the local rank */
        if( NULL != bml_btl->btl->btl_register_mem ) {
            if( NULL != detector->hb_rdma_rank_lreg ) {
                mca_bml_base_deregister_mem(detector->hb_rdma_bml_btl, detector->hb_rdma_rank_lreg);
            }
            assert( !((size_t)&detector->hb_rdma_rank & ALIGNMENT_MASK(bml_btl->btl->btl_put_alignment)) );
            mca_bml_base_register_mem(bml_btl, &detector->hb_rdma_rank, sizeof(int), 0, &detector->hb_rdma_rank_lreg);
            assert( NULL != detector->hb_rdma_rank_lreg );
            /* registration for the remote flag */
            if( NULL != detector->hb_rdma_rreg ) free(detector->hb_rdma_rreg);
            size_t regsize = bml_btl->btl->btl_registration_handle_size;
            detector->hb_rdma_rreg = malloc(regsize);
            assert( NULL != detector->hb_rdma_rreg );
            memcpy(detector->hb_rdma_rreg, &msg->rdma_rreg[0], regsize);
        }
        /* cache the bml_btl used for put */
        detector->hb_rdma_bml_btl = bml_btl;
        /* remote flag addr */
        detector->hb_rdma_raddr = msg->rdma_raddr;
    }

    fd_heartbeat_send(detector);
    return false; /* never forward on the rbcast */
}

/*
 * event loop and thread
 */

static void fd_event_cb(int fd, short flags, void* pdetector) {
    double stamp = PMPI_Wtime();
    comm_detector_t* detector = pdetector;;
    struct timeval tv;

    if( (stamp - detector->hb_sstamp) >= detector->hb_period ) {
        fd_heartbeat_send(detector);
    }

    if( comm_detector_use_rdma_hb ) {
        int flag = detector->hb_rdma_flag;

        if( -1 > flag ) {
            /* still initializing after MPI_INIT, give extra slack */
            int np = ompi_comm_size(detector->comm);
            if( (stamp - detector->hb_rstamp) < (detector->hb_timeout + (double)np) )
               return;
        }
        if( 0 <= flag ) {
            /* it's alright, we have received stamps since last time we checked */
            OPAL_OUTPUT_VERBOSE((10, ompi_ftmpi_output_handle,
                   "%s %s: evtimer triggered at stamp %g, RDMA recv grace %.1e is OK from %d :)",
                   OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                   stamp, stamp - detector->hb_rstamp, flag));
            if( flag != detector->hb_observing ) {
                opal_output_verbose(1, ompi_ftmpi_output_handle,
                   "%s %s: evtimer triggered at stamp %g, this is a rdma heartbeat from %d, but I am now observing %d, acting as-if this was a valid heartbeat",
                   OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                   flag, detector->hb_observing);
            }
            detector->hb_rdma_flag = -1;
            detector->hb_rstamp = stamp;
            return;
        }
    }

    if( (stamp - detector->hb_rstamp) > detector->hb_timeout ) {
       /* this process is now suspected dead. */
        opal_output_verbose(1, ompi_ftmpi_output_handle,
                        "%s %s: evtimer triggered at stamp %g, recv grace MISSED by %.1e, proc %d now suspected dead.",
                        OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                        stamp,
                        detector->hb_timeout - (stamp - detector->hb_rstamp), detector->hb_observing);
        ompi_proc_t* proc = ompi_comm_peer_lookup(detector->comm, detector->hb_observing);
        /* mark this process dead and forward */
        ompi_errmgr_mark_failed_peer_cause_heartbeat(proc);
        /* change the observed proc */
        detector->hb_rdma_flag = -2;
        fd_heartbeat_request(detector);
    }
}

#if OMPI_ENABLE_THREAD_MULTIPLE
void* fd_progress(opal_object_t* obj) {
    int ret;
    MPI_Request req;
    if( OMPI_SUCCESS != ompi_comm_start_detector(&ompi_mpi_comm_world.comm)) {
        fd_thread_active = true;
        return OPAL_THREAD_CANCELLED;
    }
    fd_thread_active = true;
    ret = MCA_PML_CALL(irecv(NULL, 0, MPI_BYTE, 0, MCA_COLL_BASE_TAG_FT_END, &ompi_mpi_comm_self.comm, &req));
    opal_atomic_rmb();
    while( fd_thread_active ) {
        if( 0 == comm_world_detector.hb_rdma_raddr ) { /* if RDMA hb not setup yet */
            /* force rbcast recv to progress */
            int completed = 0;
            ret = ompi_request_test(&req, &completed, MPI_STATUS_IGNORE);
            assert( OMPI_SUCCESS == ret );
            assert( 0 == completed );
        }
        opal_event_loop(fd_event_base, OPAL_EVLOOP_ONCE);
    }
    ret = ompi_request_cancel(req);
    ret = ompi_request_wait(&req, MPI_STATUS_IGNORE);
    return OPAL_THREAD_CANCELLED;
}
#endif /* OMPI_ENABLE_THREAD_MULTIPLE */


static int sendseq = 0;

/*
 * RDMA put based heartbeats
 */

static void fd_heartbeat_rdma_cb(mca_btl_base_module_t* btl, struct mca_btl_base_endpoint_t* ep,
                       void *local_address, mca_btl_base_registration_handle_t *local_handle,
                       void *context, void *cbdata, int status) {
    OPAL_OUTPUT_VERBOSE((100, ompi_ftmpi_output_handle,
                        "%s %s: rdma_hb_cb status=%d",
                        OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                        status));
}

static int fd_heartbeat_rdma_put(comm_detector_t* detector) {
    int ret = OMPI_SUCCESS;

    if( 0 == detector->hb_rdma_raddr ) return OMPI_SUCCESS; /* not initialized yet */
    do {
        ret = mca_bml_base_put(detector->hb_rdma_bml_btl, &detector->hb_rdma_rank, detector->hb_rdma_raddr,
                               detector->hb_rdma_rank_lreg, detector->hb_rdma_rreg,
                               sizeof(int), 0, MCA_BTL_NO_ORDER, fd_heartbeat_rdma_cb, NULL);
        OPAL_OUTPUT_VERBOSE((100, ompi_ftmpi_output_handle,
                        "%s %s: bml_put sendseq=%d, rc=%d",
                        OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__,
                        sendseq++, ret));
    } while( OMPI_ERR_OUT_OF_RESOURCE == ret ); /* never give up... */
    assert( (OMPI_SUCCESS == ret) || (1 == ret) ); // TODO: fallback to sendi
    return ret;
}


/*
 * send eager based heartbeats
 */

static int fd_heartbeat_send(comm_detector_t* detector) {
    assert( -1 != comm_heartbeat_recv_cb_type /* initialized */);
    ompi_communicator_t* comm = detector->comm;
    if( comm != &ompi_mpi_comm_world.comm ) return OMPI_ERR_NOT_IMPLEMENTED;

    double now = PMPI_Wtime();
    if( 0. != detector->hb_sstamp
     && (now - detector->hb_sstamp) >= 2.*detector->hb_period ) {
        opal_output_verbose(1, ompi_ftmpi_output_handle, "MISSED my SEND %d deadline by %.1e, this could trigger a false suspicion for me.", sendseq, now-detector->hb_sstamp);
    }
    detector->hb_sstamp = now;
    OPAL_OUTPUT_VERBOSE((9, ompi_ftmpi_output_handle,
                         "%s %s: Sending heartbeat to %d on communicator %3d:%d stamp %g",
                         OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, detector->hb_observer, comm->c_contextid, comm->c_epoch, detector->hb_sstamp ));

    if( comm_detector_use_rdma_hb ) return fd_heartbeat_rdma_put(detector);

    /* send the heartbeat with eager send */
    ompi_comm_heartbeat_message_t msg;
    msg.super.cid = comm->c_contextid;
    msg.super.epoch = comm->c_epoch;
    msg.super.type = comm_heartbeat_recv_cb_type;
    msg.from = comm->c_my_rank;
    ompi_proc_t* proc = ompi_comm_peer_lookup(comm, detector->hb_observer);
    ompi_comm_rbcast_send_msg(proc, &msg.super, sizeof(msg));
    return OMPI_SUCCESS;
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
                             "%s %s: Recveived heartbeat from %d on communicator %3d:%d at timestamp %g (remained %.1e of %.1e before suspecting)",
                             OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, msg->from, comm->c_contextid, comm->c_epoch, stamp, grace, detector->hb_timeout ));
        detector->hb_rstamp = stamp;
        if( grace < 0.0 ) {
            opal_output(ompi_ftmpi_output_handle,
                        "%s %s: MISSED ( %.1e )",
                        OMPI_NAME_PRINT(OMPI_PROC_MY_NAME), __func__, grace );
            // complain? This is indicative of wrong assumptions on the
            // timeout and possibly imperfect detector suspecting live processes
        }
    }
    return false; /* never forward on the rbcast */
}


