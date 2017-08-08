#include <linux/module.h> /* Needed by all modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */

static int number = 0;
module_param(number, int, 0);

int init_module(void)
{
	if(number < 0)
	{
		printk(KERN_INFO "negative\n");
	}
	else if(number == 0)
	{
		printk(KERN_INFO "zero\n");
	}
	else
	{
		printk(KERN_INFO "positive\n");
	}
	/*
	* A non 0 return means init_module failed; module can't be loaded.
	*/
	return 0;
}
void cleanup_module(void)
{
	printk(KERN_INFO "bye kernel\n");
}
