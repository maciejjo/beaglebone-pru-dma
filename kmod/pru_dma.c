#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pruss.h>
#include <linux/pinctrl/consumer.h>

#define DRV_NAME "pru_dma"

struct pru_dma_data {
	struct device *dev;
};

int pru_dma_probe(struct platform_device *pdev)
{
	struct pru_dma_data *pru_dma;
	int ret;

	pru_dma = devm_kzalloc(&pdev->dev, sizeof(*pru_dma), GFP_KERNEL);
	if (!pru_dma)
		return -ENOMEM;

	pru_dma->dev = &pdev->dev;

	dev_set_drvdata(&pdev->dev, pru_dma);

	dev_dbg(pru_dma->dev, "Probe success");

	return 0;
}

int pru_dma_remove(struct platform_device *pdev)
{
	return 0;
}


static const struct of_device_id pru_dma_ids[] = {
	{ .compatible = "pru-dma", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_dma_ids);

static struct platform_driver pru_dma_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_dma_ids,
	},
	.probe = pru_dma_probe,
	.remove = pru_dma_remove,
};

module_platform_driver(pru_dma_driver);

MODULE_DESCRIPTION("PRU DMA driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
