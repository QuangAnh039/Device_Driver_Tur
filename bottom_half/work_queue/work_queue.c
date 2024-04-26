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
#include <linux/interrupt.h>
#include <linux/workqueue.h> 

#define DRIVER_AUTHOR       "Nguyen Quang Anh"
#define DRIVER_DESC         "module gpio on off led by device file using DT"

#define LOW     0
#define HIGH    1

/*****************************************************************************************************
 *************************** _PROTOTYPE_FUNCTION_ ****************************************************
 *****************************************************************************************************
*/
static int device_open(struct inode *device_file, struct file *instance);
static int device_close(struct inode *device_file, struct file *instance);
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs);
void workqueue_fn(struct work_struct *work); 


static struct gpio_desc *gpio0_30 = NULL;
static struct gpio_desc *gpio0_31 = NULL;
int irq = 0;

DECLARE_WORK(workqueue,workqueue_fn);

void workqueue_fn(struct work_struct *work)
{
    printk(KERN_INFO "Executing Workqueue Function\n");
    int gpio_value;
    gpio_value = gpiod_get_value(gpio0_30);
    if(gpio_value == 1)
    {
        gpiod_set_value(gpio0_30, LOW);
    }
    if(gpio_value == 0)
    {
        gpiod_set_value(gpio0_30, HIGH);
    }
}

static struct file_operations fops = {
	.owner      = THIS_MODULE,
	.open       = device_open,
    .write      = device_write,
	.release    = device_close
};

struct miscdevice led = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "led",
    .fops   = &fops,
};

static irqreturn_t gpio_intterupt_handler(int irq, void *dev_id) 
{
    printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
    schedule_work(&workqueue);
    return IRQ_HANDLED;
}
static int device_open(struct inode *device_file, struct file *instance) {
	printk("open was called!\n");
	return 0;
}

static int device_close(struct inode *device_file, struct file *instance) {
	printk("close was called!\n");
	return 0;
}

static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
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
    
    //Init LED
    struct device *dev = &pdev->dev;
    gpio0_30 = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    gpiod_set_value(gpio0_30, HIGH);

    //Init Button
    gpio0_31 = gpiod_get(dev, "wakeup", GPIOD_IN);
    irq = gpiod_to_irq(gpio0_31);
    if (irq < 0) 
    {
        printk(KERN_ERR "Failed to get IRQ for platform device\n");
    }
    else
    {
        printk(KERN_INFO "IRQ for platform device: %d\n", irq);
    }

    if(request_irq(irq, (void *)gpio_intterupt_handler, IRQF_TRIGGER_FALLING, "button", NULL))
    {
        pr_err("Cannot register interrupt number: %d\n", irq);
        return -1;
    }

    //create device file
    ret = misc_register(&led);
    if(ret)
    {
        pr_err("can't misc_register\n");
        return -1;
    }
    pr_info("%s - %d", __func__, __LINE__);
    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    gpiod_set_value(gpio0_30, LOW);
    gpiod_put(gpio0_30);
    free_irq(irq, NULL);
    gpiod_put(gpio0_31);
    misc_deregister(&led);
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

module_platform_driver(mypdrv);
MODULE_DEVICE_TABLE(of, gpio_dt_ids);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);