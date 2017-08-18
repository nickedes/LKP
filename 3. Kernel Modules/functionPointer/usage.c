#include <linux/kernel.h>	
#include <linux/module.h>	
#include <linux/init.h>		
#include <linux/sched.h>
#include <linux/fs_struct.h>
#include <linux/fs.h>

extern void * fn;	

static char* text = "/home";
module_param(text, charp, 0);

void parse(char *p)
{
	struct dentry *entry = current->fs->pwd.dentry;
	char *y;
	char buf[256];
	y = dentry_path_raw(entry, buf, 256);
	if(strstr(y, text) != NULL)
	{
		printk("opened file %s", p);
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
