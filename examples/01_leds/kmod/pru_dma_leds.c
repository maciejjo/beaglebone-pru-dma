#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pru_dma.h>

#define DRV_NAME "pru_dma_leds"

struct pru_dma_leds {
	struct device *dev;
	struct pru_dma *pru_dma;
};

static void leds_pattern_generate(uint32_t *buf, uint32_t size)
{
	int i;

	for (i = 0; i < size; i++)
		buf[i] =
			1 << (i % 8);
}

int pru_dma_leds_probe(struct platform_device *pdev)
{
	struct pru_dma_leds *pru_dma_leds;
	int ret;
	uint32_t *buf;
	uint32_t buf_size;

	pru_dma_leds = devm_kzalloc(&pdev->dev, sizeof(*pru_dma_leds), GFP_KERNEL);
	if (!pru_dma_leds)
		return -ENOMEM;

	pru_dma_leds->dev = &pdev->dev;

	dev_set_drvdata(&pdev->dev, pru_dma_leds);
	pru_dma_leds->pru_dma = pru_dma_get("pru-dma");

	if (IS_ERR(pru_dma_leds->pru_dma)) {
		ret = PTR_ERR(pru_dma_leds->pru_dma);
		if (ret != -EPROBE_DEFER)
			dev_err(pru_dma_leds->dev, "Unable to get pru_dma handle.\n");
		return ret;
	}

	dev_err(pru_dma_leds->dev, "Got PRU DMA handle!.\n");

	buf = pru_dma_get_buffer(pru_dma_leds->pru_dma);
	dev_err(pru_dma_leds->dev, "Got buffer @ 0x%.8x!.\n", buf);
	buf_size = pru_dma_get_buffer_size(pru_dma_leds->pru_dma);
	dev_err(pru_dma_leds->dev, "Got buffer size - 0x%.8x!.\n", buf_size);

	leds_pattern_generate(buf, buf_size);

	ret = pru_dma_map_buffer(pru_dma_leds->pru_dma);
	if (ret) {
		dev_err(pru_dma_leds->dev, "Ret = %d!.\n", ret);
		return ret;
	}


	dev_err(pru_dma_leds->dev, "Mapped buffer!.\n");

	ret = pru_dma_tx_trigger(pru_dma_leds->pru_dma);
	if (ret) {
		pru_dma_unmap_buffer(pru_dma_leds->pru_dma);
		return ret;
	}

	dev_err(pru_dma_leds->dev, "Tx triggered.\n");


	dev_dbg(pru_dma_leds->dev, "Probe success");
	return 0;
}

int pru_dma_leds_remove(struct platform_device *pdev)
{
	struct pru_dma_leds *pru_dma_leds = dev_get_drvdata(&pdev->dev);

	pru_dma_unmap_buffer(pru_dma_leds->pru_dma);

	dev_err(pru_dma_leds->dev, "Unmapped buffer.\n");
	return 0;
}

static const struct of_device_id pru_dma_leds_ids[] = {
	{ .compatible = "pru-dma-leds", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_dma_leds_ids);

static struct platform_driver pru_dma_leds_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_dma_leds_ids,
	},
	.probe = pru_dma_leds_probe,
	.remove = pru_dma_leds_remove,
};

module_platform_driver(pru_dma_leds_driver);

MODULE_DESCRIPTION("PRU DMA LEDs driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
