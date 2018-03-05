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
