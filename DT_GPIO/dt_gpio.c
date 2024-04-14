#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h> 
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DRIVER_AUTHOR       "Nguyen Quang Anh"
#define DRIVER_DESC         "module gpio on off led by device file using DT"

#define LOW     0
#define HIGH    1

/*****************************************************************************************************
************************************ _PROTOTYPE_FUNCTION_ ********************************************
*****************************************************************************************************/
static int device_open(struct inode *device_file, struct file *instance);
static int device_close(struct inode *device_file, struct file *instance);
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs);


static struct gpio_desc *gpio0_30 = NULL;

static struct file_operations fops = {
	.owner      = THIS_MODULE,
	.open       = device_open,
    .write      = device_write,
	.release    = device_close
};

// Struct use to register device file after match with DT
struct miscdevice led = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "led",
    .fops   = &fops,
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

static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) 
{
    char value = 0;

    printk("write was called!\n");
    if(copy_from_user(&value, user_buffer, 1) != 0)
    {
        return -EFAULT;
    }
    else
    {
        printk("copy successfully\n");
    }

    if(value == '1')
    {
        printk("LED ON\n");
        gpiod_set_value(gpio0_30, HIGH);
    }
    if(value == '0')
    {
        printk("LED OFF\n");
        gpiod_set_value(gpio0_30, LOW);
    }
    return 1;
}

static int my_pdrv_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct device *dev = &pdev->dev;

    // Config LED ON after math with DT
    gpio0_30 = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    gpiod_set_value(gpio0_30, HIGH);

    // create character device file 
    ret = misc_register(&led);
    if(ret)
    {
        pr_err("can't misc_register\n");
        return -1;
    }
    // log announce probe successfully!!
    pr_info("%s - %d", __func__, __LINE__);
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    // Config LED OFF
    gpiod_set_value(gpio0_30, LOW);
    gpiod_put(gpio0_30);

    // free character device file
    misc_deregister(&led);

    // log announce remove successfully!!
    pr_info("%s - %d", __func__, __LINE__);
    return 0;
}


static const struct of_device_id gpio_dt_ids[] = {
    { .compatible = "gpio-on-of-led", },
    { /* sentinel */ },
};

static struct platform_driver mypdrv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "on-off-led",
        .of_match_table = gpio_dt_ids,
    }
};

// register platform_driver
module_platform_driver(mypdrv);
// register match with Device tree
MODULE_DEVICE_TABLE(of, gpio_dt_ids);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);