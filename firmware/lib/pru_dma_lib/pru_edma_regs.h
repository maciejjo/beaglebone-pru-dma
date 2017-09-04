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

/* EDMA PARAM registers */
typedef struct {
	uint32_t sam		: 1;
	uint32_t dam		: 1;
	uint32_t syncdim	: 1;
	uint32_t static_set	: 1;
	uint32_t		: 4;
	uint32_t fwid		: 3;
	uint32_t tccmode	: 1;
	uint32_t tcc		: 6;
	uint32_t		: 2;
	uint32_t tcinten	: 1;
	uint32_t itcinten	: 1;
	uint32_t tcchen		: 1;
	uint32_t itcchen	: 1;
	uint32_t privid		: 4;
	uint32_t		: 3;
	uint32_t priv		: 1;
} edmaParamOpt;

typedef struct {
	uint32_t acnt		: 16;
	uint32_t bcnt		: 16;
} edmaParamABcnt;

typedef struct {
	uint32_t srcbidx	: 16;
	uint32_t dstbidx	: 16;
} edmaParamBidx;

typedef struct {
	uint32_t link		: 16;
	uint32_t bcntrld	: 16;
} edmaParamLnkRld;

typedef struct {
	uint32_t srccidx	: 16;
	uint32_t dstcidx	: 16;
} edmaParamCidx;

typedef struct {
	uint32_t ccnt		: 16;
	uint32_t		: 16;
} edmaParamCcnt;

typedef struct {
	edmaParamOpt	opt;
	uint32_t	src;
	edmaParamABcnt	abcnt;
	uint32_t	dst;
	edmaParamBidx	bidx;
	edmaParamLnkRld	lnkrld;
	edmaParamCidx	cidx;
	edmaParamCcnt	ccnt;
} edmaParam;

/* EDMA Channel Registers */
#define DMAQNUM0	(0x0240 / 4)
#define DMAQNUM1	(0x0244 / 4)
#define DCHMAP_12	(0x0130 / 4)
#define QUEPRI		(0x0284 / 4)
#define EMR		(0x0300 / 4)
#define EMCR		(0x0307 / 4)
#define EMCRH		(0x030C / 4)
#define QEMCR		(0x0314 / 4)
#define CCERRCLR	(0x031C / 4)
#define DRAE0		(0x0340 / 4)
#define DRAE1		(0x0348 / 4)
#define DRAE2		(0x0350 / 4)
#define DRAE3		(0x0358 / 4)
#define QWMTHRA		(0x0620 / 4)
#define GLOBAL_ESR	(0x1010 / 4)
#define GLOBAL_ESRH	(0x1014 / 4)
#define GLOBAL_EECR	(0x1028 / 4)
#define GLOBAL_EECRH	(0x102C / 4)
#define GLOBAL_SECR	(0x1040 / 4)
#define GLOBAL_SECRH	(0x1044 / 4)
#define GLOBAL_IESR	(0x1060 / 4)
#define GLOBAL_IESRH	(0x1064 / 4)
#define GLOBAL_ICR	(0x1070 / 4)
#define GLOBAL_ICRH	(0x1074 / 4)

/* EDMA Shadow Region 1 */
#define ESR		(0x2210 / 4)
#define ESRH		(0x2214 / 4)
#define EESR		(0x1030 / 4)
#define EECR		(0x2228 / 4)
#define EECRH		(0x222C / 4)
#define SECR		(0x2240 / 4)
#define SECRH		(0x2244 / 4)
#define IPR		(0x2268 / 4)
#define IPRH		(0x226C / 4)
#define ICR		(0x2270 / 4)
#define ICRH		(0x2274 / 4)
#define IESR		(0x2260 / 4)
#define IESRH		(0x2264 / 4)
#define IEVAL		(0x2278 / 4)
#define IECR		(0x2258 / 4)
#define IECRH		(0x225C / 4)

/* EDMA PARAM registers */
#define PARAM_OFFSET	(0x4000 / 4)
#define OPT		0x00
#define SRC		0x04
#define ACNT		0x190
#define BCNT		0x1
#define DST		0x0C
#define SRC_DST_BIDX	0x10
#define LINK_BCNTRLD	0x14
#define SRC_DST_CIDX	0x18
#define CCNT		0x1

#define CHAN_MASK(c) (1 << c)

