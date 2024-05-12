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
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/interrupt.h>    
#include <asm/io.h>
#include <linux/err.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

#define DRIVER_AUTHOR       "Nguyen Quang Anh"
#define DRIVER_DESC         "Sending signal to user"

#define LOW     0
#define HIGH    1

#define SIGETX    44
/* Signaling to Application */
static struct task_struct *task = NULL;
static int signum = 0;

/*****************************************************************************************************
 *************************** _PROTOTYPE_FUNCTION_ ****************************************************
 *****************************************************************************************************
*/
static int device_open(struct inode *device_file, struct file *instance);
static int device_close(struct inode *device_file, struct file *instance);
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs);


static struct gpio_desc *gpio0_30 = NULL;
static struct gpio_desc *gpio0_31 = NULL;
int irq = 0;

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
    struct siginfo  info;
    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = 1;
    printk("gpio_irq: Interrupt was triggered and ISR was called!\n");

    if (task != NULL) 
    {
        printk(KERN_INFO "Sending signal to app\n");
        if(send_sig_info(SIGETX, (struct kernel_siginfo *)&info, task) < 0) 
        {
            printk(KERN_INFO "Unable to send signal\n");
        }
    }
    return IRQ_HANDLED;
}
static int device_open(struct inode *device_file, struct file *instance) {
    task = get_current();
    signum = SIGETX;
	printk("open was called!\n");
	return 0;
}

static int device_close(struct inode *device_file, struct file *instance) {
	printk("close was called!\n");
    struct task_struct *ref_task; 
    ref_task = get_current();
    if(ref_task == task) {
        task = NULL;
    }
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