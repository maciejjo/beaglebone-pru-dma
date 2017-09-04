/*
 * Copyright (C) 2017 Maciej Sobkowski <maciej@sobkow.ski>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "resource_table1.h"
#include <pru_dma.h>
#include <pru_cfg.h>
#include <pru_intc.h>

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
	struct pru_dma_data dma_data;
	volatile uint32_t *dst;
	int i;

	/* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	pru_dma_init(&dma_data,
			PRU_DMA_DIR_ARM_TO_PRU,
			&resourceTable.pru_dmas.rsc.pru_dma,
			0
			);

	/* Poitner to destination buffer for pattern testing */
	dst = (uint32_t *) dma_data.dst;

	while (1) {

		/* Clear all GPO pins */
		__R30 = 0x00;

		pru_dma_wait_host();

		pru_dma_trigger();

		/* Check if event from EDMA arrived, when so, push content of memory to
		 * LEDs
		 */
		pru_dma_wait();

		for (i = 0; i < dma_data.size; i++) {
			__R30 = *(dst + i);
			__delay_cycles(5000000);

		}
	}
}
