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

#ifndef PRU_DMA_H
#define PRU_DMA_H

#include <stdint.h>
#include <pru_rpmsg.h>
#include <pru_types.h>

#define EVT_FROM_EDMA 			63
#define EVT_TO_ARM_HOST			22
#define EVT_FROM_ARM_HOST		23

#define HOST0_INT			((uint32_t) 1 << 30)
#define HOST1_INT			((uint32_t) 1 << 31)

struct pru_dma_data {
	uint32_t src;
	uint32_t dst;
	uint32_t size;
};

enum pru_dma_direction {
	PRU_DMA_DIR_ARM_TO_PRU,
	PRU_DMA_DIR_PRU_TO_ARM,
};

void pru_dma_init(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num);

void pru_dma_set_dir(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num);
void pru_dma_wait_host();
void pru_dma_trigger();
void pru_dma_wait();

#endif /* PRU_DMA_H */
