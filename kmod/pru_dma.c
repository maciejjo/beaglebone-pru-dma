#include <linux/module.h>
#include <linux/init.h>
#include <linux/pruss.h>
#include <linux/rpmsg.h>
#include <linux/pinctrl/consumer.h>

#define DRV_NAME "pru_dma"

struct pru_dma_data {
	struct device *dev;
};

static int pru_dma_rx_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	print_hex_dump(KERN_INFO, "incoming message:", DUMP_PREFIX_NONE,
						16, 1, data, len, true);
	return 0;
}

static int pru_dma_probe(struct rpmsg_device *rpdev)
{
	struct pru_dma_data *pru_dma;
	int ret;

	pru_dma = devm_kzalloc(&rpdev->dev, sizeof(*pru_dma), GFP_KERNEL);
	if (!pru_dma)
		return -ENOMEM;

	pru_dma->dev = &rpdev->dev;

	dev_set_drvdata(&rpdev->dev, pru_dma);

	ret = rpmsg_send(rpdev->ept, "test", 4);
	if (ret) {
		pr_err("rpmsg_send failed: %d\n", ret);
		return ret;
	}

	dev_dbg(pru_dma->dev, "Probe success");

	return 0;
}

static void pru_dma_remove(struct rpmsg_device *rpdev)
{
}

static struct rpmsg_device_id pru_dma_id_table[] = {
	{ .name	= "pru-dma" },
	{ },
};
MODULE_DEVICE_TABLE(rpmsg, pru_dma_id_table);

static struct rpmsg_driver pru_dma_driver = {
	.drv.name	= KBUILD_MODNAME,
	.id_table	= pru_dma_id_table,
	.probe		= pru_dma_probe,
	.callback	= pru_dma_rx_cb,
	.remove		= pru_dma_remove,
};
module_rpmsg_driver(pru_dma_driver);

MODULE_DESCRIPTION("PRU DMA driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
