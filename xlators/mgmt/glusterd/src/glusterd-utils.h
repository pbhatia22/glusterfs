/*
   Copyright (c) 2006-2012 Red Hat, Inc. <http://www.redhat.com>
   This file is part of GlusterFS.

   This file is licensed to you under your choice of the GNU Lesser
   General Public License, version 3 or any later version (LGPLv3 or
   later), or the GNU General Public License, version 2 (GPLv2), in all
   cases as published by the Free Software Foundation.
*/
#ifndef _GLUSTERD_UTILS_H
#define _GLUSTERD_UTILS_H

#include <glusterfs/compat-uuid.h>

#include "glusterd-peer-utils.h"

typedef struct glusterd_add_dict_args {
    xlator_t *this;
    dict_t *voldict;
    int start;
    int end;
} glusterd_add_dict_args_t;

typedef struct glusterd_friend_synctask_args {
    dict_t *peer_data;
    dict_t *peer_ver_data;  // Dictionary to save peer version data
    /* This status_arr[1] is not a real size, real size of the array
       is dynamically allocated
    */
    uint64_t status_arr[1];
} glusterd_friend_synctask_args_t;

enum glusterd_vol_comp_status_ {
    GLUSTERD_VOL_COMP_NONE = 0,
    GLUSTERD_VOL_COMP_SCS = 1,
    GLUSTERD_VOL_COMP_UPDATE_REQ,
    GLUSTERD_VOL_COMP_RJT,
};

typedef struct addrinfo_list {
    struct cds_list_head list;
    struct addrinfo *info;
} addrinfo_list_t;

typedef enum {
    GF_AI_COMPARE_NO_MATCH = 0,
    GF_AI_COMPARE_MATCH = 1,
    GF_AI_COMPARE_ERROR = 2
} gf_ai_compare_t;

#define GLUSTERD_CKSUM_FILE "cksum"

#define GLUSTERD_SOCK_DIR "/var/run/gluster"
#define GLUSTERD_ASSIGN_BRICKID_TO_BRICKINFO(brickinfo, volinfo, brickid)      \
    do {                                                                       \
        sprintf(brickinfo->brick_id, "%s-client-%d", volinfo->volname,         \
                brickid);                                                      \
    } while (0)

#define GLUSTERD_ASSIGN_BRICKID_TO_TA_BRICKINFO(ta_brickinfo, volinfo,         \
                                                brickid)                       \
    do {                                                                       \
        sprintf(ta_brickinfo->brick_id, "%s-ta-%d", volinfo->volname,          \
                brickid);                                                      \
    } while (0)

#define ALL_VOLUME_OPTION_CHECK(volname, get_opt, key, ret, op_errstr, label)  \
    do {                                                                       \
        gf_boolean_t _all = !strcmp("all", volname);                           \
        gf_boolean_t _key_all = !strcmp(key, "all");                           \
        gf_boolean_t _is_valid_opt = _gf_false;                                \
        int32_t i = 0;                                                         \
                                                                               \
        if (!get_opt &&                                                        \
            (_key_all || !strcmp(key, GLUSTERD_MAX_OP_VERSION_KEY))) {         \
            ret = -1;                                                          \
            *op_errstr = gf_strdup("Not a valid option to set");               \
            goto out;                                                          \
        }                                                                      \
        if (_key_all) {                                                        \
            _is_valid_opt = _gf_true;                                          \
        } else {                                                               \
            for (i = 0; valid_all_vol_opts[i].option; i++) {                   \
                if (!strcmp(key, valid_all_vol_opts[i].option)) {              \
                    _is_valid_opt = _gf_true;                                  \
                    break;                                                     \
                }                                                              \
            }                                                                  \
        }                                                                      \
        if (_all && !_is_valid_opt) {                                          \
            ret = -1;                                                          \
            *op_errstr = gf_strdup("Not a valid option for all volumes");      \
            goto label;                                                        \
        } else if (!_all && _is_valid_opt) {                                   \
            ret = -1;                                                          \
            *op_errstr = gf_strdup("Not a valid option for single volume");    \
            goto label;                                                        \
        }                                                                      \
    } while (0)

struct glusterd_lock_ {
    uuid_t owner;
    time_t timestamp;
};

