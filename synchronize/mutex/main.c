#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/err.h>

struct mutex etx_mutex;
unsigned long global_variable = 0;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static struct task_struct *etx_thread1;
static struct task_struct *etx_thread2; 


int thread_function1(void *pv);
int thread_function2(void *pv);

/******************************************************/

int thread_function1(void *pv)
{
    while(!kthread_should_stop()) 
    {
        mutex_lock(&etx_mutex);
        global_variable++;
        pr_info("In Thread Function1 %lu\n", global_variable);
        mutex_unlock(&etx_mutex);
        msleep(1000);
    }
    return 0;
}

int thread_function2(void *pv)
{
    while(!kthread_should_stop()) {
        mutex_lock(&etx_mutex);
        global_variable++;
        pr_info("In Thread Function2 %lu\n", global_variable);
        mutex_unlock(&etx_mutex);
        msleep(1000);
    }
    return 0;
}

static int __init etx_driver_init(void)
{
    mutex_init(&etx_mutex);
    /* Creating Thread 1 */
    etx_thread1 = kthread_run(thread_function1,NULL,"Thread1");
    if(etx_thread1) 
    {
        pr_err("Kthread1 Created Successfully...\n");
    } 
    else 
    {
        pr_err("Cannot create kthread1\n");
    }
    /* Creating Thread 2 */
    etx_thread2 = kthread_run(thread_function2,NULL,"Thread2");
    if(etx_thread2) 
    {
        pr_err("Kthread2 Created Successfully...\n");
    } 
    else 
    {
        pr_err("Cannot create kthread2\n");
    }  

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
}

static void __exit etx_driver_exit(void)
{
    kthread_stop(etx_thread1);
    kthread_stop(etx_thread2);
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen_Quang_Anh");
MODULE_DESCRIPTION("A simple device driver - Mutex");