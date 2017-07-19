#ifndef __LINUX_PRU_DMA_H
#define __LINUX_PRU_DMA_H

struct pru_dma;

struct pru_dma *pru_dma_get(char *chan_name);
void pru_dma_put(struct pru_dma *pru_dma);

uint32_t *pru_dma_get_buffer(struct pru_dma *pru_dma);
uint32_t pru_dma_get_buffer_size(struct pru_dma *pru_dma);

int pru_dma_map_buffer(struct pru_dma *pru_dma);
void pru_dma_unmap_buffer(struct pru_dma *pru_dma);

int pru_dma_tx_trigger(struct pru_dma *pru_dma);

#endif /* __LINUX_PRU_DMA_H */