typedef struct glusterd_dict_ctx_ {
    dict_t *dict;
    int opt_count;
    char *key_name;
    char *val_name;
    char *prefix;
} glusterd_dict_ctx_t;

typedef struct glusterd_hostname_ {
    char *hostname;
    struct list_head hostname_list;
} glusterd_hostname_t;

struct glusterd_snap_ops {
    const char *name;

    gf_boolean_t (*const probe)(char *brick_path);
    int (*const details)(dict_t *rsp_dict, glusterd_brickinfo_t *snap_brickinfo,
                         char *snapname, char *snap_volume_id,
                         int32_t brick_num, char *key_prefix);
    int32_t (*const create)(glusterd_brickinfo_t *snap_brickinfo,
                            char *snapname, char *snap_volume_id,
                            int32_t brick_num);
    int32_t (*const clone)(glusterd_brickinfo_t *snap_brickinfo, char *snapname,
                           char *snap_volume_id, char *clonename,
                           char *clone_volume_id, int32_t brick_num);
    int32_t (*const remove)(glusterd_brickinfo_t *snap_brickinfo,
                            char *snapname, char *snap_volume_id,
                            int32_t brick_num);
    int32_t (*const activate)(glusterd_brickinfo_t *snap_brickinfo,
                              char *snapname, char *snap_volume_id,
                              int32_t brick_num);
    int32_t (*const deactivate)(glusterd_brickinfo_t *snap_brickinfo,
                                char *snapname, char *snap_volume_id,
                                int32_t brick_num);
    int32_t (*const restore)(glusterd_brickinfo_t *snap_brickinfo,
                             char *snapname, char *snap_volume_id,
                             int32_t brick_num,
                             gf_boolean_t *retain_origin_path);
    int32_t (*const brick_path)(char *snap_mount_dir, char *origin_brick_path,
                                int clone, char *snap_clone_name,
                                char *snap_clone_volume_id,
                                char *snap_brick_dir, int brick_num,
                                glusterd_brickinfo_t *brickinfo, int restore);
};

extern struct glusterd_snap_ops lvm_snap_ops;
extern struct glusterd_snap_ops zfs_snap_ops;

gf_boolean_t
glusterd_mntopts_exists(const char *str, const char *opts);

gf_boolean_t
is_brick_mx_enabled(void);

int
glusterd_compare_lines(const void *a, const void *b);

typedef int (*glusterd_condition_func)(glusterd_volinfo_t *volinfo,
                                       glusterd_brickinfo_t *brickinfo,
                                       void *ctx);
typedef struct glusterd_lock_ glusterd_lock_t;

int32_t
glusterd_get_lock_owner(uuid_t *cur_owner);

int32_t
glusterd_lock(uuid_t new_owner);

int32_t
glusterd_unlock(uuid_t owner);

int32_t
glusterd_get_uuid(uuid_t *uuid);

int
glusterd_submit_reply(rpcsvc_request_t *req, void *arg, struct iovec *payload,
                      int payloadcount, struct iobref *iobref,
                      xdrproc_t xdrproc);

int
glusterd_to_cli(rpcsvc_request_t *req, gf_cli_rsp *arg, struct iovec *payload,
                int payloadcount, struct iobref *iobref, xdrproc_t xdrproc,
                dict_t *dict);

int
glusterd_submit_request(struct rpc_clnt *rpc, void *req, call_frame_t *frame,
                        rpc_clnt_prog_t *prog, int procnum,
                        struct iobref *iobref, xlator_t *this,
                        fop_cbk_fn_t cbkfn, xdrproc_t xdrproc);
int32_t
glusterd_volinfo_new(glusterd_volinfo_t **volinfo);

char *
glusterd_auth_get_username(glusterd_volinfo_t *volinfo);

char *
glusterd_auth_get_password(glusterd_volinfo_t *volinfo);

int32_t
glusterd_auth_set_username(glusterd_volinfo_t *volinfo, char *username);

int32_t
glusterd_auth_set_password(glusterd_volinfo_t *volinfo, char *password);

int32_t
glusterd_brickinfo_new(glusterd_brickinfo_t **brickinfo);

