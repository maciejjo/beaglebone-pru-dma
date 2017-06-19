#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/slab.h>
#include <linux/pruss.h>

#define DMA_TEST_BUF_SIZE 16
#define DMA_TEST_PATTERN_1 0xAA
#define DMA_TEST_PATTERN_2 0x3C

#define DRV_NAME "pru_dma_test"

struct pru_dma_test_data {
	struct device *dev;
	struct dma_chan *dma_ch;
	struct pruss *pruss;
	struct pruss_mem_region pru0_memory;
	uint8_t *buf_src;
	dma_addr_t buf_src_dma;
};

static void pru_dma_test_tx_callback(void *data)
{
	struct pru_dma_test_data *dmatest = data;
	dev_err(dmatest->dev, "DMA transfer completed");

	dma_unmap_single(dmatest->dev, dmatest->buf_src_dma, DMA_TEST_BUF_SIZE, DMA_TO_DEVICE);
}

int pru_dma_test_probe(struct platform_device *pdev)
{
	struct pru_dma_test_data *dmatest;
	int i;
	struct dma_slave_config	dma_sconfig;
	struct dma_async_tx_descriptor *tx;
	int ret;
	struct device *chan_dev;
	dma_cookie_t dma_cookie;

	dmatest = devm_kzalloc(&pdev->dev, sizeof(*dmatest), GFP_KERNEL);
	if (!dmatest)
		return -ENOMEM;

	dmatest->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, dmatest);

	dev_dbg(dmatest->dev, "Start probe");

	/* Grab PRU handle */

	dmatest->pruss = pruss_get(dmatest->dev);
	if (IS_ERR(dmatest->pruss)) {
		ret = PTR_ERR(dmatest->pruss);
		dev_err(dmatest->dev, "Cannot get PRUSS handle");
		goto err_pruss_get;
	}

	/* Map pru0 memory */
	ret = pruss_request_mem_region(dmatest->pruss, PRUSS_MEM_DRAM0,
		&dmatest->pru0_memory);
	if (ret) {
		dev_err(dmatest->dev, "Unable to get PRUSS RAM.\n");
		goto err_pruss_mem;
	}

	/* Alloc buffers */

	dmatest->buf_src = devm_kzalloc(dmatest->dev, DMA_TEST_BUF_SIZE, GFP_KERNEL);
	if (!dmatest->buf_src)
		goto err_pruss_mem;

	/* Fill source buffer */

	for (i = 0; i < DMA_TEST_BUF_SIZE; i++)
		dmatest->buf_src[i] = (i % 2) ? DMA_TEST_PATTERN_1 : DMA_TEST_PATTERN_2;

	/* Dmaengine config */
	dev_dbg(dmatest->dev, "Configure dma xfer");

	dmatest->dma_ch = dma_request_chan(dmatest->dev, "pru_test_chan");
	if (IS_ERR(dmatest->dma_ch)) {
		ret = PTR_ERR(dmatest->dma_ch);
		dev_err(dmatest->dev, "Dma request channel failed");
		goto err_pruss_mem;
	}

	memset(&dma_sconfig, 0, sizeof(dma_sconfig));

	dma_sconfig.dst_addr = (dma_addr_t) dmatest->pru0_memory.pa;
	dma_sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	dma_sconfig.dst_maxburst = 1;
	dma_sconfig.direction = DMA_MEM_TO_DEV;

	ret = dmaengine_slave_config(dmatest->dma_ch, &dma_sconfig);
	if (ret) {
		dev_err(dmatest->dev, "Dmaengine slave config failed");
		goto err_chan;
	}

	chan_dev = dmatest->dma_ch->device->dev;

	dev_dbg(dmatest->dev, "Map src buf");

	dmatest->buf_src_dma = dma_map_single(chan_dev, dmatest->buf_src, DMA_TEST_BUF_SIZE, DMA_TO_DEVICE);
	ret = dma_mapping_error(dmatest->dev, dmatest->buf_src_dma);
	if (ret) {
		dev_err(dmatest->dev, "Source buffer mapping failed");
		goto err_map_src;
	}
	dev_dbg(dmatest->dev, "Perform transfer from 0x%.8x to 0x%.8x", dmatest->buf_src_dma, dmatest->pru0_memory.pa);


	tx = dmaengine_prep_slave_single(dmatest->dma_ch,
			dmatest->buf_src_dma, DMA_TEST_BUF_SIZE,
			DMA_MEM_TO_DEV,
			DMA_PREP_INTERRUPT | DMA_CTRL_ACK);

	if (!tx) {
		dev_err(dmatest->dev, "Not able to get desc for DMA xfer\n");
		goto err_desc;
	}


	tx->callback = pru_dma_test_tx_callback;
	tx->callback_param = dmatest;

	dev_dbg(dmatest->dev, "Submit xfer to pending queue");

	dma_cookie = dmaengine_submit(tx);
	if (dma_submit_error(dma_cookie)) {
		dev_err(dmatest->dev, "DMA submit failed!");
		goto err_submit;
	}

	dev_err(dmatest->dev, "Issue pending transfer");

	dma_async_issue_pending(dmatest->dma_ch);
	dma_sync_wait(dmatest->dma_ch, dma_cookie);

	dev_dbg(dmatest->dev, "Probe complete");

	return 0;

err_submit:
	dmaengine_terminate_all(dmatest->dma_ch);
err_desc:
	dma_unmap_single(dmatest->dev, dmatest->buf_src_dma, DMA_TEST_BUF_SIZE, DMA_FROM_DEVICE);
err_map_src:
err_chan:
	dma_release_channel(dmatest->dma_ch);

err_pruss_mem:
	if (dmatest->pru0_memory.va)
		pruss_release_mem_region(dmatest->pruss, &dmatest->pru0_memory);
err_pruss_get:
	return ret;
}

int pru_dma_test_remove(struct platform_device *pdev)
{
	struct pru_dma_test_data *dmatest = platform_get_drvdata(pdev);

	dmaengine_terminate_all(dmatest->dma_ch);
	dma_unmap_single(dmatest->dev, dmatest->buf_src_dma, DMA_TEST_BUF_SIZE, DMA_FROM_DEVICE);
	dma_release_channel(dmatest->dma_ch);
	if (dmatest->pru0_memory.va)
		pruss_release_mem_region(dmatest->pruss, &dmatest->pru0_memory);

	dev_dbg(dmatest->dev, "Remove complete");

	return 0;
}


static const struct of_device_id pru_dma_test_ids[] = {
	{ .compatible = "pru-dma-test", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_dma_test_ids);

static struct platform_driver pru_dma_test_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_dma_test_ids,
	},
	.probe = pru_dma_test_probe,
	.remove = pru_dma_test_remove,
};

module_platform_driver(pru_dma_test_driver);

MODULE_DESCRIPTION("DMA test driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
