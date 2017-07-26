/* Copyright (c) 2015, Dimitar Dimitrov
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <pru_ctrl.h>

#include "hc-sr04.h"

#define PRU_OCP_RATE_HZ		(200 * 1000 * 1000)

#define TRIG_PULSE_US		10

#define TRIG_PIN		8
#define ECHO_PIN		10

volatile register uint32_t __R30;
volatile register uint32_t __R31;

int hc_sr04_measure_pulse(void)
{
	bool echo, timeout;

	/* pulse the trigger for 10us */
	__R30 |= (1 << TRIG_PIN);
	__delay_cycles(TRIG_PULSE_US * (PRU_OCP_RATE_HZ / 1000000));
	__R30 &= ~(1 << TRIG_PIN);

	/* Enable counter */
	PRU1_CTRL.CYCLE = 0;
	PRU1_CTRL.CTRL_bit.CTR_EN = 1;

	/* wait for ECHO to get high */
	do {
		echo = !!(__R31 & (1u << ECHO_PIN));
		timeout = PRU1_CTRL.CYCLE > PRU_OCP_RATE_HZ;
	} while (!echo && !timeout);

	PRU1_CTRL.CTRL_bit.CTR_EN = 0;

	if (timeout)
		return -1;

	/* Restart the counter */
	PRU1_CTRL.CYCLE = 0;
	PRU1_CTRL.CTRL_bit.CTR_EN = 1;

	/* measure the "high" pulse length */
	do {
		echo = !!(__R31 & (1u << ECHO_PIN));
		timeout = PRU1_CTRL.CYCLE > PRU_OCP_RATE_HZ;
	} while (echo && !timeout);

	PRU1_CTRL.CTRL_bit.CTR_EN = 0;

	if (timeout)
		return -1;

	uint64_t cycles = PRU1_CTRL.CYCLE;

	return cycles / ((uint64_t)PRU_OCP_RATE_HZ / 1000000);
}
