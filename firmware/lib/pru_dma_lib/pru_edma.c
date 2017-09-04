/*
 * Source Modified by Maciej Sobkowski <maciej@sobkow.ski>
 * Based on the examples distributed by TI
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *	* Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 *
 *	* Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the
 *	  distribution.
 *
 *	* Neither the name of Texas Instruments Incorporated nor the names of
 *	  its contributors may be used to endorse or promote products derived
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
 */

#include <stdint.h>
#include "pru_edma.h"
#include "pru_edma_regs.h"

/*
 * Sets up EDMA registers and PaRAM slot.
 *
 * @edma_ptr - base address of EDMA instance
 * @edma_xfer_data - struct describing transfer parameters (src and dst
 * address, EDMA channel and PaRAM slot to use
 */

void edma_setup(volatile uint32_t *edma_ptr, edma_data *edma_buf)
{
	uint16_t paramOffset;
	edmaParam params;
	volatile edmaParam *pParams;

	/* TODO: Select proper DCHMAP register and offset 
	 *based on channel number
	 */
	edma_ptr[DCHMAP_12] = (edma_buf->slot << 5);

	/* Assign our channel to Shadow Region 1 
	 * (Must use SR1 because it is the only one
	 * that can generate completion interrupt to PRU.
	 */
	edma_ptr[DRAE1] |= CHAN_MASK(edma_buf->chan);

	/* Clear channel event from EDMA event registers */
	edma_ptr[SECR] |= CHAN_MASK(edma_buf->chan);
	edma_ptr[ICR] |= CHAN_MASK(edma_buf->chan);

	/* Enable channel interrupt */
	edma_ptr[IESR] |= CHAN_MASK(edma_buf->chan);

	/* Enable channel */
	edma_ptr[EESR] |= CHAN_MASK(edma_buf->chan);

	/* Clear event missed register */
	edma_ptr[EMCR] |= CHAN_MASK(edma_buf->chan);

	/* Setup channel to submit to EDMA TC0
	 * TODO: Compute proper value for DMAQNUMx based on 
	 * provided channel number
	 */
	edma_ptr[DMAQNUM1] &= 0xFFF8FFFF;

	/* Setup and store PaRAM set for transfer */
	paramOffset = PARAM_OFFSET;

	/* Move paramOffset to appropriate PaRAM slot  */
	paramOffset += ((edma_buf->slot << 5) / 4);

	params.lnkrld.link = 0xFFFF;
	params.lnkrld.bcntrld = 0x0000;
	params.opt.tcc = edma_buf->chan;
	params.opt.tcinten = 1;
	params.opt.itcchen = 1;

	params.abcnt.acnt = edma_buf->size * sizeof(uint32_t);
	params.abcnt.bcnt = BCNT;
	params.ccnt.ccnt = CCNT;
	params.bidx.srcbidx = 0x1;
	params.bidx.dstbidx = 0x1;
	params.src = edma_buf->src;
	params.dst = edma_buf->dst;

	pParams = (volatile edmaParam *)(edma_ptr + paramOffset);
	*pParams = params;
}

void edma_set_buffer(volatile uint32_t *edma_ptr, edma_data *edma_buf)
{
	edmaParam params;
	uint16_t paramOffset;
	volatile edmaParam *pParams;

	/* Setup and store PaRAM set for transfer */
	paramOffset = PARAM_OFFSET;

	/* Move paramOffset to appropriate PaRAM slot  */
	paramOffset += ((edma_buf->slot << 5) / 4);

	params.lnkrld.link = 0xFFFF;
	params.lnkrld.bcntrld = 0x0000;
	params.opt.tcc = edma_buf->chan;
	params.opt.tcinten = 1;
	params.opt.itcchen = 1;

	params.abcnt.acnt = edma_buf->size * sizeof(uint32_t);
	params.abcnt.bcnt = BCNT;
	params.ccnt.ccnt = CCNT;
	params.bidx.srcbidx = 0x1;
	params.bidx.dstbidx = 0x1;
	params.src = edma_buf->src;
	params.dst = edma_buf->dst;

	pParams = (volatile edmaParam *)(edma_ptr + paramOffset);
	*pParams = params;
}


/*
 * Trigger transfer (must be configured using edma_setup first)
 *
 * @edma_ptr - base address of EDMA instance
 * @edma_xfer_data - struct describing transfer parameters (src and dst
 * address, EDMA channel and PaRAM slot to use
 */
void edma_trigger(volatile uint32_t *edma_ptr, edma_data *edma_buf)
{
	edma_ptr[ESR] = (CHAN_MASK(edma_buf->chan));
}

int edma_check(volatile uint32_t *edma_ptr, edma_data *edma_buf)
{
	/* Check transfer completion */
	if (edma_ptr[IPR] & CHAN_MASK(edma_buf->chan))
		return 1;

	return 0;
}

