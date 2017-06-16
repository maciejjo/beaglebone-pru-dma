#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;

void main(void)
{
	while (1) {
		/* Send interrupt to ARM */
		__R31 =  32 | (SYSEV_PRU0_TO_ARM_A - 16);
		/* Sleep for one second */
		__delay_cycles(100000000);
	}
}

