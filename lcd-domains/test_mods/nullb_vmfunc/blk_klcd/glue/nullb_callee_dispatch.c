#include <lcd_config/pre_hook.h>
#include <liblcd/liblcd.h>
#include "../nullb_callee.h"
#include <lcd_config/post_hook.h>

#define trace(x) LIBLCD_MSG("blk got " #x " msg")
int blk_klcd_syncipc_dispatch(struct fipc_message *message)
{
        int fn_type;

	fn_type = async_msg_get_fn_type(message);

        switch (fn_type) {
	case INIT_HCTX_SYNC:
		trace(INIT_HCTX_SYNC);
		return init_hctx_sync_callee(message);
        default:
                LIBLCD_ERR("unexpected function label %d", fn_type);
                return -EINVAL;
        }

        return 0;
}


int blk_klcd_dispatch_loop(struct fipc_message *message)
{
	int fn_type;
	fn_type = async_msg_get_fn_type(message);
	switch (fn_type) {
		case BLK_MQ_ALLOC_TAG_SET:
			trace(BLK_MQ_ALLOC_TAG_SET);
			return blk_mq_alloc_tag_set_callee(message);

		case BLK_MQ_INIT_QUEUE:
			trace(BLK_MQ_INIT_QUEUE);
			return blk_mq_init_queue_callee(message);

		case BLK_MQ_END_REQUEST:
			/* trace(BLK_MQ_END_REQUEST); */
			return blk_mq_end_request_callee(message);

		case BLK_MQ_FREE_TAG_SET:
			trace(BLK_MQ_FREE_TAG_SET);
			return blk_mq_free_tag_set_callee(message);

		case BLK_MQ_START_REQUEST:
			/* trace(BLK_MQ_START_REQUEST); */
			return blk_mq_start_request_callee(message);

		case BLK_MQ_MAP_QUEUE:
			trace(BLK_MQ_MAP_QUEUE);
			return blk_mq_map_queue_callee(message);

		case BLK_QUEUE_LOGICAL_BLOCK_SIZE:
			trace(BLK_QUEUE_LOGICAL_BLOCK_SIZE);
			return blk_queue_logical_block_size_callee(message);

		case BLK_QUEUE_PHYSICAL_BLOCK_SIZE:
			trace(BLK_QUEUE_PHYSICAL_BLOCK_SIZE);
			return blk_queue_physical_block_size_callee(message);

		case BLK_CLEANUP_QUEUE:
			trace(BLK_CLEANUP_QUEUE);
			return blk_cleanup_queue_callee(message);

		case ALLOC_DISK:
			trace(ALLOC_DISK);
			return alloc_disk_node_callee(message);

		case DEVICE_ADD_DISK:
			trace(ADD_DISK);
			return device_add_disk_callee(message);

		case PUT_DISK:
			trace(PUT_DISK);
			return put_disk_callee(message);

		case DEL_GENDISK:
			trace(DEL_GENDISK);
			return del_gendisk_callee(message);

		case REGISTER_BLKDEV:
			trace(REGISTER_BLKDEV);
			return register_blkdev_callee(message);

		case UNREGISTER_BLKDEV:
			trace(UNREGISTER_BLKDEV);
			return unregister_blkdev_callee(message);
		
		default:
			LIBLCD_ERR("unexpected function label: %d", fn_type);
			return -EINVAL;

	}
	return 0;
}
