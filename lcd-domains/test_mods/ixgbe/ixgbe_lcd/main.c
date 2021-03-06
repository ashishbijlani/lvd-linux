/*
 * main.c - runs when ixgbe lcd boots
 */

#include <lcd_config/pre_hook.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <liblcd/liblcd.h>
#include <liblcd/sync_ipc_poll.h>
#include "./ixgbe_caller.h"

#include <lcd_config/post_hook.h>

cptr_t ixgbe_register_channel;
struct thc_channel *ixgbe_async;
struct glue_cspace *ixgbe_cspace;
cptr_t ixgbe_sync_endpoint;
int ixgbe_done = 0;
struct thc_channel_group ch_grp;

int ixgbe_init_module(void);
void ixgbe_exit_module(void);
extern int create_async_channel(void);
unsigned long loops_per_jiffy;
bool poll_start = false;
extern int __ixgbe_poll(void);
unsigned long poll_state = 0ul;
#define LCD_ROUNDROBIN
/* LOOP ---------------------------------------- */

static void main_and_loop(void)
{
	int ret;
	int stop = 0;
	struct fipc_message *msg;

	DO_FINISH(
		ASYNC(
			ret = create_async_channel();
			if (ret) {
				LIBLCD_ERR("async channel creation failed");
				stop = 1;
			} else {
				LIBLCD_MSG("ASYNC channel established!");
			}

			ret = ixgbe_init_module();
			if (ret) {
				LIBLCD_ERR("ixgbe register failed");
				stop = 1;
			} else {
				LIBLCD_MSG("SUCCESSFULLY REGISTERED DUMMY!");
			}

			);

		/* By the time we hit this loop, the async channel
		 * will be set up (the awe running init_ixgbe_fs above
		 * will not yield until it tries to use the async
		 * channel). */
		while (!stop && !ixgbe_done) {
			struct thc_channel_group_item* curr_item;
#ifdef LCD_ROUNDROBIN
			/*
			 * Do RR async receive
			 */
			list_for_each_entry(curr_item, &(ch_grp.head), list) {
				ret = thc_ipc_poll_recv(thc_channel_group_item_channel(curr_item),
					&msg);

				// if we have a message for this channel
				if (!ret) {
					if (async_msg_get_fn_type(msg) == NDO_START_XMIT) {
						ret = ndo_start_xmit_clean_callee(msg,
							curr_item->channel,
							ixgbe_cspace,
							ixgbe_sync_endpoint);

						if (unlikely(ret)) {
							LIBLCD_ERR("async dispatch failed");
							stop = 1;
						}

						// XXX: is this polling freq good enough for full-duplex communication?
						{
							if (poll_start)
								ASYNC(__ixgbe_poll(););
							cpu_relax();
							continue;
						}
					} else {
					/*
					 * Got a message. Dispatch.
					 */
					ASYNC(

						ret = dispatch_async_loop(curr_item->channel, msg,
									ixgbe_cspace,
									ixgbe_sync_endpoint);
						if (ret) {
							LIBLCD_ERR("async dispatch failed");
							stop = 1;
						}

						);
					} // if (disp_loop)
				} // if (!ret)
			} // list

			// XXX: is this polling freq good enough for full-duplex communication?
			{
				if (poll_start)
					ASYNC(__ixgbe_poll(););
				cpu_relax();
				continue;
			}
#else
			/*
			 * Do one async receive
			 */
			ret = thc_poll_recv_group(&ch_grp,
						&curr_item, &msg);
			if (ret) {
				if (ret == -EWOULDBLOCK) {
				if (poll_start)
					ASYNC(__ixgbe_poll(););

					continue;
				} else {
					LIBLCD_ERR("async recv failed");
					stop = 1; /* stop */
				}
			}

			if (async_msg_get_fn_type(msg) == NDO_START_XMIT) {
				ret = ndo_start_xmit_clean_callee(msg,
					curr_item->channel,
					ixgbe_cspace,
					ixgbe_sync_endpoint);

				if (unlikely(ret)) {
					LIBLCD_ERR("async dispatch failed");
					stop = 1;
				}

			} else {
			/*
			 * Got a message. Dispatch.
			 */
			ASYNC(

				ret = dispatch_async_loop(curr_item->channel, msg,
							ixgbe_cspace,
							ixgbe_sync_endpoint);
				if (ret) {
					LIBLCD_ERR("async dispatch failed");
					stop = 1;
				}

				);
			}
			/* FIXME: This is pretty naive. This is *NOT* how
			 * we should do napi polling.
			 */
			if (poll_start)
				ASYNC(__ixgbe_poll(););
#endif
		}
		LIBLCD_MSG("IXGBE EXITED DISPATCH LOOP");
		);

	LIBLCD_MSG("EXITED IXGBE DO_FINISH");

	return;
}

static int __noreturn ixgbe_lcd_init(void)
{
	int r = 0;

	printk("IXGBE LCD enter \n");
	r = lcd_enter();
	if (r)
		goto fail1;
	/*
	 * Get the ixgbe channel cptr from boot info
	 */
	ixgbe_register_channel = lcd_get_boot_info()->cptrs[0];
	loops_per_jiffy = lcd_get_boot_info()->cptrs[1].cptr;

	printk("ixgbe reg channel %lu\n", ixgbe_register_channel.cptr);
	printk("ixgbe lpj %lu\n", loops_per_jiffy);
	/*
	 * Initialize ixgbe glue
	 */
	r = glue_ixgbe_init();
	if (r) {
		LIBLCD_ERR("ixgbe init");
		goto fail2;
	}

	thc_channel_group_init(&ch_grp);
	/* RUN CODE / LOOP ---------------------------------------- */

	main_and_loop();

	/* DONE -------------------------------------------------- */

	glue_ixgbe_exit();

	lcd_exit(0); /* doesn't return */
fail2:
fail1:
	lcd_exit(r);
}

static int __ixgbe_lcd_init(void)
{
	int ret;

	LIBLCD_MSG("%s: entering", __func__);

	LCD_MAIN({

			ret = ixgbe_lcd_init();

		});

	return ret;
}

static void __exit ixgbe_lcd_exit(void)
{
	LIBLCD_MSG("%s: exiting", __func__);
	return;
}

module_init(__ixgbe_lcd_init);
module_exit(ixgbe_lcd_exit);
MODULE_LICENSE("GPL");