int32_t
glusterd_brickinfo_new_from_brick(char *brick, glusterd_brickinfo_t **brickinfo,
                                  gf_boolean_t construct_real_path,
                                  char **op_errstr);

int32_t
glusterd_volinfo_find(const char *volname, glusterd_volinfo_t **volinfo);

int
glusterd_get_next_available_brickid(glusterd_volinfo_t *volinfo);

int32_t
glusterd_resolve_brick(glusterd_brickinfo_t *brickinfo);

int
glusterd_brick_process_remove_brick(glusterd_brickinfo_t *brickinfo,
                                    int *last_brick);

int32_t
glusterd_volume_stop_glusterfs(glusterd_volinfo_t *volinfo,
                               glusterd_brickinfo_t *brickinfo,
                               gf_boolean_t del_brick);

int
send_attach_req(xlator_t *this, struct rpc_clnt *rpc, char *path,
                glusterd_brickinfo_t *brick, glusterd_brickinfo_t *other_brick,
                int op, gf_boolean_t graceful_cleanup);

glusterd_volinfo_t *
glusterd_volinfo_ref(glusterd_volinfo_t *volinfo);

glusterd_volinfo_t *
glusterd_volinfo_unref(glusterd_volinfo_t *volinfo);

int32_t
glusterd_brickinfo_delete(glusterd_brickinfo_t *brickinfo);

int32_t
glusterd_volume_brickinfo_get_by_brick(char *brick, glusterd_volinfo_t *volinfo,
                                       glusterd_brickinfo_t **brickinfo,
                                       gf_boolean_t construct_real_path);

int32_t
glusterd_add_volumes_to_export_dict(dict_t *peer_data, char **buf,
                                    u_int *length);

int32_t
glusterd_compare_friend_data(dict_t *peer_data, dict_t *cmp, int32_t *status,
                             char *hostname);

int
glusterd_compute_cksum(glusterd_volinfo_t *volinfo, gf_boolean_t is_quota_conf);

void
glusterd_set_socket_filepath(char *sock_filepath, char *sockpath, size_t len);

struct rpc_clnt *
glusterd_pending_node_get_rpc(glusterd_pending_node_t *pending_node);

void
glusterd_pending_node_put_rpc(glusterd_pending_node_t *pending_node);

int
glusterd_remote_hostname_get(rpcsvc_request_t *req, char *remote_host, int len);

void
glusterd_set_volume_status(glusterd_volinfo_t *volinfo,
                           glusterd_volume_status status);

int32_t
glusterd_add_volume_to_dict(glusterd_volinfo_t *volinfo, dict_t *dict,
                            int32_t count, char *prefix);

void
glusterd_set_brick_status(glusterd_brickinfo_t *brickinfo,
                          gf_brick_status_t status);

gf_boolean_t
glusterd_is_brick_started(glusterd_brickinfo_t *brickinfo);

int
glusterd_brick_start(glusterd_volinfo_t *volinfo,
                     glusterd_brickinfo_t *brickinfo, gf_boolean_t wait,
                     gf_boolean_t only_connect);
int
glusterd_brick_stop(glusterd_volinfo_t *volinfo,
                    glusterd_brickinfo_t *brickinfo, gf_boolean_t del_brick);

int
glusterd_is_defrag_on(glusterd_volinfo_t *volinfo);

int
glusterd_new_brick_validate(char *brick, glusterd_brickinfo_t *brickinfo,
                            char *op_errstr, size_t len, char *op);

int
glusterd_validate_and_create_brickpath(glusterd_brickinfo_t *brickinfo,
                                       uuid_t volume_id, char *volname,
                                       char **op_errstr, gf_boolean_t is_force,
                                       gf_boolean_t ignore_partition);
int
glusterd_sm_tr_log_transition_add(glusterd_sm_tr_log_t *log, int old_state,
                                  int new_state, int event);
int
glusterd_sm_tr_log_init(glusterd_sm_tr_log_t *log, char *(*state_name_get)(int),
                        char *(*event_name_get)(int), size_t size);
void
glusterd_sm_tr_log_delete(glusterd_sm_tr_log_t *log);

int32_t
glusterd_delete_volume(glusterd_volinfo_t *volinfo);
int32_t
glusterd_delete_brick(glusterd_volinfo_t *volinfo,
                      glusterd_brickinfo_t *brickinfo);

