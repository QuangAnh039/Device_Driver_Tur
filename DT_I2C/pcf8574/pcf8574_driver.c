#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>

#define DRIVER_AUTHOR       "Nguyen Quang Anh"
#define DRIVER_DESC         "communicate with pcf8574 by i2c protocal"


static struct i2c_client *pcf8574_client;


/* Declate the probe and remove functions */
static int my_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int my_remove(struct i2c_client *client);
static int device_open(struct inode *device_file, struct file *instance);
static int device_close(struct inode *device_file, struct file *instance);
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs);
static ssize_t device_read(struct file *filp, char __user *buf, size_t len, loff_t *off);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = device_read,
        .write          = device_write,
        .open           = device_open,
        .release        = device_close,
};
// Struct use to register device file after match with DT
struct miscdevice i2c_device = {
    .minor  = MISC_DYNAMIC_MINOR,
    .name   = "pcf8574",
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
static ssize_t device_read(struct file *File, char *user_buffer, size_t count, loff_t *offs)
{	
	//u8 adc;
	printk("read was called!\n");
	return 0;
	// adc = i2c_smbus_read_byte(pcf8574_client);
	// return sprintf(user_buffer, "%d\n", adc);
}
static ssize_t device_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) 
{
	//long val;
	char data1 = 'A';
	printk("write was called!\n");
	// if(0 == kstrtol(user_buffer, 0, &val)) 
	// {
	// 	i2c_smbus_write_byte(pcf8574_client, (u8) val);
	// }
	i2c_master_send(pcf8574_client, &data1, 1);
	return count;
}

static struct of_device_id my_driver_ids[] = {
	{.compatible = "pcf8574", }, 
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct i2c_device_id pcf[] = {
	{"pcf", 0},
	{ },
};
MODULE_DEVICE_TABLE(i2c, pcf);

static struct i2c_driver my_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = pcf,
	.driver = {
		.name = "pcf",
		.of_match_table = my_driver_ids,
	},
};
static int my_probe(struct i2c_client *client, const struct i2c_device_id *id) {
	int ret = 0;
	printk("dt_i2c - Now quanganh in the Probe function! \n");
	if(client->addr != 0x27) 
	{
		printk("dt_i2c - Wrong I2C address!\n");
		return -1;
	}
	pcf8574_client = client;

	// create character device file 
    ret = misc_register(&i2c_device);
    if(ret)
    {
        pr_err("can't misc_register\n");
        return -1;
    }
    return 0;
}
static int my_remove(struct i2c_client *client) {
	printk("dt_i2c - Now I am in the Remove function!\n");
	// free character device file
    misc_deregister(&i2c_device);
	return 0;
}

module_i2c_driver(my_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);