#include <linux/module.h>
#include <linux/init.h>
#include <linux/pruss.h>
#include <linux/rpmsg.h>
#include <linux/pinctrl/consumer.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/pru_dma.h>

#define DRV_NAME "pru_dma"

struct pru_dma {
	struct list_head node;
	struct device *dev;
	struct rpmsg_device *rpdev;
	uint32_t edma_slot;
	uint32_t edma_channel;
	uint32_t kbuf_size;
	uint32_t *kbuf;
	dma_addr_t kbuf_dma;
	const char *chan_name;
	uint32_t notify;
};

struct edma_tx_desc {
	uint32_t kbuf_addr;
	uint32_t kbuf_size;
	uint8_t  edma_slot;
	uint8_t  edma_chan;
	uint8_t  flags;
};

#define TX_DESC_FLAGS_NOTIFY_COMPLETION (1 << 0)

#define PRU_DMA_TX_COMPLETED (uint8_t) (0x01)

static DEFINE_MUTEX(pru_dma_list_mutex);
static LIST_HEAD(pru_dma_list);

uint32_t *pru_dma_get_buffer(struct pru_dma *pru_dma)
{
	return pru_dma->kbuf;
}
EXPORT_SYMBOL_GPL(pru_dma_get_buffer);

uint32_t pru_dma_get_buffer_size(struct pru_dma *pru_dma)
{
	return pru_dma->kbuf_size;
}
EXPORT_SYMBOL_GPL(pru_dma_get_buffer_size);

int pru_dma_map_buffer(struct pru_dma *pru_dma)
{
	int ret;

	pru_dma->kbuf_dma = dma_map_single(pru_dma->dev, pru_dma->kbuf,
			pru_dma->kbuf_size, DMA_BIDIRECTIONAL);
	ret = dma_mapping_error(pru_dma->dev, pru_dma->kbuf_dma);
	if (ret) {
		dev_err(pru_dma->dev, "Buffer DMA mapping failed");
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(pru_dma_map_buffer);

void pru_dma_unmap_buffer(struct pru_dma *pru_dma)
{
	dma_unmap_single(pru_dma->dev, pru_dma->kbuf_dma,
			pru_dma->kbuf_size, DMA_BIDIRECTIONAL);
}
EXPORT_SYMBOL_GPL(pru_dma_unmap_buffer);

int pru_dma_tx_trigger(struct pru_dma *pru_dma)
{
	int ret;
	struct edma_tx_desc tx_data;

	tx_data.kbuf_addr = pru_dma->kbuf_dma;
	tx_data.kbuf_size = pru_dma->kbuf_size;
	tx_data.edma_slot = pru_dma->edma_slot;
	tx_data.edma_chan = pru_dma->edma_channel;
	tx_data.flags     = pru_dma->notify ? (TX_DESC_FLAGS_NOTIFY_COMPLETION) : (0x00);

	dev_dbg(pru_dma->dev, "Sending msg of size %d\n", sizeof(tx_data));
	ret = rpmsg_send(pru_dma->rpdev->ept, &tx_data, sizeof(tx_data));
	if (ret) {
		pr_err("rpmsg_send failed: %d\n", ret);
		return ret;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(pru_dma_tx_trigger);

struct pru_dma *pru_dma_get(char *chan_name)
{
	struct pru_dma *pru_dma = NULL, *p;

	mutex_lock(&pru_dma_list_mutex);
	list_for_each_entry(p, &pru_dma_list, node) {
		if (strcmp(chan_name, p->chan_name) == 0) {
			pru_dma = p;
			get_device(pru_dma->dev);
			break;
		}
	}

	mutex_unlock(&pru_dma_list_mutex);

	return pru_dma ? pru_dma : ERR_PTR(-EPROBE_DEFER);
}
EXPORT_SYMBOL_GPL(pru_dma_get);

void pru_dma_put(struct pru_dma *pru_dma)
{
	if (!pru_dma)
		return;

	put_device(pru_dma->dev);
}
EXPORT_SYMBOL_GPL(pru_dma_put);

static int pru_dma_rx_cb(struct rpmsg_device *rpdev, void *data, int len,
						void *priv, u32 src)
{
	print_hex_dump(KERN_INFO, "incoming message:", DUMP_PREFIX_NONE,
						16, 1, data, len, true);
	return 0;
}

static int pru_dma_probe(struct rpmsg_device *rpdev)
{
	struct pru_dma *pru_dma;
	struct device_node *np = of_find_node_by_name(NULL, "pru_dma");
	int ret;

	if (!np) {
		dev_err(&rpdev->dev, "must be instantiated via devicetree\n");
		return -ENOENT;
	}

	pru_dma = devm_kzalloc(&rpdev->dev, sizeof(*pru_dma), GFP_KERNEL);
	if (!pru_dma)
		return -ENOMEM;

	pru_dma->dev = &rpdev->dev;
	pru_dma->rpdev = rpdev;

	dev_set_drvdata(&rpdev->dev, pru_dma);

	ret = of_property_read_u32(np, "edma-channel", &pru_dma->edma_channel);
	if (ret) {
		dev_err(pru_dma->dev, "invalid edma-channel in %s\n",
				np->full_name);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "edma-slot", &pru_dma->edma_slot);
	if (ret) {
		dev_err(pru_dma->dev, "invalid edma-slot in %s\n",
				np->full_name);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "buffer-size", &pru_dma->kbuf_size);
	if (ret) {
		dev_err(pru_dma->dev, "invalid buffer-size in %s\n",
				np->full_name);
		return -EINVAL;
	}

	ret = of_property_read_string(np, "chan-name", &pru_dma->chan_name);
	if (ret) {
		dev_err(pru_dma->dev, "invalid chan-name in %s\n",
				np->full_name);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "notify-completion", &pru_dma->notify);
	if (ret) {
		dev_err(pru_dma->dev, "invalid notify-completion in %s\n",
				np->full_name);
		return -EINVAL;
	}

	pru_dma->kbuf = devm_kzalloc(pru_dma->dev,
				pru_dma->kbuf_size * sizeof(uint32_t),
				GFP_KERNEL);
	if (!pru_dma->kbuf)
		return -ENOMEM;

	dev_dbg(pru_dma->dev, "Probe success");

	mutex_lock(&pru_dma_list_mutex);
	list_add_tail(&pru_dma->node, &pru_dma_list);
	mutex_unlock(&pru_dma_list_mutex);

	return 0;
}

static void pru_dma_remove(struct rpmsg_device *rpdev)
{
	struct pru_dma *pru_dma = dev_get_drvdata(&rpdev->dev);

	mutex_lock(&pru_dma_list_mutex);
	list_del(&pru_dma->node);
	mutex_unlock(&pru_dma_list_mutex);
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
