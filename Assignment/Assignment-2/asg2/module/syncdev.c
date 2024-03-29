#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/delay.h>
#include <asm/uaccess.h>	
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>

#define DEVICE_NAME "syncdev"	
#define PAUSE_COUNT 20


static int Major;		
atomic_t  open_count;

// Implementation of spin lock
static inline unsigned xchg_c(void *lock, unsigned reg)
{
  __asm__ __volatile__("xchgl %0,%1":"=r" ((unsigned) reg):"m" (*(volatile unsigned *)lock), "0" (reg):"memory");
  return reg;
}

#define STORE 1
typedef unsigned custom_spinlock;

static void cspin_lock(custom_spinlock *lock)
{
  while (1)
  {
    // return if lock becomes 1, => locked
    if (xchg_c(lock, STORE) == 0) return;
  
    while (*lock) cpu_relax();
  }
}

static void cspin_unlock(custom_spinlock *lock)
{
  barrier();
  *lock = 0;
}

typedef struct custom_rwlock custom_rwlock;
struct custom_rwlock
{
  custom_spinlock lock;
  atomic_t readers;
};

static void custom_wrlock(custom_rwlock *l)
{
  // Get write lock
  cspin_lock(&l->lock);
  
  // let readers finish
  while (atomic_read(&l->readers)) cpu_relax();
}
static void custom_wrunlock(custom_rwlock *l)
{
  // just unset the lock
  cspin_unlock(&l->lock);
}

static void custom_init(custom_rwlock *l)
{
  atomic_set(&l->readers,0);
}

static void custom_rdlock(custom_rwlock *l)
{
  while (1)
  {
    // take read lock
    atomic_inc(&l->readers);
    
    // if Success? then return (i.e. if not locked for writing)
    if (!l->lock) return;
    
    // if Failure - undo the reader, and wait until writer unlocks
    atomic_dec(&l->readers);
    while (l->lock) cpu_relax();
  }
}

static void custom_rdunlock(custom_rwlock *l)
{
  atomic_dec(&l->readers);
}


struct data{
             long num_writes;
             long counter;
             long result;
             int lock_type;
             struct rcu_head rcu;
             struct cs_handler *handler; /*generic pointer to your lock specific 
                                         implementations*/
};

struct data *gdata;

typedef enum{
                  NONE=0,
                  SPINLOCK,


                  RWLOCK,                 /*kernel read/write lock*/
                  SEQLOCK,                /*kernel seqlock*/
                  RCU,                    /*kernel RCU*/
                  RWLOCK_CUSTOM,          /*Your custom read/write lock*/
                  RESEARCH_LOCK,          /*To improve over RCU*/
                  MAX_LOCK_TYPE
                 
}LOCK_TYPE;



struct cs_handler{
                     
                     union{
                              spinlock_t spin;
                              rwlock_t rwlock;
                              seqlock_t seqlock;
                              custom_rwlock crwlock; /*Add your custom lock type here*/
                              
                     };
                     
                     
                     /*Two functions below should be called from next
                       corresponding lock implementation function, respectively. 
                       Ex: read_data implementation of the locking mechanism must 
                       call mustcall_read with appropriate parameters*/

                     int (*mustcall_read)(struct data *gd, char *buf); /*returns the length read*/
                     int (*mustcall_write)(struct data *gd); 
                     
                     /* These functions are implemented depending on the lock
                       type used */
                     int (*init_cs)(struct data *gd);
                     int (*read_data)(struct data *gd, char *buf);
                     int (*write_data)(struct data *gd);
                     int (*cleanup_cs)(struct data *gd);
};

/*Prototype for lock implementations*/

/*XXX NO lock*/
static int nolock_init_cs(struct data *gd);
static int nolock_cleanup_cs(struct data *gd);
static int nolock_write_data(struct data *gd);
static int nolock_read_data(struct data *gd, char *buf);

/*XXX spin lock*/
static int spinlock_init_cs(struct data *gd);
static int spinlock_cleanup_cs(struct data *gd);
static int spinlock_write_data(struct data *gd);
static int spinlock_read_data(struct data *gd, char *buf);

/*XXX read/write lock*/
static int rwlock_init_cs(struct data *gd);
static int rwlock_cleanup_cs(struct data *gd);
static int rwlock_write_data(struct data *gd);
static int rwlock_read_data(struct data *gd, char *buf);

/*XXX seq lock*/
static int seqlock_init_cs(struct data *gd);
static int seqlock_cleanup_cs(struct data *gd);
static int seqlock_write_data(struct data *gd);
static int seqlock_read_data(struct data *gd, char *buf);

