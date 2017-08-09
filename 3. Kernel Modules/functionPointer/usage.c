#include <linux/kernel.h>	
#include <linux/module.h>	
#include <linux/init.h>		

extern void * fn;	

static char* text = "a";
module_param(text, charp, 0);

// modify this any no. of times
void parse(char *p)
{
	if (strstr(p,text) != NULL )
	{
		printk("file - %s opened", p);
	}
}

static int __init fp_init(void)
{
	printk(KERN_INFO "fp init \n");
	fn = parse;

	return 0;
}

static void __exit fp_exit(void)
{
	printk(KERN_INFO "fp exit\n");
	fn = NULL;
}

module_init(fp_init);
module_exit(fp_exit);
