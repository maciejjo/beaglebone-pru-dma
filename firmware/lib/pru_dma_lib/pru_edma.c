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

