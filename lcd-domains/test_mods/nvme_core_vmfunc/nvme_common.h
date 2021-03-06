#ifndef __NVME_COMMON_H__
#define __NVME_COMMON_H__

#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>

#include <libcap.h>
#include <libfipc.h>
#include <thc_ipc.h>
#include <liblcd/glue_cspace.h>
#include <liblcd/liblcd.h>
#include <liblcd/sync_ipc_poll.h>
#include <linux/priv_mempool.h>
#include <asm/lcd_domains/libvmfunc.h>

#include "nvme_glue_helper.h"

#define PCI_REGIONS
#define IOMMU_ASSIGN
#define HOST_IRQ
#define SENDER_DISPATCH_LOOP
#define PASS_DEV_ADDR_IN_REG
//#define CONFIG_VMALLOC_SHARED_POOL

enum dispatch_t {
	__PCI_REGISTER_DRIVER,
	PCI_UNREGISTER_DRIVER,
	PCI_DISABLE_PCIE_ERROR_REPORTING,
	PCI_BUS_READ_CONFIG_WORD,
	PCI_BUS_WRITE_CONFIG_WORD,
	PCI_CLEANUP_AER_UNCORRECT_ERROR_STATUS,
	PCI_DISABLE_DEVICE,
	PCI_DISABLE_MSIX,
	PCI_DISABLE_MSI,
	PCI_ENABLE_MSIX,
	PCI_ENABLE_MSIX_RANGE,
	PCI_ENABLE_MSI_RANGE,
	PCI_ENABLE_PCIE_ERROR_REPORTING,
	PCIE_CAPABILITY_READ_WORD,
	PCIE_GET_MINIMUM_LINK,
	PCI_ENABLE_DEVICE_MEM,
	PCI_REQUEST_SELECTED_REGIONS,
	PCI_REQUEST_SELECTED_REGIONS_EXCLUSIVE,
	PCI_SET_MASTER,
	PCI_SAVE_STATE,
	PCI_RESTORE_STATE,
	PCI_RELEASE_SELECTED_REGIONS,
	PCI_SELECT_BARS,
	PCI_WAKE_FROM_D3,
	PCI_DEVICE_IS_PRESENT,
	PCI_VFS_ASSIGNED,
	PUT_DEVICE,
	DMA_POOL_ALLOC,
	DMA_POOL_CREATE,
	DMA_POOL_DESTROY,
	DMA_POOL_FREE,
	PROBE,
	REMOVE,
	SYNC,
	UNSYNC,
	POLL,
	REQUEST_THREADED_IRQ,
	FREE_IRQ,
	MSIX_IRQ_HANDLER,
	IRQ_SET_AFFINITY_HINT,
	TRIGGER_EXIT,
	SERVICE_EVENT_SCHED,
	TRIGGER_DUMP,
	TRIGGER_CLEAN,
	BLK_EXECUTE_RQ_NOWAIT,
	BLK_GET_QUEUE,
	BLK_MQ_COMPLETE_REQUEST,
	BLK_MQ_FREE_REQUEST,
	BLK_MQ_START_STOPPED_HW_QUEUES,
	BLK_MQ_TAGS_CPUMASK,
	BLK_MQ_TAGSET_BUSY_ITER,
	BLK_MQ_TAG_TO_RQ,
	BLK_MQ_UPDATE_NR_HW_QUEUES,
	BLK_PUT_QUEUE,
	BLK_RQ_MAP_KERN,
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
	GET_DEVICE,
	DEVICE_RELEASE_DRIVER,
	QUEUE_RQ_FN,
	COMPLETE_FN,
	MAP_QUEUE_FN,
	INIT_HCTX_FN,
	INIT_HCTX_SYNC,
	EXIT_HCTX_FN,
	INIT_REQUEST_FN,
	TIMEOUT_FN,
	POLL_FN,
	__UNREGISTER_CHRDEV,
	__REGISTER_CHRDEV,
	IDA_DESTROY,
	IDA_GET_NEW_ABOVE,
	IDA_INIT,
	IDA_PRE_GET,
	IDA_REMOVE,
	IDA_SIMPLE_GET,
	IDA_SIMPLE_REMOVE,
	BLK_SET_QUEUE_DYING,
	BLK_RQ_UNMAP_USER,
	BLK_QUEUE_WRITE_CACHE,
	BLK_QUEUE_VIRT_BOUNDARY,
	BLK_QUEUE_MAX_SEGMENTS,
	BLK_QUEUE_MAX_HW_SECTORS,
	BLK_QUEUE_MAX_DISCARD_SECTORS,
	BLK_QUEUE_CHUNK_SECTORS,
	BLK_MQ_UNFREEZE_QUEUE,
	BLK_MQ_STOP_HW_QUEUES,
	BLK_MQ_REQUEUE_REQUEST,
	BLK_MQ_REQUEST_STARTED,
	BLK_MQ_KICK_REQUEUE_LIST,
	BLK_MQ_FREEZE_QUEUE,
	BLK_MQ_CANCEL_REQUEUE_WORK,
	BLK_MQ_ALLOC_REQUEST_HCTX,
	BLK_MQ_ALLOC_REQUEST,
	BLK_MQ_ABORT_REQUEUE_LIST,
	BLK_EXECUTE_RQ,
	BDPUT,
	BDGET_DISK,
	__CLASS_CREATE,
	CLASS_DESTROY,
	DEVICE_CREATE,
	DEVICE_DESTROY,
	FOPS_OPEN_FN,
	FOPS_RELEASE_FN,
	FOPS_UNLOCKED_IOCTL_FN,
	BD_OPEN_FN,
	BD_RELEASE_FN,
	BD_IOCTL_FN,
	BD_GETGEO_FN,
	BD_REVALIDATE_DISK_FN,
	MODULE_INIT,
	MODULE_EXIT,
	SYNC_PROBE,
	SYNC_NDO_SET_MAC_ADDRESS,
	SYNCHRONIZE_IRQ,
	CAPABLE,
	REVALIDATE_DISK,
	COPY_FROM_USER,
	COPY_TO_USER,
};

enum sub_cmd {
	NVME_PASSTHRU_CMD,
	NVME_SUBMIT_IO,
};
#define LOWER32_BITS    32
#define LOWER_HALF(x)   (x & ((1ULL << LOWER32_BITS) - 1))
#define UPPER_HALF(x)   (x >> LOWER32_BITS)

#define INIT_FIPC_MSG(msg)		memset(msg, 0x0, sizeof(*msg))

struct pcidev_info {
	unsigned int domain, bus, slot, fn;
};

#define MAX_RQ_BUFS	64

union rq_pack {
	struct {
		uint8_t qnum;
		uint8_t cmd_typ;
		uint16_t tag;
		uint32_t rq_bytes;
	};
	uint64_t reg;
};

/* CSPACES ------------------------------------------------------------ */
int glue_cap_init(void);

int glue_cap_create(struct glue_cspace **cspace);

void glue_cap_destroy(struct glue_cspace *cspace);

void glue_cap_exit(void);

void glue_cap_remove( struct glue_cspace *cspace, cptr_t c);

#define INIT_IPC_MSG(m)		memset(m, 0x0, sizeof(*m))
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

#endif /* __NVME_COMMON_H__ */
