#ifndef _GLUE_HELPER_H_
#define _GLUE_HELPER_H_

#include <linux/kthread.h>
#include <linux/module.h>

/* Block layer deps */
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <libcap.h>
#include <libfipc.h>
#include <thc_ipc.h>
#include <liblcd/glue_cspace.h>
#include <liblcd/liblcd.h>
#include <liblcd/sync_ipc_poll.h>
#include <asm/lcd_domains/libvmfunc.h>

#define SENDER_DISPATCH_LOOP

enum dispatch_t {
        BLK_MQ_ALLOC_TAG_SET,
        BLK_MQ_INIT_QUEUE,
        BLK_CLEANUP_QUEUE,
	BLK_MQ_END_REQUEST,
        BLK_MQ_FREE_TAG_SET,
        BLK_MQ_START_REQUEST,
        BLK_MQ_MAP_QUEUE,
        BLK_QUEUE_LOGICAL_BLOCK_SIZE,
        BLK_QUEUE_PHYSICAL_BLOCK_SIZE,
        ALLOC_DISK,
	DEVICE_ADD_DISK,
        PUT_DISK,
        DEL_GENDISK,
        DISK_NODE,
        REGISTER_BLKDEV,
        UNREGISTER_BLKDEV,
        REGISTER_CHARDEV,
	QUEUE_RQ_FN,
        MAP_QUEUE_FN,
        INIT_HCTX_FN,
	INIT_HCTX_SYNC,
        SOFTIRQ_DONE_FN,
        OPEN,
	RELEASE,
	MODULE_INIT,
	MODULE_EXIT
};

/* CONTAINERS ------------------------------------------------------------ */
struct blk_mq_hw_ctx_container {
        struct blk_mq_hw_ctx blk_mq_hw_ctx;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct blk_mq_ops_container {
        struct blk_mq_ops blk_mq_ops;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct blk_mq_queue_data_container {
        struct blk_mq_queue_data blk_mq_queue_data;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct blk_mq_tag_set_container {
        struct blk_mq_tag_set tag_set;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct block_device_container {
        struct block_device block_device;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct block_device_operations_container {
        struct block_device_operations block_device_operations;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct gendisk_container {
        struct gendisk gendisk;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct module_container {
        struct module module;
        cptr_t other_ref;
        cptr_t my_ref;
};
//struct nullb_container {
//        struct nullb nullb;
 //       cptr_t other_ref;
  //      cptr_t my_ref;
//};
struct request_container {
        struct request request;
        cptr_t other_ref;
        cptr_t my_ref;
};
struct request_queue_container {
        struct request_queue request_queue;
        cptr_t other_ref;
        cptr_t my_ref;
};

/* CSPACES ------------------------------------------------------------ */
int glue_cap_init(void);

int glue_cap_create(struct glue_cspace **cspace);

void glue_cap_destroy(struct glue_cspace *cspace);

void glue_cap_exit(void);

void glue_cap_remove(
        struct glue_cspace *cspace,
        cptr_t c);

int glue_cap_insert_blk_mq_ops_type(struct glue_cspace *cspace, 
                        struct blk_mq_ops_container *blk_mq_ops_container,
                        cptr_t *c_out);

int glue_cap_insert_module_type(struct glue_cspace *cspace,
                        struct module_container *module_container,
                        cptr_t *c_out);

int glue_cap_insert_blk_mq_tag_set_type(struct glue_cspace *cspace,
                        struct blk_mq_tag_set_container *set_container,
                         cptr_t *c_out);

int glue_cap_insert_gendisk_type(struct glue_cspace *cspace,
                        struct gendisk_container *disk_container,
                        cptr_t *c_out);

int glue_cap_insert_blk_mq_hw_ctx_type(struct glue_cspace *cspace,
                        struct blk_mq_hw_ctx_container *ctx_container,
                        cptr_t *c_out);

int glue_cap_insert_blk_dev_ops_type(struct glue_cspace *cspace,
                        struct block_device_operations_container *blo_container,
                        cptr_t *c_out);

int glue_cap_insert_blk_mq_queue_data_type(struct glue_cspace *cspace,
                        struct blk_mq_queue_data_container *bd_container,
                        cptr_t *c_out);

int glue_cap_insert_request_queue_type(struct glue_cspace *cspace,
                        struct request_queue_container *req_queue_container,
                        cptr_t *c_out);

int glue_cap_lookup_blk_mq_ops_type(struct glue_cspace *cspace,
                        cptr_t c,
                         struct blk_mq_ops_container **blk_mq_ops_container);

int glue_cap_lookup_blk_mq_tag_set_type(struct glue_cspace *cspace,
                        cptr_t c,
                        struct blk_mq_tag_set_container **set_container);

int glue_cap_lookup_gendisk_type(struct glue_cspace *cspace,
                        cptr_t c,
                        struct gendisk_container **disk_container);

int glue_cap_lookup_blk_mq_hw_ctx_type(struct glue_cspace *cspace,
                        cptr_t c,
                         struct blk_mq_hw_ctx_container **ctx_container);

int glue_cap_lookup_blk_mq_queue_data_type(struct glue_cspace *cspace,
                        cptr_t c,
                        struct blk_mq_queue_data_container **bd_container);

int glue_cap_lookup_module_type(struct glue_cspace *cspace,
                                cptr_t c,
                                struct module_container **module_container);

int glue_cap_lookup_request_queue_type(struct glue_cspace *cspace,
                        cptr_t c,
                        struct request_queue_container **req_queue_container);

int glue_cap_lookup_blk_dev_ops_type(struct glue_cspace *cspace,
		 	cptr_t c,
			struct block_device_operations_container **blo_container);

void glue_cap_remove(struct glue_cspace *cspace, cptr_t c);


/* ASYNC HELPERS -------------------------------------------------- */

static inline
int
async_msg_get_fn_type(struct fipc_message *msg)
{
	return msg->rpc_id;
}

static inline
void
async_msg_set_fn_type(struct fipc_message *msg, int type)
{
	msg->vmfunc_id = VMFUNC_RPC_CALL;
	msg->rpc_id = type;
}

#endif /* _GLUE_HELPER_H_ */
