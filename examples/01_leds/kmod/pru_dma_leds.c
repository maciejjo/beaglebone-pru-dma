/*
 * PRU DMA - LEDs example
 *
 * Copyright (C) 2017 Maciej Sobkowski <maciej@sobkow.ski>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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

static ssize_t pru_dma_leds_tx_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	struct pru_dma_leds *pru_dma_leds =
		platform_get_drvdata(to_platform_device(dev));

	ret = pru_dma_tx_trigger(pru_dma_leds->pru_dma, 0);
	if (ret)
		return ret;

	dev_dbg(pru_dma_leds->dev, "Tx triggered.\n");

	return count;
}

static DEVICE_ATTR(tx, S_IWUSR, NULL, pru_dma_leds_tx_store);

static struct attribute *pru_dma_leds_attributes[] = {
	&dev_attr_tx.attr,
	NULL,
};

static const struct attribute_group pru_dma_leds_group = {
		.attrs = pru_dma_leds_attributes,
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

	dev_dbg(pru_dma_leds->dev, "Got PRU DMA handle!.\n");

	buf = pru_dma_get_buffer(pru_dma_leds->pru_dma, 0);
	if (IS_ERR(buf)) {
		ret = PTR_ERR(buf);
		if (ret != -EPROBE_DEFER)
			dev_err(pru_dma_leds->dev, "Unable to get DMA buffer.\n");
		return ret;
	}

	buf_size = pru_dma_get_buffer_size(pru_dma_leds->pru_dma, 0);

	leds_pattern_generate(buf, buf_size);

	dev_dbg(pru_dma_leds->dev, "Mapped buffer!.\n");

	ret = sysfs_create_group(&pdev->dev.kobj, &pru_dma_leds_group);
	if (ret) {
		dev_err(&pdev->dev, "sysfs_create_group() failed (%d)\n",
				ret);
		return ret;
	}

	dev_dbg(pru_dma_leds->dev, "Probe success");
	return 0;
}

int pru_dma_leds_remove(struct platform_device *pdev)
{
	struct pru_dma_leds *pru_dma_leds = dev_get_drvdata(&pdev->dev);

	pru_dma_unmap_buffer(pru_dma_leds->pru_dma, 0);

	sysfs_remove_group(&pdev->dev.kobj, &pru_dma_leds_group);

	dev_dbg(pru_dma_leds->dev, "Unmapped buffer.\n");
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
MODULE_LICENSE("GPL v2");
