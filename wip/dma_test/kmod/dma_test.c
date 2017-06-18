#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/slab.h>

#define DMA_TEST_BUF_SIZE 16
#define DMA_TEST_PATTERN_1 0xAA
#define DMA_TEST_PATTERN_2 0x3C

#define DRV_NAME "dma_test"

struct dma_test_data {
	struct device *dev;
	struct dma_chan *dma_ch;
	dma_addr_t buf_src_dma, buf_dst_dma;
};

static void dma_test_tx_callback(void *data)
{
	struct dma_test_data *dmatest = data;
	dev_dbg(dmatest->dev, "DMA transfer completed");

	dma_unmap_single(dmatest->dev, dmatest->buf_dst_dma, DMA_TEST_BUF_SIZE, DMA_TO_DEVICE);
	dma_unmap_single(dmatest->dev, dmatest->buf_src_dma, DMA_TEST_BUF_SIZE, DMA_BIDIRECTIONAL);
}

int dma_test_probe(struct platform_device *pdev)
{
	struct dma_test_data *dmatest;
	char *buf_src, *buf_dst;
	int i;
	struct dma_slave_config	cfg;
	struct dma_async_tx_descriptor *tx;
	int ret;
	struct device *chan_dev;
	struct dma_device *dma_dev;

	dmatest = devm_kzalloc(&pdev->dev, sizeof(*dmatest), GFP_KERNEL);
	if (!dmatest)
		return -ENOMEM;

	dmatest->dev = &pdev->dev;

	dev_dbg(dmatest->dev, "Start probe");

	dev_set_drvdata(&pdev->dev, dmatest);

	/* Alloc buffers */

	buf_src = devm_kzalloc(dmatest->dev, DMA_TEST_BUF_SIZE, GFP_KERNEL);
	if (!buf_src)
		return -ENOMEM;
	buf_dst = devm_kzalloc(dmatest->dev, DMA_TEST_BUF_SIZE, GFP_KERNEL);
	if (!buf_dst)
		return -ENOMEM;

	/* Fill source buffer */

	for (i = 0; i < DMA_TEST_BUF_SIZE; i++)
		buf_src[i] = (i % 2) ? DMA_TEST_PATTERN_1 : DMA_TEST_PATTERN_2;

	/* Dmaengine config */
	dev_dbg(dmatest->dev, "Configure dma xfer");

	dmatest->dma_ch = dma_request_chan(dmatest->dev, "test_chan");
	if (IS_ERR(dmatest->dma_ch)) {
		ret = PTR_ERR(dmatest->dma_ch);
		dev_err(dmatest->dev, "Dma request channel failed");
		goto err;
	}

	memset(&cfg, 0, sizeof(cfg));

	cfg.src_addr = dmatest->buf_src_dma;
	cfg.dst_addr = dmatest->buf_dst_dma;
	cfg.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	cfg.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	cfg.src_maxburst = 16;
	cfg.dst_maxburst = 16;

	ret = dmaengine_slave_config(dmatest->dma_ch, &cfg);
	if (ret) {
		dev_err(dmatest->dev, "Dmaengine slave config failed");
		goto err_chan;
	}

	/* Now real transfer */
	dev_dbg(dmatest->dev, "Perform transfer");

	chan_dev = dmatest->dma_ch->device->dev;
	dma_dev = dmatest->dma_ch->device;

	dev_dbg(dmatest->dev, "Map src buf");
	dmatest->buf_src_dma = dma_map_single(chan_dev, buf_src, DMA_TEST_BUF_SIZE, DMA_TO_DEVICE);
	ret = dma_mapping_error(dmatest->dev, dmatest->buf_src_dma);
	if (ret) {
		dev_err(dmatest->dev, "Source buffer mapping failed");
		goto err_map_src;
	}

	dev_dbg(dmatest->dev, "Map dst buf");
	dmatest->buf_dst_dma = dma_map_single(chan_dev, buf_dst, DMA_TEST_BUF_SIZE, DMA_BIDIRECTIONAL);
	ret = dma_mapping_error(dmatest->dev, dmatest->buf_dst_dma);
	if (ret) {
		dev_err(dmatest->dev, "Destination buffer mapping failed");
		goto err_map_dst;
	}


	tx = dma_dev->device_prep_dma_memcpy(dmatest->dma_ch,
			dmatest->buf_dst_dma, dmatest->buf_src_dma,
			DMA_TEST_BUF_SIZE, DMA_PREP_INTERRUPT);

	if (!tx) {
		dev_err(dmatest->dev, "Not able to get desc for DMA xfer\n");
		goto err_desc;
	}


	tx->callback = dma_test_tx_callback;
	tx->callback_param = dmatest;

	dev_dbg(dmatest->dev, "Submit xfer to pending queue");

	if (dma_submit_error(dmaengine_submit(tx))) {
		dev_err(dmatest->dev, "DMA submit failed!");
		goto err_submit;
	}

	dev_dbg(dmatest->dev, "Issue pending transfer");

	dma_async_issue_pending(dmatest->dma_ch);

	dev_dbg(dmatest->dev, "Probe complete");

	return 0;

err_submit:
	dmaengine_terminate_all(dmatest->dma_ch);
err_desc:
	dma_unmap_single(dmatest->dev, dmatest->buf_dst_dma, DMA_TEST_BUF_SIZE, DMA_TO_DEVICE);
err_map_dst:
	dma_unmap_single(dmatest->dev, dmatest->buf_src_dma, DMA_TEST_BUF_SIZE, DMA_FROM_DEVICE);
err_map_src:
err_chan:
	dma_release_channel(dmatest->dma_ch);
err:
	return ret;
}

int dma_test_remove(struct platform_device *pdev)
{
	struct dma_test_data *dmatest = platform_get_drvdata(pdev);
	dev_dbg(dmatest->dev, "Remove complete");
	return 0;
}


static const struct of_device_id dma_test_ids[] = {
	{ .compatible = "dma-test", },
	{},
};
MODULE_DEVICE_TABLE(of, dma_test_ids);

static struct platform_driver dma_test_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = dma_test_ids,
	},
	.probe = dma_test_probe,
	.remove = dma_test_remove,
};

module_platform_driver(dma_test_driver);

MODULE_DESCRIPTION("DMA test driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
