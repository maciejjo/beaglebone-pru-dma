#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include "resource_table1.h"
#include "pru_edma.h"

#define EDMA_EVENT_NUM 63

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
	edma_data edma_buf;
	volatile uint32_t *edma_ptr;
	volatile uint32_t *src, *dst;
	int i;

	/* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Clear all system events */
	CT_INTC.SECR0 = 0xFFFFFFFF;
	CT_INTC.SECR1 = 0xFFFFFFFF;

	/* TODO: Buffer, channel and slot supplied by kernel module */
	edma_buf.src = 0x4A310000;		
	edma_buf.dst = 0x4A310100;	
	edma_buf.chan = 12;
	edma_buf.slot = 200;

	/* Poitners to source and destination buffer for pattern testing */
	src = (uint32_t *) edma_buf.src;
	dst = (uint32_t *) edma_buf.dst;

	edma_ptr = EDMA0_CC_BASE;

	/* Write pattern to source buffer */
	for (i = 0; i < 100; i++) {
		*src = i % 2 ? 0x55555555 : 0xaaaaaaaa;
		src++;
	}

	/* Set up EDMA for transfer */
	edma_setup(edma_ptr, &edma_buf);

	/* Clear EDMA event and transfer */
	CT_INTC.SICR_bit.STS_CLR_IDX = EDMA_EVENT_NUM;

	__R30 = 0xffff;
	__delay_cycles(50000000);

	/* Wait for event on channel 0 */
	while(!(__R31 & (1U << 30)))
		;

	/* Check if event from EDMA arrived, if yes push content of memory to LEDs */
	if (CT_INTC.SECR1 & (1U << (EDMA_EVENT_NUM - 32))) {

		for (i = 0; i < 100; i++) {
			__R30 = *dst;
			dst++;
			__delay_cycles(5000000);

		}

		/* Clear the EDMA event */
		CT_INTC.SECR1 = (1U << 31);
	}

	__halt();
}
