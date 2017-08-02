#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pru_dma.h>

#define DRV_NAME "pru_dma_hcsr04"

struct pru_dma_hcsr04 {
	struct device *dev;
	struct pru_dma *pru_dma;
};

static ssize_t pru_dma_hcsr04_tx_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct pru_dma_hcsr04 *pru_dma_hcsr04 =
		platform_get_drvdata(to_platform_device(dev));

	ret = pru_dma_tx_trigger(pru_dma_hcsr04->pru_dma);
	if (ret)
		return ret;

	dev_err(pru_dma_hcsr04->dev, "Tx triggered.\n");

	return count;
}

static DEVICE_ATTR(tx, S_IWUSR, NULL, pru_dma_hcsr04_tx_store);

static ssize_t pru_dma_hcsr04_buf_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	uint32_t *dma_buf;
	uint32_t dma_buf_size;
	int i;
	int len = 0;
	uint32_t mean = 0;

	struct pru_dma_hcsr04 *pru_dma_hcsr04 =
		platform_get_drvdata(to_platform_device(dev));

	pru_dma_tx_completion_wait(pru_dma_hcsr04->pru_dma);

	dma_buf = pru_dma_get_buffer(pru_dma_hcsr04->pru_dma);
	dma_buf_size = pru_dma_get_buffer_size(pru_dma_hcsr04->pru_dma);

	dev_err(pru_dma_hcsr04->dev, "Buffer obtained.\n");

	for (i = 0; i < dma_buf_size; i++)
		mean += *(dma_buf + i);

	mean *= 1000;
	mean /= (291 * 2 * dma_buf_size);

	len = sprintf(buf, "%umm\n", mean);

	return len;
}

static DEVICE_ATTR(buf, S_IRUGO, pru_dma_hcsr04_buf_show, NULL);

static struct attribute *pru_dma_hcsr04_attributes[] = {
	&dev_attr_tx.attr,
	&dev_attr_buf.attr,
	NULL,
};

static const struct attribute_group pru_dma_hcsr04_group = {
		.attrs = pru_dma_hcsr04_attributes,
};

int pru_dma_hcsr04_probe(struct platform_device *pdev)
{
	struct pru_dma_hcsr04 *pru_dma_hcsr04;
	int ret;

	pru_dma_hcsr04 = devm_kzalloc(&pdev->dev, sizeof(*pru_dma_hcsr04), GFP_KERNEL);
	if (!pru_dma_hcsr04)
		return -ENOMEM;

	pru_dma_hcsr04->dev = &pdev->dev;

	dev_set_drvdata(&pdev->dev, pru_dma_hcsr04);
	pru_dma_hcsr04->pru_dma = pru_dma_get("pru-dma");

	if (IS_ERR(pru_dma_hcsr04->pru_dma)) {
		ret = PTR_ERR(pru_dma_hcsr04->pru_dma);
		if (ret != -EPROBE_DEFER)
			dev_err(pru_dma_hcsr04->dev, "Unable to get pru_dma handle.\n");
		return ret;
	}

	dev_err(pru_dma_hcsr04->dev, "Got PRU DMA handle!.\n");

	ret = pru_dma_map_buffer(pru_dma_hcsr04->pru_dma);
	if (ret) {
		dev_err(pru_dma_hcsr04->dev, "Ret = %d!.\n", ret);
		return ret;
	}

	dev_err(pru_dma_hcsr04->dev, "Mapped buffer!.\n");

	ret = sysfs_create_group(&pdev->dev.kobj, &pru_dma_hcsr04_group);
	if (ret) {
		dev_err(&pdev->dev, "sysfs_create_group() failed (%d)\n",
				ret);
		return ret;
	}

	dev_dbg(pru_dma_hcsr04->dev, "Probe success");
	return 0;
}

int pru_dma_hcsr04_remove(struct platform_device *pdev)
{
	struct pru_dma_hcsr04 *pru_dma_hcsr04 = dev_get_drvdata(&pdev->dev);

	pru_dma_unmap_buffer(pru_dma_hcsr04->pru_dma);

	sysfs_remove_group(&pdev->dev.kobj, &pru_dma_hcsr04_group);

	dev_err(pru_dma_hcsr04->dev, "Unmapped buffer.\n");
	return 0;
}

static const struct of_device_id pru_dma_hcsr04_ids[] = {
	{ .compatible = "pru-dma-hcsr04", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_dma_hcsr04_ids);

static struct platform_driver pru_dma_hcsr04_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_dma_hcsr04_ids,
	},
	.probe = pru_dma_hcsr04_probe,
	.remove = pru_dma_hcsr04_remove,
};

module_platform_driver(pru_dma_hcsr04_driver);

MODULE_DESCRIPTION("PRU DMA LEDs driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
