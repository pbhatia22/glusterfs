/*
   Copyright (c) 2006-2013 Red Hat, Inc. <http://www.redhat.com>
   This file is part of GlusterFS.

   This file is licensed to you under your choice of the GNU Lesser
   General Public License, version 3 or any later version (LGPLv3 or
   later), or the GNU General Public License, version 2 (GPLv2), in all
   cases as published by the Free Software Foundation.
*/
#ifndef _GLUSTERD_H_
#define _GLUSTERD_H_

#include <sys/types.h>
#include <pthread.h>
#include <libgen.h>

#include <glusterfs/compat-uuid.h>

#include <glusterfs/logging.h>
#include "glusterd-mem-types.h"
#include "glusterd-sm.h"
#include "glusterd-snapd-svc.h"
#include "glusterd-shd-svc.h"
#include "glusterd1-xdr.h"
#include "glusterd-pmap.h"
#include "cli1-xdr.h"
#include <glusterfs/syncop.h>
#include <glusterfs/events.h>
#include "glusterd-gfproxyd-svc.h"

/* Threading limits for glusterd event threads. */
#define GLUSTERD_MIN_EVENT_THREADS 1
#define GLUSTERD_MAX_EVENT_THREADS 32

#define GLUSTERD_TR_LOG_SIZE 50
#define GLUSTERD_QUORUM_TYPE_KEY "cluster.server-quorum-type"
#define GLUSTERD_QUORUM_RATIO_KEY "cluster.server-quorum-ratio"
#define GLUSTERD_GLOBAL_OPT_VERSION "global-option-version"
#define GLUSTERD_GLOBAL_OP_VERSION_KEY "cluster.op-version"
#define GLUSTERD_MAX_OP_VERSION_KEY "cluster.max-op-version"
#define GEO_CONF_MAX_OPT_VALS 6
#define GLUSTER_SHARED_STORAGE "gluster_shared_storage"
#define GLUSTERD_SHARED_STORAGE_KEY "cluster.enable-shared-storage"
#define GLUSTERD_BRICK_MULTIPLEX_KEY "cluster.brick-multiplex"
#define GLUSTER_BRICK_GRACEFUL_CLEANUP "cluster.brick-graceful-cleanup"
#define GLUSTERD_VOL_CNT_PER_THRD "glusterd.vol_count_per_thread"
#define GLUSTERD_BRICKMUX_LIMIT_KEY "cluster.max-bricks-per-process"
#define GLUSTERD_BRICKMUX_LIMIT_DFLT_VALUE "250"
#define GLUSTERD_VOL_CNT_PER_THRD_DEFAULT_VALUE "100"
#define GLUSTERD_LOCALTIME_LOGGING_KEY "cluster.localtime-logging"
#define GLUSTERD_DAEMON_LOG_LEVEL_KEY "cluster.daemon-log-level"

#define STRIPE_COUNT 1
#define FMTSTR_CHECK_VOL_EXISTS "Volume %s does not exist"
#define FMTSTR_RESOLVE_BRICK "Could not find peer on which brick %s:%s resides"

#define LOGSTR_FOUND_BRICK "Found brick %s:%s in volume %s"
#define LOGSTR_BUILD_PAYLOAD "Failed to build payload for operation 'Volume %s'"
#define LOGSTR_STAGE_FAIL "Staging of operation 'Volume %s' failed on %s %s %s"
#define LOGSTR_COMMIT_FAIL "Commit of operation 'Volume %s' failed on %s %s %s"

#define OPERRSTR_BUILD_PAYLOAD                                                 \
    "Failed to build payload. Please check the log "                           \
    "file for more details."
#define OPERRSTR_STAGE_FAIL                                                    \
    "Staging failed on %s. Please check the log file "                         \
    "for more details."
#define OPERRSTR_COMMIT_FAIL                                                   \
    "Commit failed on %s. Please check the log file "                          \
    "for more details."
struct glusterd_volinfo_;
typedef struct glusterd_volinfo_ glusterd_volinfo_t;

struct glusterd_snap_;
typedef struct glusterd_snap_ glusterd_snap_t;

/* For every new feature please add respective enum of new feature
 * at the end of latest enum (just before the GD_OP_MAX enum)
 */
