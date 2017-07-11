#ifndef PRU_DMA_H
#define PRU_DMA_H

#include <stdint.h>
#include <pru_rpmsg.h>

#define EVT_FROM_EDMA 			63
#define EVT_TO_ARM_HOST			18
#define EVT_FROM_ARM_HOST		19

#define HOST0_INT			((uint32_t) 1 << 30)
#define HOST1_INT			((uint32_t) 1 << 31)

struct pru_dma_data {
	uint32_t src;
	uint32_t dst;
	uint32_t size;
};

void pru_dma_init(struct pru_dma_data *dma_data,
			struct fw_rsc_vdev *rpmsg_vdev,
			struct fw_rsc_vdev_vring *rpmsg_vring0,
			struct fw_rsc_vdev_vring *rpmsg_vring1);
void pru_dma_trigger();
void pru_dma_wait();

#endif /* PRU_DMA_H */