/*XXX rcu lock*/
static int rculock_init_cs(struct data *gd);
static int rculock_cleanup_cs(struct data *gd);
static int rculock_write_data(struct data *gd);
static int rculock_read_data(struct data *gd, char *buf);
void gd_reclaim(struct rcu_head *rp);

/*XXX custom lock*/
static int customlock_init_cs(struct data *gd);
static int customlock_cleanup_cs(struct data *gd);
static int customlock_write_data(struct data *gd);
static int customlock_read_data(struct data *gd, char *buf);

static  int readit(struct data *gd, char *buf)
{
        int retval;
        int pctr = PAUSE_COUNT;

        while(pctr--)   /*Spend some cpu cycles w/o performing anything useful*/
          cpu_relax();
        
        retval = sprintf(buf, "%ld %ld %ld", gd->num_writes, gd->counter, gd->result);
        
        pctr = PAUSE_COUNT;           
        

        return retval;
}

static int writeit(struct data *gd)
{

        int pctr = PAUSE_COUNT;
        gd->counter++;

        while(pctr--)   /*Spend some cpu cycles w/o performing anything useful*/
          cpu_relax();
     
        gd->num_writes++;
        
        pctr = PAUSE_COUNT;           
        
        while(pctr--)  /*Spend some cpu cycles w/o performing anything useful*/
          cpu_relax();

        gd->result = gd->num_writes + gd->counter;

	return 0;
}

static inline int set_cs_implementation(struct cs_handler *handler, unsigned newlock_type)
{
        if(newlock_type >= MAX_LOCK_TYPE)
               return -EINVAL;
        switch(newlock_type)
        {
            case NONE:
                         handler->init_cs = nolock_init_cs;
                         handler->cleanup_cs = nolock_cleanup_cs;
                         handler->read_data = nolock_read_data;
                         handler->write_data = nolock_write_data;
                         break;
           case SPINLOCK:
                         handler->init_cs = spinlock_init_cs;
                         handler->cleanup_cs = spinlock_cleanup_cs;
                         handler->read_data = spinlock_read_data;
                         handler->write_data = spinlock_write_data;
                         break;
           case RWLOCK:                /*kernel read/write lock*/
                         handler->init_cs = rwlock_init_cs;
                         handler->cleanup_cs = rwlock_cleanup_cs;
                         handler->read_data = rwlock_read_data;
                         handler->write_data = rwlock_write_data;
                         break;
           case SEQLOCK:               /*kernel seqlock*/
                         handler->init_cs = seqlock_init_cs;
                         handler->cleanup_cs = seqlock_cleanup_cs;
                         handler->read_data = seqlock_read_data;
                         handler->write_data = seqlock_write_data;
                         break;
           case RCU:                    /*kernel RCU*/
                         handler->init_cs = rculock_init_cs;
                         handler->cleanup_cs = rculock_cleanup_cs;
                         handler->read_data = rculock_read_data;
                         handler->write_data = rculock_write_data;
                         break;
           case RWLOCK_CUSTOM:         /*Your custom read/write lock*/
                         handler->init_cs = customlock_init_cs;
                         handler->cleanup_cs = customlock_cleanup_cs;
                         handler->read_data = customlock_read_data;
                         handler->write_data = customlock_write_data;
                         break;
           case RESEARCH_LOCK:          /*To improve over RCU*/
           default:
                    printk(KERN_INFO "Not implemented currently, you have to implement these as shown for the first two cases\n");
                    return -EINVAL;
        }
      return 0;
}

static inline int init_cs_handler(struct data *gd)
{
  struct cs_handler *handler;

  BUG_ON(!gd || gd->handler);

  handler = kzalloc(sizeof(struct cs_handler), GFP_KERNEL);
  if(!handler)
       return -ENOMEM;
  handler->mustcall_read = readit;
  handler->mustcall_write = writeit;
  if(set_cs_implementation(handler, gd->lock_type))
      return -EINVAL;
  gd->handler = handler;
  return 0;   
}

static  inline int free_cs_handler(struct data *gd)
{
   kfree(gd->handler);
   return 0;
}

//////////////////////////////////////////////////////////////////////////////////
/*All lock implementations go here. On sysfs variable change, you should change
  the last four functions of the data->handler to your implementation*/
//////////////////////////////////////////////////////////////////////////////////

/*No lock implementation*/

int nolock_init_cs(struct data *gd)
{
   return 0;
}
int nolock_cleanup_cs(struct data *gd)
{
   return 0;
}

