#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/proc_fs.h>
#include <linux/cdev.h>
dev_t dev = 0; // dev_t hold major n minor number 
static struct class *dev_class;
static struct cdev dev_cdev;

/* Declate the probe and remove functions */
static int dt_probe(struct platform_device *pdev);
static int dt_remove(struct platform_device *pdev);
static int device_open(struct inode *device_file, struct file *instance); 
static int device_close(struct inode *device_file, struct file *instance);
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t device_read(struct file *filp, char __user *user_buffer, size_t count, loff_t *offs);

static struct file_operations fops = {
        .owner      = THIS_MODULE,
	.open       = device_open,
        .write      = device_write,
        .read       = device_read,
	.release    = device_close
};
static int device_open(struct inode *device_file, struct file *instance) 
{
	printk("open was called!\n");
	return 0;
}
static int device_close(struct inode *device_file, struct file *instance) 
{
	printk("close was called!\n");
	return 0;
}
static ssize_t device_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
        printk("read was called!\n");
        return 0;
}
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) 
{
        printk("write was called!\n");
	return count;
}
static struct of_device_id my_driver_ids[] = {
	{
	        .compatible = "quanganhnguyen1",
	},
        {
	        .compatible = "quanganhnguyen2",
	},  
        { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct platform_driver my_driver = {
	.probe = dt_probe,
	.remove = dt_remove,
	.driver = {
		.name = "my_device_driver",
		.of_match_table = my_driver_ids,
	},
};
static int dt_probe(struct platform_device *pdev) {
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 2, "quanganh")) <0) //  appear in /proc/devices/quanganh
        {
            printk(KERN_INFO "Cannot allocate major number for device 1\n");
            return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
        /*Creating struct class*/
        dev_class = class_create(THIS_MODULE,"quanganh_class");   // It will create a structure under/sys/class/quanganh_class
        if(IS_ERR(dev_class)){
            pr_err("Cannot create the struct class for device\n");
            goto r_class;
        }
        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"quanganh_device"))) // create /dev/quanganh_device
        {
            pr_err("Cannot create the Device\n");
            goto r_device;
        }

        /*Creating cdev structure*/
        cdev_init(&dev_cdev, &fops);

        /*Adding character device to the system*/
        if((cdev_add(&dev_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto rm_device;
        }
        printk(KERN_INFO "Kernel Module Inserted Successfully...\n");

        return 0;
rm_device: 
        device_destroy(dev_class,dev);
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

static int dt_remove(struct platform_device *pdev) {
        cdev_del(&dev_cdev);   
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        unregister_chrdev_region(dev, 1);
        pr_info("Kernel Module Removed Successfully...\n");
        return 0;
}

module_platform_driver(my_driver);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Quang Anh");
MODULE_DESCRIPTION("Simple linux driver (Automatically Creating a Device file)");
MODULE_VERSION("1.2");