typedef enum glusterd_op_ {
    GD_OP_NONE = 0,
    GD_OP_CREATE_VOLUME,
    GD_OP_START_BRICK,
    GD_OP_STOP_BRICK,
    GD_OP_DELETE_VOLUME,
    GD_OP_START_VOLUME,
    GD_OP_STOP_VOLUME,
    GD_OP_DEFRAG_VOLUME,
    GD_OP_ADD_BRICK,
    GD_OP_REMOVE_BRICK,
    GD_OP_REPLACE_BRICK,
    GD_OP_SET_VOLUME,
    GD_OP_RESET_VOLUME,
    GD_OP_SYNC_VOLUME,
    GD_OP_LOG_ROTATE,
    GD_OP_GSYNC_SET,
    GD_OP_PROFILE_VOLUME,
    GD_OP_QUOTA,
    GD_OP_STATUS_VOLUME,
    GD_OP_REBALANCE,
    GD_OP_HEAL_VOLUME,
    GD_OP_STATEDUMP_VOLUME,
    GD_OP_LIST_VOLUME,
    GD_OP_CLEARLOCKS_VOLUME,
    GD_OP_DEFRAG_BRICK_VOLUME,
    GD_OP_COPY_FILE,
    GD_OP_SYS_EXEC,
    GD_OP_GSYNC_CREATE,
    GD_OP_SNAP,
    GD_OP_BARRIER,
    GD_OP_GANESHA, /* obsolete */
    GD_OP_BITROT,
    GD_OP_DETACH_TIER,
    GD_OP_TIER_MIGRATE,
    GD_OP_SCRUB_STATUS,
    GD_OP_SCRUB_ONDEMAND,
    GD_OP_RESET_BRICK,
    GD_OP_MAX_OPVERSION,
    GD_OP_TIER_START_STOP,
    GD_OP_TIER_STATUS,
    GD_OP_DETACH_TIER_STATUS,
    GD_OP_DETACH_NOT_STARTED,
    GD_OP_REMOVE_TIER_BRICK,
    GD_OP_ADD_TIER_BRICK,
    GD_OP_MAX,
} glusterd_op_t;

extern const char *gd_op_list[];

/* Keeping all the paths required in glusterd would
   cause many buffer overflow errors, as we append
   more defined paths to the brick path, workdir etc etc.
   It is better to keep limit on this as lesser value,
   so we get an option to continue with all functionalities.
   For example, snapname max would be appended on brick-path and
   would be stored on workdir... all of these being PATH_MAX, is
   not an ideal situation for success. */
#define VALID_GLUSTERD_PATHMAX (PATH_MAX - (256 + 64))

typedef struct {
    struct _volfile_ctx *volfile;
    pthread_mutex_t mutex;
    struct cds_list_head peers;
    uuid_t uuid;
    rpcsvc_t *rpc;
    glusterd_svc_t nfs_svc;
    glusterd_svc_t bitd_svc;
    glusterd_svc_t scrub_svc;
    glusterd_svc_t quotad_svc;
    struct pmap_registry *pmap;
    struct cds_list_head volumes;
    struct cds_list_head snapshots;   /*List of snap volumes */
    struct cds_list_head brick_procs; /* List of brick processes */
    struct cds_list_head shd_procs;   /* List of shd processes */
    pthread_mutex_t xprt_lock;
    struct list_head xprt_list;
    pthread_mutex_t import_volumes;
    gf_store_handle_t *handle;
    glusterd_sm_tr_log_t op_sm_log;
    struct rpc_clnt_program *gfs_mgmt;
    dict_t *mgmt_v3_lock;        /* Dict for saving
                                  * mgmt_v3 locks */
    dict_t *glusterd_txn_opinfo; /* Dict for saving
                                  * transaction opinfos */
    uuid_t global_txn_id;        /* To be used in
                                  * heterogeneous
                                  * cluster with no
                                  * transaction ids */

    dict_t *mgmt_v3_lock_timer;
    struct cds_list_head mount_specs;
    void *hooks_priv;

    xlator_t *xl; /* Should be set to 'THIS' before creating thread */
    /* need for proper handshake_t */
    int op_version; /* Starts with 1 for 3.3.0 */
    gf_boolean_t pending_quorum_action;
    gf_boolean_t restart_done;
    dict_t *opts;
    synclock_t big_lock;
    synccond_t cond_restart_bricks;
    synccond_t cond_restart_shd;
    synccond_t cond_blockers;
    rpcsvc_t *uds_rpc; /* RPCSVC for the unix domain socket */
    uint32_t base_port;
    uint32_t max_port;
    gf_store_handle_t *missed_snaps_list_shandle;
    struct cds_list_head missed_snaps_list;
    time_t ping_timeout;
    uint32_t generation;
    int32_t workers;
    time_t mgmt_v3_lock_timeout;
    gf_atomic_t blockers;
    pthread_mutex_t attach_lock; /* Lock can be per process or a common one */
    pthread_mutex_t volume_lock; /* We release the big_lock from lot of places
                                    which might lead the modification of volinfo
                                    list.
                                 */
    gf_atomic_t thread_count;
    gf_boolean_t restart_bricks;
    gf_boolean_t restart_shd; /* This flag prevents running two shd manager
                                 simultaneously
                              */
    char workdir[VALID_GLUSTERD_PATHMAX];
    char rundir[VALID_GLUSTERD_PATHMAX];
    char logdir[VALID_GLUSTERD_PATHMAX];
    struct list_head hostnames;
    struct list_head remote_hostnames;
} glusterd_conf_t;

