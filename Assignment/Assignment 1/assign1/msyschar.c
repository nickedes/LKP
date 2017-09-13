#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/time.h>
#include<linux/fs_struct.h>
#include <linux/uaccess.h>
#include<linux/string.h>
#include <linux/semaphore.h>
#include "syschar.h"
#define MAX_BUF_SIZE 8192
#define DEVNAME "demo2"
#define MAXPROC 4
#define INIT 5
#define MAX_MAP 1000
#define MAX_MSG 1000
#define MAX_LOGIN 1000

int get_EmptyMapSlot(void);
int get_EmptyMsgSlot(void);
static int major;
atomic_t  device_opened;
static unsigned buf_size;
static unsigned current_usage = 0;
static short readpos=0;
static int total_write = 0;

static struct map {
    int pid;
    int msg_id;
    int flag;
    int delete;
}maps[MAX_MAP];

static struct mesg
{
    int pid; // sender
    unsigned int time;
    char message[100];
    int num_reads;
    int delete;
}msgs[MAX_MSG];


static struct login{
    int pid;
    unsigned int time;
    char handle[100];
}logins[MAX_LOGIN];

static int login_index = 0;
static int logstore[10000] = {0};

static int msg_index = 0;
static int map_index = 0;

static int demo_open(struct inode *inode, struct file *file)
{
    int i;
    for (i = 0; i < MAX_MSG; ++i)
    {
        if(msgs[i].pid != 0 && msgs[i].num_reads == 0 && msgs[i].delete == 1)
        {
            printk("deleted msg : %s", msgs[i].message);
        }
    }
    atomic_inc(&device_opened);
    try_module_get(THIS_MODULE);
    printk(KERN_INFO "Device opened successfully\n");
    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}

static ssize_t demo_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset)
{
    
    int i,j, pid, msgId, sender, retval = -EINVAL;
    char send_msg_handle[1000]={0};

    printk(KERN_INFO "trying to support read......\n");
    pid = current->pid;
    for (i = 0; i < MAX_MAP; i++)
    {
        if(maps[i].pid == pid && maps[i].flag == 1 && maps[i].delete == 0)
        {
            msgId = maps[i].msg_id;            
            // sending process
            sender = msgs[msgId].pid;
            
            // combine handle + msg
            strcat(send_msg_handle, logins[ logstore[sender] ].handle);
            strcat(send_msg_handle, " :");
            strcat(send_msg_handle, msgs[msgId].message);
            strcat(send_msg_handle, "\n");

            // marked as read!
            maps[i].flag = 0;
            msgs[msgId].num_reads--;
        }
    }
    // printk("string read - %s", send_msg_handle);
    retval = copy_to_user(buffer, send_msg_handle, length);

    // deletion of message
    for (i = 0; i < MAX_MSG; i++)
    {
        if (msgs[i].num_reads == 0)
        {
            msgs[i].delete = 1;

            for (j = 0; j < MAX_MAP; ++j)
            {
                if(maps[j].msg_id == i)
                    maps[j].delete = 1;
            }
        }
    }
    return retval;
}

static ssize_t
demo_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    int ret, i;
    struct timeval now;
    unsigned int temp;
    printk(KERN_INFO "Trying to support write....\n");        
    msg_index = get_EmptyMsgSlot();
    //printk(KERN_INFO "Sorry, this operation isn't supported.\n");
    
    memset(msgs[msg_index].message , 0, 100);
    current_usage -= len;
    readpos=0;
    do_gettimeofday(&now);
    temp = now.tv_sec;
    msgs[msg_index].pid = current->pid;
    msgs[msg_index].time = temp;
    ret = copy_from_user(msgs[msg_index].message, buff, len);
    msgs[msg_index].num_reads = 0;
    msgs[msg_index].delete = 0;
    // get no. of readers
    for (i = 0; i < login_index; ++i)
    {
        if(logins[i].time < msgs[msg_index].time && logins[i].pid != msgs[msg_index].pid)
        {
            // increment readers!
            msgs[msg_index].num_reads = msgs[msg_index].num_reads + 1;

            map_index = get_EmptyMapSlot();
            maps[map_index].pid = logins[i].pid;
            maps[map_index].msg_id = msg_index;
            // set flag for reading
            maps[map_index].flag = 1;
            maps[map_index].delete = 0;
        }
    }

    if (ret == 0)
    {
    total_write += len;
    return len;
    }
    return ret;
}

static long demo_ioctl(struct file *file,
 unsigned int ioctl_num,
 unsigned long arg)

{
    int retval = -EINVAL, temp, i;
    struct timeval now;
    unsigned int timestamp;
    switch(ioctl_num){
      case IOCTL_LOGIN:
            do_gettimeofday(&now);
            timestamp = now.tv_sec;
            memset(logins[login_index].handle, 0, 100);
            logins[login_index].pid = current->pid;
            logins[login_index].time = timestamp;
            retval = copy_from_user(logins[login_index].handle, (char *)arg, strlen((char *)arg));
            logstore[current->pid] = login_index;
            printk("Login successfully");
            // printk("handle - %s", logins[login_index].handle);
            // printk("pid - %d", logins[login_index].pid);
            // printk("time - %d", logins[login_index].time);
            login_index++;
            retval = 0;
            break;
      case IOCTL_LOGOUT:
            temp = logstore[current->pid];
            for (i = temp; i < login_index; ++i)
            {
                logins[i] = logins[i+1];
                logstore[ logins[i].pid ] = i;
            }
            logstore[current->pid] = 0;
            // printk("pid -%d", current->pid);
            printk("logout successfully");
            login_index--;
            retval = 0;
            break;
      default:  
      printk(KERN_INFO "Sorry, this operation isn't supported.\n");
  }
  return retval;
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
        int i = 0, ret;
    printk(KERN_INFO "Hello kernel\n");
        major = register_chrdev(0, DEVNAME, &fops);
        if (major < 0) {      
          printk(KERN_ALERT "Registering char device failed with %d\n", major);   
          return major;
        }                 
      
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVNAME, major);  
        atomic_set(&device_opened, 0);

        // delete 1 means the map entry is deleted or empty
        for (i = 0; i < MAX_MAP; i++)
            maps[i].delete = 1;

        // delete 1 means the msg entry is deleted or empty
        for (i = 0; i < MAX_MSG; i++)
            msgs[i].delete = 1;

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

int get_EmptyMapSlot(void)
{
    int i;
    for(i = 0; i < MAX_MAP; ++i)
    {
        if (maps[i].delete == 1)
            break;
    }
    return i;
}

int get_EmptyMsgSlot(void)
{
    int i;
    for(i = 0; i < MAX_MSG; ++i)
    {
        if (msgs[i].delete == 1)
            break;
    }
    return i;
}


MODULE_AUTHOR("nickedes@cse.iitk.ac.in");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Demo modules");
