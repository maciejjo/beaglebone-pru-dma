#include <linux/init.h>
#include <linux/module.h>

static int __init pru_dma_init(void)
{
	pr_debug("PRU DMA kmod\n");
	return 0;
}

static void __exit pru_dma_exit(void)
{
}

module_init(pru_dma_init);
module_exit(pru_dma_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Maciej Sobkowski");