typedef enum gf_brick_status {
    GF_BRICK_STOPPED,
    GF_BRICK_STARTED,
    GF_BRICK_STOPPING,
    GF_BRICK_STARTING
} gf_brick_status_t;

typedef struct glusterd_brickinfo glusterd_brickinfo_t;

struct glusterd_brick_proc {
    int port;
    uint32_t brick_count;
    struct cds_list_head brick_proc_list;
    struct cds_list_head bricks;
};

typedef struct glusterd_brick_proc glusterd_brick_proc_t;

struct glusterd_snap_ops;

struct glusterd_brickinfo {
    struct cds_list_head brick_list;
    uuid_t uuid;
    int port;
    int rdma_port;
    char *logfile;
    gf_store_handle_t *shandle;
    struct rpc_clnt *rpc;
    int decommissioned;
    gf_brick_status_t status;
    int32_t snap_status;
    struct glusterd_snap_ops *snap;
    /*
     * The group is used to identify which bricks are part of the same
     * replica set during brick-volfile generation, so that JBR volfiles
     * can "cross-connect" the bricks to one another. It is also used by
     * AFR to load the arbiter xlator in the appropriate brick in case of
     * a replica 3 volume with arbiter enabled.
     */
    uint16_t group;
    gf_boolean_t port_registered;
    gf_boolean_t start_triggered;

    /* Below are used for handling the case of multiple bricks sharing
       the backend filesystem */
    uint64_t statfs_fsid;
    pthread_mutex_t restart_mutex;
    glusterd_brick_proc_t *brick_proc; /* Information regarding mux bricks */
    struct cds_list_head mux_bricks; /* List to store the bricks in brick_proc*/
    uint32_t fs_share_count;
    char hostname[NAME_MAX];
    char path[VALID_GLUSTERD_PATHMAX];
    char real_path[VALID_GLUSTERD_PATHMAX];
    char device_path[VALID_GLUSTERD_PATHMAX];
    char origin_path[VALID_GLUSTERD_PATHMAX];
    char mount_dir[VALID_GLUSTERD_PATHMAX];
    char brick_id[1024];      /*Client xlator name, AFR changelog name*/
    char fstype[NAME_MAX];    /* Brick file-system type */
    char snap_type[NAME_MAX]; /* Brick snapshot type */
    char mnt_opts[1024];      /* Brick mount options */
    char vg[PATH_MAX];        /* FIXME: Use max size for length of vg */
};

typedef int (*defrag_cbk_fn_t)(glusterd_volinfo_t *volinfo,
                               gf_defrag_status_t status);

struct glusterd_defrag_info_ {
    int refcnt;
    gf_lock_t lock;
    gf_boolean_t connected;
    struct rpc_clnt *rpc;
    defrag_cbk_fn_t cbk_fn;
};

typedef struct glusterd_defrag_info_ glusterd_defrag_info_t;

typedef enum gf_transport_type_ {
    GF_TRANSPORT_TCP,  // DEFAULT
    GF_TRANSPORT_RDMA,
    GF_TRANSPORT_BOTH_TCP_RDMA,
} gf_transport_type;

typedef struct _auth {
    char *username;
    char *password;
} auth_t;

struct glusterd_rebalance_ {
    uint64_t rebalance_files;
    uint64_t rebalance_data;
    uint64_t lookedup_files;
    uint64_t skipped_files;
    uint64_t rebalance_failures;
    glusterd_defrag_info_t *defrag;
    gf_defrag_type_t defrag_cmd;
    gf_defrag_status_t defrag_status;
    uuid_t rebalance_id;
    double rebalance_time;
    time_t time_left;
    dict_t *dict; /* Dict to store misc information
                   * like list of bricks being removed */
    glusterd_op_t op;
    uint32_t commit_hash;
};

typedef struct glusterd_rebalance_ glusterd_rebalance_t;

struct glusterd_replace_brick_ {
    glusterd_brickinfo_t *src_brick;
    glusterd_brickinfo_t *dst_brick;
};

typedef struct glusterd_replace_brick_ glusterd_replace_brick_t;

typedef enum gd_quorum_status_ {
    NOT_APPLICABLE_QUORUM,  // Does not follow quorum
    MEETS_QUORUM,           // Follows quorum and meets.
    DOESNT_MEET_QUORUM,     // Follows quorum and does not meet.
} gd_quorum_status_t;