int nolock_write_data(struct data *gd)
{

  BUG_ON(!gdata->handler->mustcall_write);
  gdata->handler->mustcall_write(gd); 
  return 0;
}         
int nolock_read_data(struct data *gd, char *buf)
{

  BUG_ON(!gdata->handler->mustcall_read);
  gdata->handler->mustcall_read(gd, buf); 
  return 0;
}         

/*Spin lock implementation*/

int spinlock_init_cs(struct data *gd)
{
   struct cs_handler *handler = gd->handler;
   spin_lock_init(&handler->spin);
   return 0;
}
int spinlock_cleanup_cs(struct data *gd)
{
   return 0;
}

int spinlock_write_data(struct data *gd)
{

  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);
  
  spin_lock(&handler->spin);
  handler->mustcall_write(gd);  /*Call the Write CS*/
  spin_unlock(&handler->spin);

  return 0;
}         
int spinlock_read_data(struct data *gd, char *buf)
{
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);

  spin_lock(&handler->spin);
  handler->mustcall_read(gd, buf);  /*Call the read CS*/
  spin_unlock(&handler->spin);
  return 0;
}

/* read/write lock implementation*/

int rwlock_init_cs(struct data *gd)
{
   struct cs_handler *handler = gd->handler;
   rwlock_init(&handler->rwlock);
   return 0;
}

int rwlock_cleanup_cs(struct data *gd)
{
   return 0;
}

int rwlock_write_data(struct data *gd)
{
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);
  
  write_lock(&handler->rwlock);
  handler->mustcall_write(gd);  /*Call the Write CS*/
  write_unlock(&handler->rwlock);

  return 0;
}        

int rwlock_read_data(struct data *gd, char *buf)
{
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);

  read_lock(&handler->rwlock);
  handler->mustcall_read(gd, buf);  /*Call the read CS*/
  read_unlock(&handler->rwlock);
  return 0;
}

/* sequential lock implementation*/

int seqlock_init_cs(struct data *gd)
{
   struct cs_handler *handler = gd->handler;
   seqlock_init(&handler->seqlock);
   return 0;
}

int seqlock_cleanup_cs(struct data *gd)
{
   return 0;
}

int seqlock_write_data(struct data *gd)
{
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);
  
  write_seqlock(&handler->seqlock);
  handler->mustcall_write(gd);  /*Call the Write CS*/
  write_sequnlock(&handler->seqlock);

  return 0;
}        

int seqlock_read_data(struct data *gd, char *buf)
{
  unsigned int seq;
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);

  do {
    seq = read_seqbegin(&handler->seqlock);
    handler->mustcall_read(gd, buf);  /*Call the read CS*/
  } while(read_seqretry(&handler->seqlock, seq));

  return 0;
}


/* rcu lock implementation*/

int rculock_init_cs(struct data *gd)
{
  struct cs_handler *handler = gd->handler;
  init_rcu_head(&gd->rcu);
  spin_lock_init(&handler->spin);
  return 0;
}

int rculock_cleanup_cs(struct data *gd)
{
    return 0;
}

int rculock_write_data(struct data *gd)
{
  struct cs_handler *handler = gd->handler;
  struct data *new_gd;
  struct data *old_gd;
  BUG_ON(!handler->mustcall_write);
  new_gd = kmalloc(sizeof(*new_gd), GFP_KERNEL);

  spin_lock(&handler->spin);
  old_gd = rcu_dereference_protected(gdata, lockdep_is_held(&handler->spin));
  *new_gd = *old_gd;
  handler->mustcall_write(new_gd);  /*Call the Write CS*/
  rcu_assign_pointer(gdata, new_gd);
  call_rcu(&old_gd->rcu, gd_reclaim);
  spin_unlock(&handler->spin);
  // synchronize_rcu();
  // kfree(old_gd);
  return 0;
}

void gd_reclaim(struct rcu_head *r)
{
  // struct cs_handler *fp = container_of(r, struct cs_handler, rcu);
  struct data *fp = container_of(r, struct data, rcu);
  kfree(fp);
}

int rculock_read_data(struct data *gd, char *buf)
{
  struct cs_handler *handler = gd->handler;
  struct data *p;
  BUG_ON(!handler->mustcall_write);

  rcu_read_lock();
  p  =  rcu_dereference(gdata);
  handler->mustcall_read(p, buf);  /*Call the read CS*/
  rcu_read_unlock();
  return 0;
}

/* custom lock implementation*/

int customlock_init_cs(struct data *gd)
{
   struct cs_handler *handler = gd->handler;
   custom_init(&handler->crwlock);
   return 0;
}
int customlock_cleanup_cs(struct data *gd)
{
   return 0;
}

