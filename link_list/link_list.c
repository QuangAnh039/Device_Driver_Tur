#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h> 
#include<linux/kobject.h> 
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/workqueue.h>            // Required for workqueues
#include <linux/err.h>

static struct class *dev_class;
static struct cdev etx_cdev;
dev_t dev = 0;
char etx_value;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);

/*Linked List Node*/
struct my_list{
    struct list_head list;     //linux kernel list implementation
    char data;
};
LIST_HEAD(Head_Node);
static int etx_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    struct my_list *temp;
    int count = 0;
    printk(KERN_INFO "Read function\n");
    list_for_each_entry(temp, &Head_Node, list) 
    {
        printk(KERN_INFO "Node %d data = %c\n", count++, temp->data);
    }
    printk(KERN_INFO "Total Nodes = %d\n", count);
    return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    struct my_list *temp_node = NULL;
    if(copy_from_user(&etx_value, buf, 1) !=0)
    {
        return -EFAULT;
    }
    else
    {
        printk("copy successfully\n");
    }
    temp_node = kmalloc(sizeof(struct my_list), GFP_KERNEL);
    temp_node->data = etx_value;
    INIT_LIST_HEAD(&temp_node->list);
    list_add_tail(&temp_node->list, &Head_Node);
    printk(KERN_INFO "Write Function\n");
    return len;
}

static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = etx_read,
    .write          = etx_write,
    .open           = etx_open,
    .release        = etx_release,
};


static int __init etx_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0)
    {
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d n",MAJOR(dev), MINOR(dev));
    /*Creating cdev structure*/
    cdev_init(&etx_cdev,&fops);
    /*Adding character device to the system*/
    if((cdev_add(&etx_cdev,dev,1)) < 0)
    {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }
    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"etx_class")))
    {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device")))
    {
        printk(KERN_INFO "Cannot create the Device \n");
        goto r_device;
    }
    return 0;
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&etx_cdev);
        return -1;
}

static void __exit etx_driver_exit(void)
{
    /* Go through the list and free the memory. */
    struct my_list *cursor, *temp;
    list_for_each_entry_safe(cursor, temp, &Head_Node, list) 
    {
        list_del(&cursor->list);
        kfree(cursor);
    }
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Quang Anh Nguyen");
MODULE_DESCRIPTION("A simple device driver - Kernel Linked List");
MODULE_VERSION("1.13");