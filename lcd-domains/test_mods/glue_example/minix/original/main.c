/**
 * main.c - implementation of fake minix
 */

#include <lcd_config/pre_hook.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "../../include/vfs.h"

#include <lcd_config/post_hook.h>

#ifdef LCD_ISOLATE
struct file_container {
	struct file file;
	u64 e1;
	u64 e2;
};
#endif

static int minix_new_file(int id, struct file **file_out) {

	struct file *f;
#ifdef LCD_ISOLATE
	struct file_container *file_container;
#endif

	if (id != 179) {
		printk(KERN_ERR "unexpected file id");
		return -EINVAL;
	}

	printk("sizeof file_container is 0x%lx", sizeof(*file_container));

#ifdef LCD_ISOLATE
	f = NULL;
	file_container = kmalloc(sizeof(*file_container), GFP_KERNEL);
	if (file_container)
		f = &file_container->file;
#else
	f = kmalloc(sizeof(*f), GFP_KERNEL);
#endif
	if (!f)
		return -ENOMEM;
	f->id = id;
	f->private = 1024;
	*file_out = f;
	return 0;

}

static void minix_rm_file(struct file *file) {

	if (file->private != 1024 || file->id != 179) {

		printk(KERN_ERR "fake minix: unexpected file fields:\n");
		printk(KERN_ERR "   id: %d, private field: %d\n",
			file->id, file->private);
		return;

	}

	kfree(file);
	return;
}

#ifdef LCD_ISOLATE

/* Ideally, we shouldn't bring these in to unmodified code. I
 * converted e.g. cptr --> u64 to avoid bringing in those defs. */
struct ipc_channel {
	int type;
	u64 x;
	struct list_head channel_list;
};
struct fs_container {
	struct fs fs;
	u64 x1;
	u64 x2;
	u64 x3;
};
struct fs_operations_container {
	struct fs_operations fs_operations;
	u64 x1;
	u64 x2;
	struct ipc_channel chnl;
};
static struct fs_operations_container minix_fs_ops_container = {
	.fs_operations = {
		.new_file = minix_new_file,
		.rm_file = minix_rm_file
	}
};
static struct fs_container minix_fs_container = {
	.fs = {
		.id = 15,
		.size = 512,
		.fs_operations = &(minix_fs_ops_container.fs_operations)
	}
};

#else /* !LCD_ISOLATE */

static struct fs_operations minix_fs_ops = {
	.new_file = minix_new_file,
	.rm_file = minix_rm_file
};
static struct fs minix_fs = {
	.id = 15,
	.size = 512,
	.fs_operations = &minix_fs_ops
};

#endif


int 
#ifndef LCD_ISOLATE
__init 
#endif
original_minix_lcd_init(void) {
	/*
	 * Register.
	 */
#ifdef LCD_ISOLATE
	return register_fs(&minix_fs_container.fs);
#else
	return register_fs(&minix_fs);
#endif
}

void 
#ifndef LCD_ISOLATE
__exit 
#endif
original_minix_lcd_exit(void) {
	/*
	 * Unregister.
	 */
#ifdef LCD_ISOLATE
	unregister_fs(&minix_fs_container.fs);
#else
	unregister_fs(&minix_fs);
#endif
}

#ifndef LCD_ISOLATE
module_init(original_minix_lcd_init);
module_exit(original_minix_lcd_exit);
#endif