int
glusterd_spawn_daemons(void *opaque);

int
glusterd_start_gsync(glusterd_volinfo_t *primary_vol, char *secondary,
                     char *path_list, char *conf_path, char *glusterd_uuid_str,
                     char **op_errstr, gf_boolean_t is_pause);
int
glusterd_get_local_brickpaths(glusterd_volinfo_t *volinfo, char **pathlist);

int
glusterd_add_brick_detail_to_dict(glusterd_volinfo_t *volinfo,
                                  glusterd_brickinfo_t *brickinfo, dict_t *dict,
                                  int32_t count);

int32_t
glusterd_add_brick_to_dict(glusterd_volinfo_t *volinfo,
                           glusterd_brickinfo_t *brickinfo, dict_t *dict,
                           int32_t count);

gf_boolean_t
glusterd_is_fuse_available();

int
glusterd_brick_statedump(glusterd_volinfo_t *volinfo,
                         glusterd_brickinfo_t *brickinfo, char *options,
                         int option_cnt, char **op_errstr);

#ifdef BUILD_GNFS
int
glusterd_nfs_statedump(char *options, int option_cnt, char **op_errstr);
#endif

int
glusterd_quotad_statedump(char *options, int option_cnt, char **op_errstr);

gf_boolean_t
glusterd_is_volume_replicate(glusterd_volinfo_t *volinfo);

gf_boolean_t
glusterd_is_brick_decommissioned(glusterd_volinfo_t *volinfo, char *hostname,
                                 char *path);
int
glusterd_friend_contains_vol_bricks(glusterd_volinfo_t *volinfo,
                                    uuid_t friend_uuid);

int
glusterd_friend_remove_cleanup_vols(uuid_t uuid);

int
glusterd_get_trusted_client_filepath(char *filepath,
                                     glusterd_volinfo_t *volinfo,
                                     gf_transport_type type);
int
glusterd_restart_rebalance_for_volume(glusterd_volinfo_t *volinfo);

int
glusterd_get_dist_leaf_count(glusterd_volinfo_t *volinfo);

gf_boolean_t
glusterd_is_local_brick(xlator_t *this, glusterd_volinfo_t *volinfo,
                        glusterd_brickinfo_t *brickinfo);
int
glusterd_validate_volume_id(dict_t *op_dict, glusterd_volinfo_t *volinfo);

int
glusterd_defrag_volume_status_update(glusterd_volinfo_t *volinfo,
                                     dict_t *rsp_dict, int32_t cmd);

int
glusterd_check_files_identical(char *filename1, char *filename2,
                               gf_boolean_t *identical);

int
glusterd_check_topology_identical(const char *filename1, const char *filename2,
                                  gf_boolean_t *identical);

void
glusterd_volinfo_reset_defrag_stats(glusterd_volinfo_t *volinfo);
int
glusterd_volset_help(dict_t *dict, char **op_errstr);

int32_t
glusterd_sync_use_rsp_dict(dict_t *aggr, dict_t *rsp_dict);
int32_t
glusterd_rb_use_rsp_dict(dict_t *aggr, dict_t *rsp_dict);
int
glusterd_profile_volume_use_rsp_dict(dict_t *aggr, dict_t *rsp_dict);
int
glusterd_volume_status_copy_to_op_ctx_dict(dict_t *aggr, dict_t *rsp_dict);
int
glusterd_volume_rebalance_use_rsp_dict(dict_t *aggr, dict_t *rsp_dict);
int32_t
glusterd_handle_node_rsp(dict_t *req_ctx, void *pending_entry, glusterd_op_t op,
                         dict_t *rsp_dict, dict_t *op_ctx, char **op_errstr,
                         gd_node_type type);

int32_t
glusterd_check_if_quota_trans_enabled(glusterd_volinfo_t *volinfo);

int32_t
glusterd_set_originator_uuid(dict_t *dict);

/* Should be used only when an operation is in progress, as that is the only
 * time a lock_owner is set
 */
gf_boolean_t
is_origin_glusterd(dict_t *dict);

int
glusterd_get_next_global_opt_version_str(dict_t *opts, char **version_str);

