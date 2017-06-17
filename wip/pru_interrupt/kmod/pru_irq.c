#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pruss.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>

#define DRV_NAME "pru_irq"

struct pru_irq_data {
	struct device *dev;
	int irq_from_pru;
	int irq_to_pru;
};

irqreturn_t pru_irq_handler(int irq, void *data)
{
	struct pru_irq_data *pi = data;
	dev_dbg(pi->dev, "Interrupt from PRU received");

	dev_dbg(pi->dev, "Sending interrupt to PRU");
	pruss_intc_trigger(pi->irq_to_pru);

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

	pi->irq_from_pru = platform_get_irq_byname(pdev, "irq_from_pru");
	if (pi->irq_from_pru < 0)
		return pi->irq_from_pru;

	pi->irq_to_pru = platform_get_irq_byname(pdev, "irq_to_pru");
	if (pi->irq_to_pru < 0)
		return pi->irq_to_pru;

	ret = devm_request_irq(&pdev->dev, pi->irq_from_pru, pru_irq_handler,
			IRQF_ONESHOT, dev_name(&pdev->dev), pi);

	dev_dbg(pi->dev, "Probe success");

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
