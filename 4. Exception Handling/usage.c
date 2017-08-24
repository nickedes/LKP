#include <linux/kernel.h>	
#include <linux/module.h>	
#include <linux/init.h>		
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/sched/signal.h>
#include <linux/ptrace.h>

extern int(*exceptionHandler)(struct pt_regs *regs, int sig);

int parse(struct pt_regs *regs, int sig)
{
	if(sig == SIGFPE)
	{
		regs->ip += 2;
		printk("in module\n");
		return 0;
	}
	return 0;
}

static int __init fp_init(void)
{
	printk(KERN_INFO "exception handler init \n");
	exceptionHandler = parse;

	return 0;
}

static void __exit fp_exit(void)
{
	printk(KERN_INFO "exception handler exit\n");
	exceptionHandler = NULL;
}

module_init(fp_init);
module_exit(fp_exit);
