#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/delayed_call.h>
#include<linux/fs_struct.h>
#include<asm/uaccess.h>
#include<linux/string.h>

#define MAX_BUF_SIZE 8192
#define DEVNAME "demo"


static int major;
static int numproc = 0;
atomic_t  device_opened;
static unsigned buf_size;
static unsigned current_usage = 0;
static short readpos=0;
static char msg[100]={0};
static int total_read = 0, total_write = 0;
static int demo_open(struct inode *inode, struct file *file)
{
	numproc++;
	if(numproc > 1)
	{
		printk(KERN_INFO "Number of read operations exceeded (read operation not allowed)");
		return -EINVAL;
	}
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
	

	printk("nump - %d\n",numproc);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);

	numproc--;

        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}
static ssize_t demo_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset)
{
	
	int count=0;
	int temp_pos = 0;
	if(numproc > 1)
	{
		printk(KERN_INFO "Number of read operations exceeded (read operation not allowed)");
		return -EINVAL;
	}

	printk(KERN_INFO "trying to support read......\n");
        //printk(KERN_INFO "Sorry, this operation isn't supported.\n");

	current_usage += length;
	while(length&&(msg[readpos]!=0)){
	 put_user(msg[readpos], buffer++);
	count++;
	 length--;
	 readpos++;
	}

	total_read += count;
        return count;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	short ind=len-1;
	short count=0;	

	if(numproc > 1)
	{
		printk(KERN_INFO "Number of read operations exceeded (read operation not allowed)");
		return -EINVAL;
	}
	printk(KERN_INFO "Trying to support write....\n");        
	//printk(KERN_INFO "Sorry, this operation isn't supported.\n");
	
	memset(msg,0,100);
	current_usage -= len;
	readpos=0;
	while(len>0){
	msg[count++]=buff[ind--];
	len--;	
	}
	total_write += count;
        return count;
}

static long demo_ioctl(struct file *file,
                 unsigned int ioctl_num,
                 unsigned long arg)

{
   printk(KERN_INFO "Sorry, this operation isn't supported.\n");
   return -EINVAL;
}
static struct file_operations fops = {
        .read = demo_read,
        .write = demo_write,
        .open = demo_open,
        .release = demo_release,
        .unlocked_ioctl = demo_ioctl,
};


static ssize_t demodev_buf_size_show(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%u\n", buf_size);
}

static ssize_t demodev_buf_size_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int err;
        unsigned long mode;

        if(current_usage){
            printk(KERN_INFO "Can not change size while buf being used\n");
            return -EINVAL;
        }
        err = kstrtoul(buf, 10, &mode);
        if (err || mode < 0 || mode > MAX_BUF_SIZE )
                return -EINVAL;

        buf_size = mode;
        return count;
}

static struct kobj_attribute demodev_buf_size_attribute = __ATTR(buffer_size,0644,demodev_buf_size_show, demodev_buf_size_set);

static ssize_t demodev_usage_count_show(struct kobject *kobj,
                                  struct kobj_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", atomic_read(&device_opened));
}
static struct kobj_attribute demodev_usage_count_attribute = __ATTR(usage_count, 0444,demodev_usage_count_show, NULL);

static struct attribute *demodev_attrs[] = {
        &demodev_buf_size_attribute.attr,
        &demodev_usage_count_attribute.attr,
        NULL,
};
static struct attribute_group demodev_attr_group = {
        .attrs = demodev_attrs,
        .name = "demodev",
};

int init_module(void)
{
        int ret;
	printk(KERN_INFO "Hello kernel\n");
        major = register_chrdev(0, DEVNAME, &fops);
        if (major < 0) {      
          printk(KERN_ALERT "Registering char device failed with %d\n", major);   
          return major;
        }                 
      
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVNAME, major);  
        atomic_set(&device_opened, 0);

        /*sysfs creation*/
         ret = sysfs_create_group (kernel_kobj, &demodev_attr_group);
         if(unlikely(ret))
                printk(KERN_INFO "demodev: can't create sysfs\n");
 
	return 0;
}

void cleanup_module(void)
{
        unregister_chrdev(major, DEVNAME);
        sysfs_remove_group (kernel_kobj, &demodev_attr_group);
	printk(KERN_INFO "Goodbye kernel\n");
}
MODULE_AUTHOR("deba@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