int
glusterd_generate_and_set_task_id(dict_t *dict, char *key, const int keylen);

int
glusterd_copy_uuid_to_dict(uuid_t uuid, dict_t *dict, char *key,
                           const int keylen);

gf_boolean_t
glusterd_is_same_address(char *name1, char *name2);

void
gd_update_volume_op_versions(glusterd_volinfo_t *volinfo);

gf_boolean_t
gd_is_remove_brick_committed(glusterd_volinfo_t *volinfo);

int
glusterd_remove_brick_validate_bricks(gf1_op_commands cmd, int32_t brick_count,
                                      dict_t *dict, glusterd_volinfo_t *volinfo,
                                      char **errstr, gf_defrag_type_t type);
int
glusterd_get_secondary_details_confpath(glusterd_volinfo_t *volinfo,
                                        dict_t *dict, char **secondary_url,
                                        char **secondary_host,
                                        char **secondary_vol, char **conf_path,
                                        char **op_errstr);

int
glusterd_get_secondary_info(char *secondary, char **secondary_url,
                            char **hostname, char **secondary_vol,
                            char **op_errstr);

int
glusterd_get_statefile_name(glusterd_volinfo_t *volinfo, char *secondary,
                            char *conf_path, char **statefile,
                            gf_boolean_t *is_template_in_use);

int
glusterd_gsync_read_frm_status(char *path, char *buf, size_t blen);

int
glusterd_create_status_file(char *primary, char *secondary, char *secondary_url,
                            char *secondary_vol, char *status);

int
glusterd_check_restart_gsync_session(glusterd_volinfo_t *volinfo,
                                     char *secondary, dict_t *resp_dict,
                                     char *path_list, char *conf_path,
                                     gf_boolean_t is_force);

int
glusterd_check_gsync_running_local(char *primary, char *secondary,
                                   char *conf_path, gf_boolean_t *is_run);

gf_boolean_t
gd_should_i_start_rebalance(glusterd_volinfo_t *volinfo);

int
glusterd_is_volume_quota_enabled(glusterd_volinfo_t *volinfo);

int
glusterd_is_bitrot_enabled(glusterd_volinfo_t *volinfo);

gf_boolean_t
glusterd_all_volumes_with_quota_stopped();

void
glusterd_clean_up_quota_store(glusterd_volinfo_t *volinfo);

gf_boolean_t
glusterd_status_has_tasks(int cmd);

rpc_clnt_t *
glusterd_rpc_clnt_unref(glusterd_conf_t *conf, rpc_clnt_t *rpc);

int32_t
glusterd_compare_volume_name(struct cds_list_head *, struct cds_list_head *);

char *
glusterd_get_brick_mount_device(char *brick_path);

struct mntent *
glusterd_get_mnt_entry_info(char *mnt_pt, char *buff, int buflen,
                            struct mntent *entry_ptr);

int
glusterd_get_brick_root(char *path, char **mount_point);

int32_t
glusterd_get_brick_mount_dir(char *brickpath, char *hostname, char *mount_dir);

int32_t
glusterd_aggr_brick_mount_dirs(dict_t *aggr, dict_t *rsp_dict);

int
glusterd_launch_synctask(synctask_fn_t fn, void *opaque);

int
glusterd_enable_default_options(glusterd_volinfo_t *volinfo, char *option);

int
glusterd_unlink_file(char *sock_file_path);

int32_t
glusterd_find_brick_mount_path(char *brick_path, char **brick_mount_path);

/*
 * Function to retrieve list of snap volnames and their uuids
 */
int
glusterd_snapshot_get_volnames_uuids(dict_t *dict, char *volname,
                                     gf_getsnap_name_uuid_rsp *snap_info_rsp);

int
glusterd_update_mntopts(char *brick_path, glusterd_brickinfo_t *brickinfo);

int
glusterd_update_fs_label(char *brick_path, char *fstype, char *device_path);

int
glusterd_get_global_max_op_version(rpcsvc_request_t *req, dict_t *ctx,
                                   int count);

int
glusterd_get_global_options_for_all_vols(rpcsvc_request_t *req, dict_t *dict,
                                         char **op_errstr);

