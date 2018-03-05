/*
 * Copyright (C) 2017-2018 Maciej Sobkowski <maciej@sobkow.ski>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in
 *	  the documentation and/or other materials provided with the
 *	  distribution.
 *
 *	* Neither the name of the copyright holders nor the names of
 *	  contributors may be used to endorse or promote products derived
 *	  from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the GNU
 * General Public License ("GPL") version 2 as published by the Free Software
 * Foundation.
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
