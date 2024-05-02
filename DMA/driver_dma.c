#include <linux/module.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple DMA example for copying data from RAM to RAM");

void my_dma_transfer_completed(void *param) 
{
	struct completion *cmp = (struct completion *) param;
	complete(cmp);
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	dma_cap_mask_t mask;
	struct dma_chan *chan;
	struct dma_async_tx_descriptor *chan_desc;
	dma_cookie_t cookie;
	dma_addr_t src_addr, dst_addr;
	u8 *src_buf, *dst_buf;
	struct completion cmp;
	int status;

	printk("my_dma_memcpy - Init\n");

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE | DMA_PRIVATE, mask);
	chan = dma_request_channel(mask, NULL, NULL);
	if(!chan) {
		printk("my_dma_memcpy - Error requesting dma channel\n");
		return -ENODEV;
	}

	src_buf = dma_alloc_coherent(chan->device->dev, 1024, &src_addr, GFP_KERNEL);
	dst_buf = dma_alloc_coherent(chan->device->dev, 1024, &dst_addr, GFP_KERNEL);

	memset(src_buf, 0x12, 1024);
	memset(dst_buf, 0x0, 1024);

	printk("my_dma_memcpy - Before DMA Transfer: src_buf[0] = %x\n", src_buf[0]);
	printk("my_dma_memcpy - Before DMA Transfer: dst_buf[0] = %x\n", dst_buf[0]);

	chan_desc = dmaengine_prep_dma_memcpy(chan, dst_addr, src_addr, 1024, DMA_MEM_TO_MEM);
	if(!chan_desc) {
		printk("my_dma_memcpy - Error requesting dma channel\n");
		status = -1;
		goto free;
	}

	init_completion(&cmp);

	chan_desc->callback = my_dma_transfer_completed;
	chan_desc->callback_param = &cmp;

	cookie = dmaengine_submit(chan_desc);

	/* Fire the DMA transfer */
	dma_async_issue_pending(chan);

	if(wait_for_completion_timeout(&cmp, msecs_to_jiffies(3000)) <= 0) {
		printk("my_dma_memcpy - Timeout!\n");
		status = -1;
	}

	status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
	if(status == DMA_COMPLETE) {
		printk("my_dma_memcpy - DMA transfer has completed!\n");
		status = 0;
		printk("my_dma_memcpy - After DMA Transfer: src_buf[0] = %x\n", src_buf[0]);
		printk("my_dma_memcpy - After DMA Transfer: dst_buf[0] = %x\n", dst_buf[0]);
	} else {
		printk("my_dma_memcpy - Error on DMA transfer\n");
	}

	dmaengine_terminate_all(chan);
free:
	dma_free_coherent(chan->device->dev, 1024, src_buf, src_addr);
	dma_free_coherent(chan->device->dev, 1024, dst_buf, dst_addr);

	dma_release_channel(chan);
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("Goodbye, Kernel\n");
}

module_init(my_init);
module_exit(my_exit);