int
glusterd_get_default_val_for_volopt(dict_t *dict, gf_boolean_t all_opts,
                                    char *key, char *orig_key,
                                    glusterd_volinfo_t *volinfo,
                                    char **err_str);

int
glusterd_check_client_op_version_support(char *volname, uint32_t op_version,
                                         char **op_errstr);

void
glusterd_get_rebalance_volfile(glusterd_volinfo_t *volinfo, char *path,
                               int path_len);

void
glusterd_get_gfproxy_client_volfile(glusterd_volinfo_t *volinfo, char *path,
                                    int path_len);

int
glusterd_vol_add_quota_conf_to_dict(glusterd_volinfo_t *volinfo, dict_t *load,
                                    int vol_idx, char *prefix);

int32_t
glusterd_import_volinfo(dict_t *peer_data, int count,
                        glusterd_volinfo_t **volinfo, char *prefix);

int
glusterd_import_quota_conf(dict_t *peer_data, int vol_idx,
                           glusterd_volinfo_t *new_volinfo, char *prefix);

gf_boolean_t
glusterd_is_shd_compatible_volume(glusterd_volinfo_t *volinfo);

gf_boolean_t
glusterd_are_all_volumes_stopped();

#ifdef BUILD_GNFS
void
glusterd_nfs_pmap_deregister();
#endif

gf_boolean_t
glusterd_is_volume_started(glusterd_volinfo_t *volinfo);

void
glusterd_list_add_order(struct cds_list_head *new, struct cds_list_head *head,
                        int (*compare)(struct cds_list_head *,
                                       struct cds_list_head *));

struct rpc_clnt *
glusterd_defrag_rpc_get(glusterd_defrag_info_t *defrag);

struct rpc_clnt *
glusterd_defrag_rpc_put(glusterd_defrag_info_t *defrag);

int
glusterd_get_dummy_client_filepath(char *filepath, glusterd_volinfo_t *volinfo,
                                   gf_transport_type type);

int
glusterd_handle_replicate_brick_ops(glusterd_volinfo_t *volinfo,
                                    glusterd_brickinfo_t *brickinfo,
                                    glusterd_op_t op);
void
assign_brick_groups(glusterd_volinfo_t *volinfo);

glusterd_brickinfo_t *
get_last_brick_of_brick_group(glusterd_volinfo_t *volinfo,
                              glusterd_brickinfo_t *brickinfo);
int
glusterd_get_rb_dst_brickinfo(glusterd_volinfo_t *volinfo,
                              glusterd_brickinfo_t **brickinfo);
int
rb_update_dstbrick_port(glusterd_brickinfo_t *dst_brickinfo, dict_t *rsp_dict,
                        dict_t *req_dict);
int
glusterd_op_perform_replace_brick(glusterd_volinfo_t *volinfo, char *old_brick,
                                  char *new_brick, dict_t *dict);
char *
gd_rb_op_to_str(char *op);

glusterd_op_t
gd_cli_to_gd_op(char *cli_op);

int
glusterd_get_dst_brick_info(char **dst_brick, char *volname, char **op_errstr,
                            glusterd_brickinfo_t **dst_brickinfo, char **host,
                            dict_t *dict, char **dup_dstbrick);

int
glusterd_brick_op_prerequisites(dict_t *dict, char **op, glusterd_op_t *gd_op,
                                char **volname, glusterd_volinfo_t **volinfo,
                                char **src_brick,
                                glusterd_brickinfo_t **src_brickinfo,
                                char *pidfile, char **op_errstr,
                                dict_t *rsp_dict);

int
glusterd_get_volinfo_from_brick(char *brick, glusterd_volinfo_t **volinfo);

gf_boolean_t
glusterd_is_profile_on(glusterd_volinfo_t *volinfo);

char *
search_brick_path_from_proc(pid_t brick_pid, char *brickpath);

int32_t
glusterd_check_brick_order(dict_t *dict, char *err_str, int32_t type,
                           char **volname, char **bricks, int32_t *brick_count,
                           int32_t sub_count, int flag);
gf_boolean_t
glusterd_gf_is_local_addr(char *hostname);

int
glusterd_defrag_ref(glusterd_defrag_info_t *defrag);

int
glusterd_defrag_unref(glusterd_defrag_info_t *defrag);
#endif
