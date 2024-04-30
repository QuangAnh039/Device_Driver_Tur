#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

static int serial_user_probe(struct serdev_device *pdev);
static void serial_user_remove(struct serdev_device *pdev);

static ssize_t serial_user_write(struct file *file, const char __user *buf, size_t len, loff_t *pos);
static ssize_t serial_user_read(struct file *file, char __user *buf, size_t len, loff_t *pos);
static int serial_user_open(struct inode *inode, struct file *file);
static int serial_user_close(struct inode *inodep, struct file *filp);

/* Matching table */
static const struct of_device_id serial_user_BBB_of_match[] = {
	{ .compatible = "serial-user-demo"},
    {}
};

static const struct file_operations serial_user_fops = 
{
    .owner			= THIS_MODULE,
    .write			= serial_user_write,
	.read			= serial_user_read,
    .open			= serial_user_open,
    .release		= serial_user_close
};

struct miscdevice serial_user_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "serial_user_dev",
    .fops = &serial_user_fops,
};

static ssize_t serial_user_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    // char * dynamic_ptr = NULL;
    // int int_value = 0;
    // int ret = -1;

    pr_info("%d %s\n", __LINE__, __func__);

    return 1;
}

static ssize_t serial_user_read(struct file *file, char __user *buf, size_t len, loff_t *pos)
{
    pr_info("%d %s\n", __LINE__, __func__);

    return 1;
}

static int serial_user_open(struct inode *inode, struct file *file)
{
    pr_info("%d %s\n", __LINE__, __func__);

    return 0;
}

static int serial_user_close(struct inode *inodep, struct file *filp)
{
    pr_info("%d %s\n", __LINE__, __func__);

    return 0;
}

/**
 * @brief Callback is called whenever a character is received
 */
static int serdev_echo_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size) 
{
	printk("serdev_echo - Received %d bytes with %s\n", size, *buffer);

    return 0;
}

static const struct serdev_device_ops serdev_echo_ops = {
	.receive_buf = serdev_echo_recv,
};

static struct serdev_device_driver serial_user_driver = 
{
	.probe		= serial_user_probe,
    .remove     = serial_user_remove,
	.driver		= {
		.name	= "serial_user",
		.of_match_table = serial_user_BBB_of_match,
	},
};

static int serial_user_probe(struct serdev_device *pdev)
{
    int ret = -1;
    int status = 0;

    pr_info("probe is called\n");

    serdev_device_set_client_ops(pdev, &serdev_echo_ops);
    status = serdev_device_open(pdev);
	if(status) {
		printk("serdev_echo - Error opening serial port!\n");
		return -status;
	}

    serdev_device_set_baudrate(pdev, 9600);
	serdev_device_set_flow_control(pdev, false);
	serdev_device_set_parity(pdev, SERDEV_PARITY_NONE);

    status = serdev_device_write_buf(pdev, "Type something: ", sizeof("Type something: "));
    pr_info("Byte wrote : %d\n", status);

    /* Create misc module */
    ret = misc_register(&serial_user_device);
    if (ret) {
        pr_err("can't misc_register\n");
        return -1;
    }

    return 0;
}

static void serial_user_remove(struct serdev_device  *pdev)
{
    //int ret = -1;

    pr_info("remove serial module\n");

    /* deregister pwm device */
    misc_deregister(&serial_user_device);

    //return 0;
}

module_serdev_device_driver(serial_user_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Serial Led kernel module");