struct glusterd_volinfo_ {
    gf_lock_t lock;
    glusterd_snap_t *snapshot;
    uuid_t restored_from_snap;
    char restored_from_snapname_id[GD_VOLUME_NAME_MAX];
    char restored_from_snapname[GD_VOLUME_NAME_MAX];
    int type;
    int brick_count;
    uint64_t snap_count;
    uint64_t snap_max_hard_limit;
    struct cds_list_head vol_list;
    /* In case of a snap volume
       i.e (is_snap_volume == TRUE) this
       is linked to glusterd_snap_t->volumes.
       In case of a non-snap volume, this is
       linked to glusterd_conf_t->volumes */
    struct cds_list_head snapvol_list;
    /* This is a current pointer for
       glusterd_volinfo_t->snap_volumes */
    struct cds_list_head bricks;
    struct cds_list_head ta_bricks;
    struct cds_list_head snap_volumes;
    /* TODO : Need to remove this, as this
     * is already part of snapshot object.
     */
    glusterd_volume_status status;
    int sub_count; /* backward compatibility */
    int replica_count;
    int arbiter_count;
    int thin_arbiter_count;
    int disperse_count;
    int redundancy_count;
    int subvol_count;    /* Number of subvolumes in a
                          distribute volume */
    int dist_leaf_count; /* Number of bricks in one
                          distribute subvolume */
    int port;
    gf_store_handle_t *shandle;
    gf_store_handle_t *node_state_shandle;
    gf_store_handle_t *quota_conf_shandle;

    /* Defrag/rebalance related */
    glusterd_rebalance_t rebal;

    /* Replace brick status */
    glusterd_replace_brick_t rep_brick;

    int version;
    uint32_t quota_conf_version;
    uint32_t cksum;
    uint32_t quota_conf_cksum;

    dict_t *dict;

    uuid_t volume_id;
    auth_t auth;
    char *logdir;

    dict_t *gsync_secondaries;
    dict_t *gsync_active_secondaries;

    xlator_t *xl;
    int decommission_in_progress;

    int op_version;
    int client_op_version;
    int32_t quota_xattr_version;
    gf_atomic_t refcnt;
    gd_quorum_status_t quorum_status;

    glusterd_snapdsvc_t snapd;
    glusterd_shdsvc_t shd;
    glusterd_gfproxydsvc_t gfproxyd;
    pthread_mutex_t store_volinfo_lock; /* acquire lock for
                                         * updating the volinfo
                                         */
    gf_transport_type transport_type;
    gf_boolean_t is_snap_volume;
    gf_boolean_t memory_accounting;
    gf_boolean_t stage_deleted; /* volume has passed staging
                                 * for delete operation
                                 */
    char parent_volname[GD_VOLUME_NAME_MAX];
    /* In case of a snap volume
       i.e (is_snap_volume == TRUE) this
       field will contain the name of
       the volume which is snapped. In
       case of a non-snap volume, this
       field will be initialized as N/A */

    char volname[GD_VOLUME_NAME_MAX];

    gf_atomic_t volpeerupdate;
    /* Flag to check about volume has received updates
       from peer
    */

    char snap_plugin[NAME_MAX]; /* Snapshot Plugin name */
};

typedef enum gd_snap_status_ {
    GD_SNAP_STATUS_NONE,
    GD_SNAP_STATUS_INIT,
    GD_SNAP_STATUS_IN_USE,
    GD_SNAP_STATUS_DECOMMISSION,
    GD_SNAP_STATUS_UNDER_RESTORE,
    GD_SNAP_STATUS_RESTORED,
} gd_snap_status_t;

struct glusterd_snap_ {
    gf_lock_t lock;
    struct cds_list_head volumes;
    struct cds_list_head snap_list;
    char *description;
    uuid_t snap_id;
    time_t time_stamp;
    gf_store_handle_t *shandle;
    gd_snap_status_t snap_status;
    gf_boolean_t snap_restored;
    char snapname[GLUSTERD_MAX_SNAP_NAME];
};

typedef struct glusterd_snap_op_ {
    char *snap_vol_id;
    char *brick_path;
    struct cds_list_head snap_ops_list;
    int32_t brick_num;
    int32_t op;
    int32_t status;
} glusterd_snap_op_t;

typedef struct glusterd_missed_snap_ {
    char *node_uuid;
    char *snap_uuid;
    struct cds_list_head missed_snaps;
    struct cds_list_head snap_ops;
} glusterd_missed_snap_info;

typedef enum gd_node_type_ {
    GD_NODE_NONE,
    GD_NODE_BRICK,
    GD_NODE_SHD,
    GD_NODE_REBALANCE,
    GD_NODE_NFS,
    GD_NODE_QUOTAD,
    GD_NODE_SNAPD,
    GD_NODE_BITD,
    GD_NODE_SCRUB
} gd_node_type;

typedef enum missed_snap_stat {
    GD_MISSED_SNAP_NONE,
    GD_MISSED_SNAP_PENDING,
    GD_MISSED_SNAP_DONE,
} missed_snap_stat;

typedef struct glusterd_pending_node_ {
    struct cds_list_head list;
    void *node;
    gd_node_type type;
    int32_t index;
} glusterd_pending_node_t;

enum glusterd_op_ret {
    GLUSTERD_CONNECTION_AWAITED = 100,
};

