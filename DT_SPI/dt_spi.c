#include <linux/module.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Quang Anh");
MODULE_DESCRIPTION("A driver for my simple SPI");

struct spi_device *spi_client;
char buffer = 'A';
uint8_t rx  = 0x00;
struct spi_transfer  tr = 
{
    .tx_buf  = &buffer,
    .rx_buf = &rx,
    .len    = 1,
};
/* Declate the probe and remove functions */
static int my_probe(struct spi_device *client);
static int my_remove(struct spi_device *client);

static struct of_device_id my_driver_ids[] = {
	{
		.compatible = "logicanalyzer,myadc",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct spi_device_id my_adc[] = {
	{"my_adc", 0},
	{ },
};
MODULE_DEVICE_TABLE(spi, my_adc);

static struct spi_driver my_driver = {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = my_adc,
	.driver = {
		.name = "my_adc",
		.of_match_table = my_driver_ids,
	},
};

static int my_probe(struct spi_device *client) {
	int ret;
	
	printk("dt_iio - Now I am in the Probe function!\n");
	spi_client = client;
	ret = spi_setup(spi_client);
	if(ret < 0) {
		printk("dt_iio - Error! Failed to set up the SPI Bus\n");
		return ret;
	}
	spi_sync_transfer(spi_client, &tr, 1 );
	if(ret < 0) {
		printk("dt_iio - Error! Could not power ADC up\n");
		return -1;
	}
	return 0;
}
static int my_remove(struct spi_device *client) {
	printk("dt_iio - Now I am in the Remove function!\n");
	return 0;
}
module_spi_driver(my_driver);