#include <linux/module.h>
#include <linux/init.h>

#include <linux/platform_device.h>

#include <linux/interrupt.h>
#include <linux/irqreturn.h>

#define DRV_NAME "pru_irq"

struct pru_irq_data {
	struct device *dev;
	int irq;
};

irqreturn_t pru_irq_handler(int irq, void *data)
{
	pr_err("Interrupt from PRU received");
	return IRQ_HANDLED;
}

int pru_irq_probe(struct platform_device *pdev)
{
	struct pru_irq_data *pi;
	int ret;

	pi = devm_kzalloc(&pdev->dev, sizeof(*pi), GFP_KERNEL);
	if (!pi)
		return -ENOMEM;

	pi->dev = &pdev->dev;

	dev_set_drvdata(&pdev->dev, pi);

	pi->irq = platform_get_irq_byname(pdev, "irq");
	if (pi->irq < 0)
		return pi->irq;

	ret = devm_request_irq(&pdev->dev, pi->irq, pru_irq_handler,
			IRQF_ONESHOT, dev_name(&pdev->dev), NULL);

	pr_err("Probe success");

	return 0;
}

int pru_irq_remove(struct platform_device *pdev)
{
	return 0;
}


static const struct of_device_id pru_irq_ids[] = {
	{ .compatible = "pru-irq", },
	{},
};
MODULE_DEVICE_TABLE(of, pru_irq_ids);

static struct platform_driver pru_irq_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = pru_irq_ids,
	},
	.probe = pru_irq_probe,
	.remove = pru_irq_remove,
};

module_platform_driver(pru_irq_driver);

MODULE_DESCRIPTION("PRU IRQ driver");
MODULE_AUTHOR("Maciej Sobkowski");
MODULE_LICENSE("GPL");