#define GLUSTERD_DEFAULT_PORT GF_DEFAULT_BASE_PORT
#define GLUSTERD_INFO_FILE "glusterd.info"

#define GLUSTERD_VOLUME_QUOTA_CONFIG "quota.conf"
#define GLUSTERD_VOLUME_INFO_FILE "info"
#define GLUSTERD_VOL_QUOTA_CKSUM_FILE "quota.cksum"
#define GLUSTERD_TRASH "trash"
#define GLUSTERD_VOL_SNAP_DIR_PREFIX "snaps"

#define GLUSTER_SHARED_STORAGE_BRICK_DIR GLUSTERD_DEFAULT_WORKDIR "/ss_brick"

// typedef ssize_t (*gd_serialize_t)(struct iovec outmsg, void *args);

#define GLUSTERD_GET_VOLUME_DIR(path, volinfo, priv)                           \
    do {                                                                       \
        int32_t _vol_dir_len;                                                  \
        if (volinfo->is_snap_volume) {                                         \
            _vol_dir_len = snprintf(                                           \
                path, PATH_MAX, "%s/snaps/%s/%s", priv->workdir,               \
                volinfo->snapshot->snapname, volinfo->volname);                \
        } else {                                                               \
            _vol_dir_len = snprintf(path, PATH_MAX, "%s/vols/%s",              \
                                    priv->workdir, volinfo->volname);          \
        }                                                                      \
        if ((_vol_dir_len < 0) || (_vol_dir_len >= PATH_MAX)) {                \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_DEFRAG_DIR(path, volinfo, priv)                           \
    do {                                                                       \
        char vol_path[PATH_MAX];                                               \
        int32_t _defrag_dir_len;                                               \
        GLUSTERD_GET_VOLUME_DIR(vol_path, volinfo, priv);                      \
        _defrag_dir_len = snprintf(path, PATH_MAX, "%s/%s", vol_path,          \
                                   "rebalance");                               \
        if ((_defrag_dir_len < 0) || (_defrag_dir_len >= PATH_MAX)) {          \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_DEFRAG_PID_FILE(path, volinfo, priv)                      \
    do {                                                                       \
        char defrag_path[PATH_MAX];                                            \
        int32_t _defrag_pidfile_len;                                           \
        GLUSTERD_GET_DEFRAG_DIR(defrag_path, volinfo, priv);                   \
        _defrag_pidfile_len = snprintf(path, PATH_MAX, "%s/%s.pid",            \
                                       defrag_path, uuid_utoa(MY_UUID));       \
        if ((_defrag_pidfile_len < 0) || (_defrag_pidfile_len >= PATH_MAX)) {  \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_SHD_RUNDIR(path, volinfo, priv)                           \
    do {                                                                       \
        int32_t _shd_dir_len;                                                  \
        _shd_dir_len = snprintf(path, PATH_MAX, "%s/shd/%s", priv->rundir,     \
                                volinfo->volname);                             \
        if ((_shd_dir_len < 0) || (_shd_dir_len >= PATH_MAX)) {                \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_VOLUME_PID_DIR(path, volinfo, priv)                       \
    do {                                                                       \
        int32_t _vol_pid_len;                                                  \
        if (volinfo->is_snap_volume) {                                         \
            _vol_pid_len = snprintf(path, PATH_MAX, "%s/snaps/%s/%s",          \
                                    priv->rundir, volinfo->snapshot->snapname, \
                                    volinfo->volname);                         \
        } else {                                                               \
            _vol_pid_len = snprintf(path, PATH_MAX, "%s/vols/%s",              \
                                    priv->rundir, volinfo->volname);           \
        }                                                                      \
        if ((_vol_pid_len < 0) || (_vol_pid_len >= PATH_MAX)) {                \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_SNAP_GEO_REP_DIR(path, snap, priv)                        \
    do {                                                                       \
        int32_t _snap_geo_len;                                                 \
        _snap_geo_len = snprintf(path, PATH_MAX, "%s/snaps/%s/%s",             \
                                 priv->workdir, snap->snapname, GEOREP);       \
        if ((_snap_geo_len < 0) || (_snap_geo_len >= PATH_MAX)) {              \
            path[0] = 0;                                                       \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_QUOTA_LIMIT_MOUNT_PATH(abspath, volname, path)            \
    do {                                                                       \
        snprintf(abspath, sizeof(abspath) - 1,                                 \
                 DEFAULT_VAR_RUN_DIRECTORY "/%s_quota_limit%s", volname,       \
                 path);                                                        \
    } while (0)

#define GLUSTERD_REMOVE_SLASH_FROM_PATH(path, string)                          \
    do {                                                                       \
        int i = 0;                                                             \
        for (i = 1; i < strlen(path); i++) {                                   \
            string[i - 1] = path[i];                                           \
            if (string[i - 1] == '/' && (i != strlen(path) - 1))               \
                string[i - 1] = '-';                                           \
        }                                                                      \
    } while (0)

#define GLUSTERD_GET_BRICK_PIDFILE(pidfile, volinfo, brickinfo, priv)          \
    do {                                                                       \
        char exp_path[PATH_MAX] = {                                            \
            0,                                                                 \
        };                                                                     \
        char volpath[PATH_MAX] = {                                             \
            0,                                                                 \
        };                                                                     \
        int32_t _brick_pid_len = 0;                                            \
        GLUSTERD_GET_VOLUME_PID_DIR(volpath, volinfo, priv);                   \
        GLUSTERD_REMOVE_SLASH_FROM_PATH(brickinfo->path, exp_path);            \
        _brick_pid_len = snprintf(pidfile, PATH_MAX, "%s/%s-%s.pid", volpath,  \
                                  brickinfo->hostname, exp_path);              \
        if ((_brick_pid_len < 0) || (_brick_pid_len >= PATH_MAX)) {            \
            pidfile[0] = 0;                                                    \
        }                                                                      \
    } while (0)

#define RCU_READ_LOCK                                                          \
    pthread_mutex_lock(&(THIS->ctx)->cleanup_lock);                            \
    {                                                                          \
        rcu_read_lock();                                                       \
    }                                                                          \
    pthread_mutex_unlock(&(THIS->ctx)->cleanup_lock);

#define RCU_READ_UNLOCK                                                        \
    pthread_mutex_lock(&(THIS->ctx)->cleanup_lock);                            \
    {                                                                          \
        rcu_read_unlock();                                                     \
    }                                                                          \
    pthread_mutex_unlock(&(THIS->ctx)->cleanup_lock);

int
glusterd_uuid_init();

int
glusterd_uuid_generate_save();

#define MY_UUID (__glusterd_uuid())

static inline unsigned char *
__glusterd_uuid()
{
    glusterd_conf_t *priv = THIS->private;

    if (gf_uuid_is_null(priv->uuid))
        glusterd_uuid_init();
    return &priv->uuid[0];
}

int
glusterd_big_locked_notify(struct rpc_clnt *rpc, void *mydata,
                           rpc_clnt_event_t event, void *data,
                           rpc_clnt_notify_t notify_fn);

int
glusterd_big_locked_cbk(struct rpc_req *req, struct iovec *iov, int count,
                        void *myframe, fop_cbk_fn_t fn);

int
glusterd_big_locked_handler(rpcsvc_request_t *req, rpcsvc_actor actor_fn);

int
glusterd_xfer_friend_add_resp(rpcsvc_request_t *req, char *myhostname,
                              char *remote_hostname, int port, int32_t op_ret,
                              int32_t op_errno);

int
glusterd_friend_add_from_peerinfo(glusterd_peerinfo_t *friend,
                                  gf_boolean_t restore,
                                  glusterd_peerctx_args_t *args);
int
glusterd_friend_rpc_create(xlator_t *this, glusterd_peerinfo_t *peerinfo,
                           glusterd_peerctx_args_t *args);
int
glusterd_friend_remove(uuid_t uuid, char *hostname);

int
glusterd_op_lock_send_resp(rpcsvc_request_t *req, int32_t status);

int
glusterd_op_unlock_send_resp(rpcsvc_request_t *req, int32_t status);

int
glusterd_op_mgmt_v3_lock_send_resp(rpcsvc_request_t *req, uuid_t *txn_id,
                                   int32_t status);

int
glusterd_op_mgmt_v3_unlock_send_resp(rpcsvc_request_t *req, uuid_t *txn_id,
                                     int32_t status);

int
glusterd_op_stage_send_resp(rpcsvc_request_t *req, int32_t op, int32_t status,
                            char *op_errstr, dict_t *rsp_dict);

int
glusterd_handle_create_volume(rpcsvc_request_t *req);

int
glusterd_handle_defrag_volume(rpcsvc_request_t *req);

int
glusterd_xfer_cli_probe_resp(rpcsvc_request_t *req, int32_t op_ret,
                             int32_t op_errno, char *op_errstr, char *hostname,
                             int port, dict_t *dict);

int
glusterd_op_commit_send_resp(rpcsvc_request_t *req, int32_t op, int32_t status,
                             char *op_errstr, dict_t *rsp_dict);

int
glusterd_xfer_friend_remove_resp(rpcsvc_request_t *req, char *hostname,
                                 int port);

int
glusterd_handle_cli_start_volume(rpcsvc_request_t *req);

int
glusterd_handle_cli_stop_volume(rpcsvc_request_t *req);

int
glusterd_handle_cli_delete_volume(rpcsvc_request_t *req);

int
glusterd_handle_add_brick(rpcsvc_request_t *req);

int
glusterd_handle_attach_tier(rpcsvc_request_t *req);

int
glusterd_handle_add_tier_brick(rpcsvc_request_t *req);

int
glusterd_handle_replace_brick(rpcsvc_request_t *req);

int
glusterd_handle_remove_brick(rpcsvc_request_t *req);

int
glusterd_handle_log_rotate(rpcsvc_request_t *req);

int
glusterd_handle_copy_file(rpcsvc_request_t *req);

int
glusterd_handle_sys_exec(rpcsvc_request_t *req);

int
glusterd_handle_gsync_set(rpcsvc_request_t *req);

int
glusterd_handle_quota(rpcsvc_request_t *req);

int
glusterd_handle_bitrot(rpcsvc_request_t *req);

int
glusterd_handle_reset_brick(rpcsvc_request_t *req);

int
glusterd_xfer_cli_deprobe_resp(rpcsvc_request_t *req, int32_t op_ret,
                               int32_t op_errno, char *op_errstr,
                               char *hostname, dict_t *dict);

int
glusterd_client_statedump_submit_req(char *volname, char *target_ip, char *pid);

int
glusterd_fetchspec_notify(xlator_t *this);

int
glusterd_fetchsnap_notify(xlator_t *this);

int
glusterd_peer_dump_version(xlator_t *this, struct rpc_clnt *rpc,
                           glusterd_peerctx_t *peerctx);

int
glusterd_validate_reconfopts(glusterd_volinfo_t *volinfo, dict_t *val_dict,
                             char **op_errstr);

int
glusterd_brick_rpc_notify(struct rpc_clnt *rpc, void *mydata,
                          rpc_clnt_event_t event, void *data);

int
glusterd_rpc_create(struct rpc_clnt **rpc, dict_t *options,
                    rpc_clnt_notify_t notify_fn, void *notify_data,
                    gf_boolean_t force);

/* handler functions */
int32_t
glusterd_op_begin(rpcsvc_request_t *req, glusterd_op_t op, void *ctx,
                  char *err_str, size_t size);

/* removed other definitions as they have been defined elsewhere in this file*/

int
glusterd_handle_cli_statedump_volume(rpcsvc_request_t *req);

int
glusterd_handle_defrag_start(glusterd_volinfo_t *volinfo, char *op_errstr,
                             size_t len, int cmd, defrag_cbk_fn_t cbk,
                             glusterd_op_t op);
int
glusterd_rebalance_rpc_create(glusterd_volinfo_t *volinfo);

int
glusterd_rebalance_defrag_init(glusterd_volinfo_t *volinfo,
                               defrag_cbk_fn_t cbk);

int
glusterd_handle_cli_heal_volume(rpcsvc_request_t *req);

int
glusterd_handle_snapshot(rpcsvc_request_t *req);

/* op-sm functions */
int
glusterd_op_stage_heal_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_heal_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_stage_gsync_set(dict_t *dict, char **op_errstr);
int
glusterd_op_gsync_set(dict_t *dict, char **op_errstr, dict_t *rsp_dict);
int
glusterd_op_stage_copy_file(dict_t *dict, char **op_errstr);
int
glusterd_op_copy_file(dict_t *dict, char **op_errstr);
int
glusterd_op_stage_sys_exec(dict_t *dict, char **op_errstr);
int
glusterd_op_sys_exec(dict_t *dict, char **op_errstr, dict_t *rsp_dict);
int
glusterd_op_stage_gsync_create(dict_t *dict, char **op_errstr);
int
glusterd_op_gsync_create(dict_t *dict, char **op_errstr, dict_t *rsp_dict);
int
glusterd_op_quota(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_bitrot(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_quota(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_bitrot(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_replace_brick(dict_t *dict, char **op_errstr,
                                dict_t *rsp_dict);
int
glusterd_op_replace_brick(dict_t *dict, dict_t *rsp_dict);
int
glusterd_op_log_rotate(dict_t *dict);
int
glusterd_op_stage_log_rotate(dict_t *dict, char **op_errstr);
int
glusterd_op_stage_create_volume(dict_t *dict, char **op_errstr,
                                dict_t *rsp_dict);
int
glusterd_op_stage_start_volume(dict_t *dict, char **op_errstr,
                               dict_t *rsp_dict);
int
glusterd_op_stage_stop_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_stage_delete_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_create_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_start_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_stop_volume(dict_t *dict);
int
glusterd_op_delete_volume(dict_t *dict);
int
glusterd_handle_ganesha_op(dict_t *dict, char **op_errstr, char *key,
                           char *value);
int
glusterd_check_ganesha_cmd(char *key, char *value, char **errstr, dict_t *dict);
int
glusterd_op_stage_set_ganesha(dict_t *dict, char **op_errstr);
int
glusterd_op_set_ganesha(dict_t *dict, char **errstr);
int
ganesha_manage_export(dict_t *dict, char *value,
                      gf_boolean_t update_cache_invalidation, char **op_errstr);
int
gd_ganesha_send_dbus(char *volname, char *value);
gf_boolean_t
glusterd_is_ganesha_cluster();
gf_boolean_t
glusterd_check_ganesha_export(glusterd_volinfo_t *volinfo);
int
manage_export_config(char *volname, char *value, char **op_errstr);

int
glusterd_op_add_brick(dict_t *dict, char **op_errstr);
int
glusterd_op_remove_brick(dict_t *dict, char **op_errstr);
int
glusterd_op_stage_add_brick(dict_t *dict, char **op_errstr, dict_t *rsp_dict);
int
glusterd_op_stage_remove_brick(dict_t *dict, char **op_errstr);

int
glusterd_set_rebalance_id_for_remove_brick(dict_t *req_dict, dict_t *rsp_dict);

int
glusterd_set_rebalance_id_in_rsp_dict(dict_t *req_dict, dict_t *rsp_dict);

int
glusterd_mgmt_v3_op_stage_rebalance(dict_t *dict, char **op_errstr);

int
glusterd_mgmt_v3_op_rebalance(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_rebalance(dict_t *dict, char **op_errstr);
int
glusterd_op_rebalance(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_statedump_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_statedump_volume(dict_t *dict, char **op_errstr);

int
glusterd_op_stage_clearlocks_volume(dict_t *dict, char **op_errstr);
int
glusterd_op_clearlocks_volume(dict_t *dict, char **op_errstr, dict_t *rsp_dict);

int
glusterd_op_stage_barrier(dict_t *dict, char **op_errstr);
int
glusterd_op_barrier(dict_t *dict, char **op_errstr);

/* misc */
int
glusterd_op_perform_remove_brick(glusterd_volinfo_t *volinfo, char *brick,
                                 int force, int *need_migrate);
int
glusterd_op_stop_volume_args_get(dict_t *dict, char **volname, int *flags);
int
glusterd_op_statedump_volume_args_get(dict_t *dict, char **volname,
                                      char **options, int *option_cnt);

int
glusterd_op_gsync_args_get(dict_t *dict, char **op_errstr, char **primary,
                           char **secondary, char **host_uuid);

int
glusterd_op_get_max_opversion(char **op_errstr, dict_t *rsp_dict);

int
glusterd_start_volume(glusterd_volinfo_t *volinfo, int flags,
                      gf_boolean_t wait);

int
glusterd_stop_volume(glusterd_volinfo_t *volinfo);

/* Synctask part */
int32_t
glusterd_op_begin_synctask(rpcsvc_request_t *req, glusterd_op_t op, void *dict);
int32_t
glusterd_defrag_event_notify_handle(dict_t *dict);

/* snapshot */
glusterd_snap_t *
glusterd_new_snap_object();

int32_t
glusterd_list_add_snapvol(glusterd_volinfo_t *origin_vol,
                          glusterd_volinfo_t *snap_vol);

glusterd_snap_t *
glusterd_remove_snap_by_id(uuid_t snap_id);

glusterd_snap_t *
glusterd_remove_snap_by_name(char *snap_name);

glusterd_snap_t *
glusterd_find_snap_by_name(char *snap_name);

glusterd_snap_t *
glusterd_find_snap_by_id(uuid_t snap_id);

int
glusterd_snapshot_prevalidate(dict_t *dict, char **op_errstr, dict_t *rsp_dict,
                              uint32_t *op_errno);
int
glusterd_snapshot_brickop(dict_t *dict, char **op_errstr, dict_t *rsp_dict);
int
glusterd_snapshot(dict_t *dict, char **op_errstr, uint32_t *op_errno,
                  dict_t *rsp_dict);
int
glusterd_snapshot_postvalidate(dict_t *dict, int32_t op_ret, char **op_errstr,
                               dict_t *rsp_dict);
int32_t
glusterd_snap_remove(dict_t *rsp_dict, glusterd_snap_t *snap,
                     gf_boolean_t remove_lvm, gf_boolean_t force,
                     gf_boolean_t is_clone);
int32_t
glusterd_add_missed_snaps_to_list(dict_t *dict, int32_t missed_snap_count);

int32_t
glusterd_add_new_entry_to_list(char *missed_info, char *snap_vol_id,
                               int32_t brick_num, char *brick_path,
                               int32_t snap_op, int32_t snap_status);

int
glusterd_snapshot_revert_restore_from_snap(glusterd_snap_t *snap);

gf_boolean_t
glusterd_should_i_stop_bitd();

int
glusterd_remove_brick_migrate_cbk(glusterd_volinfo_t *volinfo,
                                  gf_defrag_status_t status);

int
glusterd_options_init(xlator_t *this);

int32_t
glusterd_recreate_volfiles(glusterd_conf_t *conf);

void
glusterd_add_peers_to_auth_list(char *volname);

int
glusterd_replace_old_auth_allow_list(char *volname);

int
glusterd_peer_hostname_update(glusterd_peerinfo_t *peerinfo,
                              const char *hostname, gf_boolean_t store_update);

#endif
