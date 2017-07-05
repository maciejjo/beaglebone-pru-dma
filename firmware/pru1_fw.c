#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_rpmsg.h>
#include "resource_table1.h"
#include "pru_edma.h"

#define EVT_FROM_EDMA 			63
#define EVT_TO_ARM_HOST			18
#define EVT_FROM_ARM_HOST		19

#define CHAN_NAME			"pru-dma"
#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

#define HOST0_INT			((uint32_t) 1 << 30)
#define HOST1_INT			((uint32_t) 1 << 31)

#define VIRTIO_CONFIG_S_DRIVER_OK	4

volatile register uint32_t __R30;
volatile register uint32_t __R31;

uint8_t payload[RPMSG_BUF_SIZE];

void main(void)
{
	edma_data edma_buf;
	volatile uint32_t *edma_ptr;
	volatile uint32_t *src, *dst;
	int i;

	struct pru_rpmsg_transport transport;
	volatile uint8_t *status;
	uint16_t rpmsg_src, rpmsg_dst, rpmsg_len;

	/* Clear SYSCFG[STANDBY_INIT] to enable OCP master port */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

	/* Clear all system events */
	CT_INTC.SECR0 = 0xFFFFFFFF;
	CT_INTC.SECR1 = 0xFFFFFFFF;


	__R30 = 0x0;

	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

	pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0,
			&resourceTable.rpmsg_vring1,
			EVT_TO_ARM_HOST, EVT_FROM_ARM_HOST);

	while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME,
				CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);

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
	for (i = 0; i < 100; i++)
		*(src + i) = i % 2 ? 0x55555555 : 0xaaaaaaaa;

	/* Set up EDMA for transfer */
	edma_setup(edma_ptr, &edma_buf);

	/* Clear EDMA event and transfer */
	CT_INTC.SICR_bit.STS_CLR_IDX = EVT_FROM_EDMA;

	edma_trigger(edma_ptr, &edma_buf);

	/* Wait for event on channel 0 */
	while(!(__R31 & HOST0_INT))
		;

	/* Check if event from EDMA arrived, if yes push content of memory to LEDs */
	if (CT_INTC.SECR1 & (1U << (EVT_FROM_EDMA - 32))) {

		/* Clear the EDMA event */
		CT_INTC.SECR1 = (1U << 31);

		for (i = 0; i < 100; i++) {
			__R30 = *(dst + i);
			__delay_cycles(5000000);

		}

	}

	while (1) {
		if (__R31 & HOST1_INT) {
			CT_INTC.SICR_bit.STS_CLR_IDX = EVT_FROM_ARM_HOST;
			while (pru_rpmsg_receive(&transport, &rpmsg_src, &rpmsg_dst, payload, &rpmsg_len) == PRU_RPMSG_SUCCESS) {
				pru_rpmsg_send(&transport, rpmsg_dst, rpmsg_src, payload, rpmsg_len);
			}
		}
	}
}