int customlock_write_data(struct data *gd)
{

  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);
  
  custom_wrlock(&handler->crwlock);
  handler->mustcall_write(gd);  /*Call the Write CS*/
  custom_wrunlock(&handler->crwlock);

  return 0;
} 

int customlock_read_data(struct data *gd, char *buf)
{
  struct cs_handler *handler = gd->handler;
  BUG_ON(!handler->mustcall_write);

  custom_rdlock(&handler->crwlock);
  handler->mustcall_read(gd, buf);  /*Call the read CS*/
  custom_rdunlock(&handler->crwlock);
  return 0;
}



/*TODO   Your implementations for assignment II*/ 






        
/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static ssize_t asg2_lock_show(struct kobject *kobj, struct kobj_attribute *attr,
                        char *buf)
{

        if(!gdata || !gdata->handler)
           return -EINVAL;
        return sprintf(buf, "%d\n", gdata->lock_type);
}

static ssize_t asg2_lock_set(struct kobject *kobj,
                                   struct kobj_attribute *attr,
                                   const char *buf, size_t count)
{
        int err;
        unsigned long mode;


        if(atomic_read(&open_count)) /*Change only when no process has opened the file*/
          return -EINVAL;
        
        if(!gdata || !gdata->handler)
             return -EINVAL;

        err = kstrtoul(buf, 10, &mode);
        if (err)
                return -EINVAL;

        if(set_cs_implementation(gdata->handler, mode))
            return -EINVAL;
         
        gdata->lock_type = mode;
        return count;
}

static struct kobj_attribute lock_type_attribute = __ATTR(asg2_lock, 0644, asg2_lock_show, asg2_lock_set);

static struct attribute *lt_attrs[] = {
                    &lock_type_attribute.attr,
                    NULL,
};
static struct attribute_group lt_attr_group = {
        .attrs = lt_attrs,
};

/* 
   * Called when a process tries to open the device file, like
   * "cat /dev/mycharfile"
   */
static int device_open(struct inode *inode, struct file *file)
{

        if(atomic_read(&open_count))
            return -EBUSY;
	
	atomic_inc(&open_count);
        
        
        BUG_ON(!gdata || !gdata->handler || !gdata->handler->init_cs);
        gdata->num_writes = 0;
        gdata->counter = 0;
        gdata->result = 0;
        
        gdata->handler->init_cs(gdata);  


	try_module_get(THIS_MODULE);
	return 0;
}

/* 
   * Called when a process closes the device file.
   */
static int device_release(struct inode *inode, struct file *file)
{
        

        BUG_ON(!gdata || !gdata->handler || !gdata->handler->cleanup_cs);
        gdata->handler->cleanup_cs(gdata);  
        atomic_dec(&open_count);
	module_put(THIS_MODULE);
	return 0;
}

/* 
   * Called when a process, which already opened the dev file, attempts to
   * read from it.
   */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
		char *buffer,	/* buffer to fill with data (vfs_records)*/
		size_t length,	/* in bytes*/
		loff_t * offset)
{
        int retval;
      
        BUG_ON(!gdata || !gdata->handler || !gdata->handler->read_data);
        retval = gdata->handler->read_data(gdata, buffer);  
    
        return retval;
}

/*  
    * Called when a process writes to dev file
     Three elements of gdata structure should be updated such that
     consistency among the values maintained.
    */
static ssize_t
device_write(struct file *file, const char *buff, size_t len, loff_t * off)
{
        BUG_ON(!gdata || !gdata->handler || !gdata->handler->write_data);
        gdata->handler->write_data(gdata);  
        
	return 0;
}
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	
        if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

        printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	//printk(KERN_INFO "Assigned major number: %d. Now create the dev file: \"/dev/%s\"\n", Major,DEVICE_NAME);
        
        gdata = kzalloc(sizeof(struct data), GFP_KERNEL);
        BUG_ON(!gdata);

       
        atomic_set(&open_count, 0);

        if(sysfs_create_group(kernel_kobj, &lt_attr_group))
                    printk(KERN_INFO "can't create sysfs\n");

        BUG_ON(init_cs_handler(gdata));
	return 0;
}

  /*
   * This function is called when the module is unloaded
   */
void cleanup_module(void)
{
        free_cs_handler(gdata); 
        kfree(gdata);       
        sysfs_remove_group(kernel_kobj, &lt_attr_group);
	unregister_chrdev(Major, DEVICE_NAME);
}

MODULE_LICENSE("GPL");
