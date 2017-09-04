/*
 * PRU DMA - Loopback example
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

#define DRV_NAME "pru_dma_loopback"

struct pru_dma_loop {
	struct device *dev;
	struct pru_dma *pru_dma;
	uint32_t *dma_buf;
	uint32_t dma_buf_size;
};

static ssize_t pru_dma_loop_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int cnt;
	struct pru_dma_loop *pru_dma_loop =
		platform_get_drvdata(to_platform_device(dev));

	if (count > pru_dma_loop->dma_buf_size)
		cnt = pru_dma_loop->dma_buf_size;
	else
		cnt = count;

	strncpy((char *)pru_dma_loop->dma_buf, buf, cnt);

	ret = pru_dma_tx_trigger(pru_dma_loop->pru_dma, 0);
	if (ret)
		return ret;

	pru_dma_tx_completion_wait(pru_dma_loop->pru_dma, 0);

	return cnt;
}

static ssize_t pru_dma_loop_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret;

	struct pru_dma_loop *pru_dma_loop =
		platform_get_drvdata(to_platform_device(dev));

	ret = pru_dma_tx_trigger(pru_dma_loop->pru_dma, 0);
	if (ret)
		return ret;

	pru_dma_tx_completion_wait(pru_dma_loop->pru_dma, 0);

	strncpy(buf, (char *)pru_dma_loop->dma_buf, pru_dma_loop->dma_buf_size);

	return pru_dma_loop->dma_buf_size;
}

static DEVICE_ATTR(loop, S_IWUSR | S_IRUGO, pru_dma_loop_show, pru_dma_loop_store);

static struct attribute *pru_dma_loop_attributes[] = {
	&dev_attr_loop.attr,
	NULL,
};

static const struct attribute_group pru_dma_loop_group = {
		.attrs = pru_dma_loop_attributes,
};

int pru_dma_loop_probe(struct platform_device *pdev)
{
	struct pru_dma_loop *pru_dma_loop;
	int ret;

	pru_dma_loop = devm_kzalloc(&pdev->dev, sizeof(*pru_dma_loop), GFP_KERNEL);
	if (!pru_dma_loop)
		return -ENOMEM;

	pru_dma_loop->dev = &pdev->dev;

	dev_set_drvdata(&pdev->dev, pru_dma_loop);
	pru_dma_loop->pru_dma = pru_dma_get("pru-dma");

	if (IS_ERR(pru_dma_loop->pru_dma)) {
		ret = PTR_ERR(pru_dma_loop->pru_dma);
		if (ret != -EPROBE_DEFER)
			dev_err(pru_dma_loop->dev, "Unable to get pru_dma handle.\n");
		return ret;
	}

	pru_dma_loop->dma_buf = pru_dma_get_buffer(pru_dma_loop->pru_dma, 0);
	if (IS_ERR(pru_dma_loop->dma_buf)) {
		ret = PTR_ERR(pru_dma_loop->dma_buf);
		if (ret != -EPROBE_DEFER)
			dev_err(pru_dma_loop->dev, "Unable to get DMA buffer (%d).\n", ret);
		return ret;
	}

	pru_dma_loop->dma_buf_size = pru_dma_get_buffer_size(pru_dma_loop->pru_dma, 0);

	ret = sysfs_create_group(&pdev->dev.kobj, &pru_dma_loop_group);
	if (ret) {
		dev_err(&pdev->dev, "sysfs_create_group() failed (%d)\n",
				ret);
		return ret;
	}

	dev_dbg(pru_dma_loop->dev, "Probe success");
	return 0;
}

int pru_dma_loop_remove(struct platform_device *pdev)
{
	struct pru_dma_loop *pru_dma_loop = dev_get_drvdata(&pdev->dev);

	pru_dma_unmap_buffer(pru_dma_loop->pru_dma, 0);

	sysfs_remove_group(&pdev->dev.kobj, &pru_dma_loop_group);

	return 0;
}

static const struct of_device_id pru_dma_loop_ids[] = {
	{ .compatible = "pru-dma-loopback", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_dma_loop_ids);

static struct platform_driver pru_dma_loop_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_dma_loop_ids,
	},
	.probe = pru_dma_loop_probe,
	.remove = pru_dma_loop_remove,
};

module_platform_driver(pru_dma_loop_driver);

MODULE_DESCRIPTION("PRU DMA loopback driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL v2");
