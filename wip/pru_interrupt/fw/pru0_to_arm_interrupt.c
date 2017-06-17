#define PRU0

#include "resource_table.h"
#include "pru_defs.h"

void main(void)
{
	CT_INTC.SICR_bit.STS_CLR_IDX = SYSEV_ARM_TO_PRU0_A;

	while (1) {
		/* Send interrupt to ARM */
		SIGNAL_EVENT(SYSEV_PRU0_TO_ARM_A);

		/* Sleep for one second */
		__delay_cycles(200000000);


		/* Wait until receipt of interrupt on host 0 */
		while (!pru_signal())
			;

		CT_INTC.SICR_bit.STS_CLR_IDX = SYSEV_ARM_TO_PRU0_A;


	}
}

