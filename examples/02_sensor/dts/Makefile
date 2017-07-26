pru-dma-hcsr04-00A0.dtbo: pru-dma-hcsr04-00A0.dts
	dtc -Wno-unit_address_vs_reg -O dtb -o pru-dma-hcsr04-00A0.dtbo -b 0 -@ pru-dma-hcsr04-00A0.dts

.PHONY: clean
clean:
	rm -f pru-dma-hcsr04-00A0.dtbo

overlay: pru-dma-hcsr04-00A0.dtbo

