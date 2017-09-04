/*
 * Copyright (C) 2017 Maciej Sobkowski <maciej@sobkow.ski>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef PRU_DEFS_H
#define PRU_DEFS_H

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>

#define CHECK_EVT(e) ((e < 32) ? (CT_INTC.SECR0 & (1U << e)) : (CT_INTC.SECR1 & (1U << (e - 32))))
#define CLEAR_EVT(e) ((e < 32) ? (CT_INTC.SECR0 = (1U << e)) : (CT_INTC.SECR1 = (1U << (e - 32))))

#define SIGNAL_EVENT(x) \
	__R31 = (1 << 5) | ((x) - 16); \

#define PRU_SHMEM_OFFSET (0x4A310000)

volatile register uint32_t __R30;
volatile register uint32_t __R31;

#endif /* PRU_DEFS_H */
