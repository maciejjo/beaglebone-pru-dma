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

#include "pru_defs.h"
#include "pru_edma.h"
#include <pru_intc.h>
#include <pru_cfg.h>
#include <pru_dma.h>

#define PRU_DMA_IS_TX_COMPLETE() ((__R31 & HOST0_INT) && CHECK_EVT(EVT_FROM_EDMA))
#define PRU_DMA_TX_ACK()         (CLEAR_EVT(EVT_FROM_EDMA))

static volatile uint32_t *edma_ptr;
uint8_t rpmsg_init_done = 0;

uint32_t mem[2] __attribute__((location(0))) = {0};


#define PRU_DMA_FLAGS_NOTIFY_COMPLETION (1 << 0)


struct current_dma_data {
	edma_data edma_buf;	
	uint32_t tx_flags;
	uint32_t chan_num;
};

static struct current_dma_data cd;

void pru_dma_init(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num)
{
	if (dir == PRU_DMA_DIR_ARM_TO_PRU) {
		cd.edma_buf.src = pru_dmas->dma_ch[chan_num].buf_addr;
		cd.edma_buf.dst = PRU_SHMEM_OFFSET;
	} else {
		cd.edma_buf.src = PRU_SHMEM_OFFSET;
		cd.edma_buf.dst = pru_dmas->dma_ch[chan_num].buf_addr;
	}

	cd.edma_buf.chan = pru_dmas->dma_ch[chan_num].edma_channel;
	cd.edma_buf.slot = pru_dmas->dma_ch[chan_num].param_slot;
	cd.edma_buf.size = pru_dmas->dma_ch[chan_num].buf_size;

	cd.chan_num = chan_num;

	if (pru_dmas->dma_ch[chan_num].notify_completion)
		cd.tx_flags = PRU_DMA_FLAGS_NOTIFY_COMPLETION;

	edma_ptr = EDMA0_CC_BASE;

	// Set up EDMA for transfer 
	edma_setup(edma_ptr, &cd.edma_buf);

	dma_data->src = cd.edma_buf.src;
	dma_data->dst = cd.edma_buf.dst;
	dma_data->size = cd.edma_buf.size;

	CT_INTC.SICR_bit.STS_CLR_IDX = EVT_FROM_ARM_HOST;
}

void pru_dma_set_dir(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num)
{
	if (dir == PRU_DMA_DIR_ARM_TO_PRU) {
		cd.edma_buf.src = pru_dmas->dma_ch[chan_num].buf_addr;
		cd.edma_buf.dst = PRU_SHMEM_OFFSET;
	} else {
		cd.edma_buf.src = PRU_SHMEM_OFFSET;
		cd.edma_buf.dst = pru_dmas->dma_ch[chan_num].buf_addr;
	}

	edma_ptr = EDMA0_CC_BASE;

	// Set up EDMA for transfer 
	edma_set_buffer(edma_ptr, &cd.edma_buf);

	dma_data->src = cd.edma_buf.src;
	dma_data->dst = cd.edma_buf.dst;
	dma_data->size = cd.edma_buf.size;
}

void pru_dma_wait_host()
{

	while (!(__R31 & HOST1_INT))
		;

	CT_INTC.SICR_bit.STS_CLR_IDX = EVT_FROM_ARM_HOST;
}

void pru_dma_trigger()
{
	/* Clear EDMA event and transfer */
	edma_trigger(edma_ptr, &cd.edma_buf);
}

static void pru_dma_ack(uint32_t flags)
{
	PRU_DMA_TX_ACK();

	if (flags & PRU_DMA_FLAGS_NOTIFY_COMPLETION) {
		mem[0] |= 1 << cd.chan_num;
		SIGNAL_EVENT(EVT_TO_ARM_HOST);
	}
}

void pru_dma_wait()
{
	while (1) {
		if (edma_check(edma_ptr, &cd.edma_buf)) {
			pru_dma_ack(cd.tx_flags);
			break;
		}
	}
}

