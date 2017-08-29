#include "resource_table1.h"
#include <pru_dma.h>
#include <pru_cfg.h>
#include <pru_intc.h>

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
	struct pru_dma_data dma_data;
	volatile uint32_t *ptr;
	uint32_t buf[100];
	int i;

	/* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	pru_dma_init(&dma_data,
			PRU_DMA_DIR_ARM_TO_PRU,
			&resourceTable.pru_dmas.rsc.pru_dma,
			0
			);


	while (1) {

		pru_dma_wait_host();
		pru_dma_trigger();
		pru_dma_wait();

		ptr = (uint32_t *) dma_data.dst;

		for (i = 0; i < dma_data.size; i++)
			buf[i] = *(ptr + i);

		pru_dma_set_dir(&dma_data, PRU_DMA_DIR_PRU_TO_ARM,
			&resourceTable.pru_dmas.rsc.pru_dma,
			0);


		ptr = (uint32_t *) dma_data.src;

		*(ptr) = 0x50505000;

		pru_dma_wait_host();
		pru_dma_trigger();
		pru_dma_wait();


	}
}
