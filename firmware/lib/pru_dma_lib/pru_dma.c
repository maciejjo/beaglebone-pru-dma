#include "pru_defs.h"
#include "pru_edma.h"
#include <pru_intc.h>
#include <pru_cfg.h>
#include <pru_dma.h>

#define CHAN_NAME			"pru-dma"
#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

#define VIRTIO_CONFIG_S_DRIVER_OK	4

#define PRU_DMA_IS_TX_COMPLETE() ((__R31 & HOST0_INT) && CHECK_EVT(EVT_FROM_EDMA))
#define PRU_DMA_TX_ACK()         (CLEAR_EVT(EVT_FROM_EDMA))

static volatile uint32_t *edma_ptr;
static struct pru_rpmsg_transport transport;
static uint16_t rpmsg_src, rpmsg_dst, rpmsg_len;
uint8_t rpmsg_init_done = 0;

struct pru_dma_tx_desc {
	uint32_t kbuf_addr;
	uint32_t kbuf_size;
	uint8_t  edma_slot;
	uint8_t  edma_chan;
	uint8_t  flags;
};

#define TX_DESC_FLAGS_NOTIFY_COMPLETION (1 << 0)

#define PRU_DMA_TX_COMPLETED (uint8_t) (0x01)

static struct pru_dma_tx_desc tx_desc;
static edma_data edma_buf;

void pru_dma_init(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_vdev *rpmsg_vdev,
			struct fw_rsc_vdev_vring *rpmsg_vring0,
			struct fw_rsc_vdev_vring *rpmsg_vring1,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num)
{

	if (!rpmsg_init_done) {
		volatile uint8_t *status;

		status = &rpmsg_vdev->status;

		while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK))
			;

		pru_rpmsg_init(&transport, rpmsg_vring0, rpmsg_vring1,
				EVT_TO_ARM_HOST, EVT_FROM_ARM_HOST);

		while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME,
					CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS)
			;

		rpmsg_init_done = 1;
	}

	
	if (dir == PRU_DMA_DIR_ARM_TO_PRU) {
		edma_buf.src = pru_dmas->dma_ch[chan_num].buf_addr;
		edma_buf.dst = PRU_SHMEM_OFFSET;
	} else {
		edma_buf.src = PRU_SHMEM_OFFSET;
		edma_buf.dst = pru_dmas->dma_ch[chan_num].buf_addr;
	}

	edma_buf.chan = pru_dmas->dma_ch[chan_num].edma_channel;
	edma_buf.slot = pru_dmas->dma_ch[chan_num].param_slot;
	edma_buf.size = pru_dmas->dma_ch[chan_num].buf_addr;

	edma_ptr = EDMA0_CC_BASE;

	// Set up EDMA for transfer 
	edma_setup(edma_ptr, &edma_buf);

	dma_data->src = edma_buf.src;
	dma_data->dst = edma_buf.dst;
	dma_data->size = edma_buf.size;
}

void pru_dma_wait_host()
{
	while (1) {

		if (__R31 & HOST1_INT) {

			CT_INTC.SICR_bit.STS_CLR_IDX = EVT_FROM_ARM_HOST;

			if (pru_rpmsg_receive(&transport, &rpmsg_src,
						&rpmsg_dst, &tx_desc,
						&rpmsg_len) ==
					PRU_RPMSG_SUCCESS) {
				break;
			}
		}
	}
}

void pru_dma_trigger()
{
	/* Clear EDMA event and transfer */
	CLEAR_EVT(EVT_FROM_EDMA);
	edma_trigger(edma_ptr, &edma_buf);
}

static void pru_dma_ack()
{
	uint8_t msg = PRU_DMA_TX_COMPLETED;
	uint16_t len = sizeof(msg);

	PRU_DMA_TX_ACK();

	if (tx_desc.flags & TX_DESC_FLAGS_NOTIFY_COMPLETION)
		pru_rpmsg_send(&transport, rpmsg_dst, rpmsg_src, &msg,
				len);
}

void pru_dma_wait()
{
	while(1) {
		if (PRU_DMA_IS_TX_COMPLETE() && edma_check(edma_ptr, &edma_buf)) {
			pru_dma_ack();
			break;
		}
	}
}

