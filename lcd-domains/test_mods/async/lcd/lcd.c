/*
 * lcd.c - code for isolated LCD in liblcd test
 */

#include <lcd_config/pre_hook.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <liblcd/liblcd.h>
#include <thc.h>

#include <lcd_config/post_hook.h>

void f1(void)
{
	LIBLCD_MSG("called f1, yielding...");
	THCYield();
	LIBLCD_MSG("back in f1 from yield");
}

void f2(void)
{
	LIBLCD_MSG("called f2, yielding...");
	THCYield();
	LIBLCD_MSG("back in f2 from yield");
}

void do_stuff(void)
{
	DO_FINISH(

		ASYNC(
			f1();

			);

		ASYNC(
			f2();

			);

		);

	LIBLCD_MSG("out of do-finish");
}

static int __noreturn async_lcd_init(void) 
{
	int r = 0;
	r = lcd_enter();
	if (r)
		goto out;
	
	do_stuff();

	goto out;

out:
	lcd_exit(r);
}

static int __async_lcd_init(void)
{
	int ret;

	LCD_MAIN({

			ret = async_lcd_init();

		});

	return ret;
}

static void async_lcd_exit(void)
{
	return;
}

module_init(__async_lcd_init);
module_exit(async_lcd_exit);
