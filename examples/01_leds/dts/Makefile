pru-dma-leds-00A0.dtbo: pru-dma-leds-00A0.dts
	dtc -Wno-unit_address_vs_reg -O dtb -o pru-dma-leds-00A0.dtbo -b 0 -@ pru-dma-leds-00A0.dts

.PHONY: clean
clean:
	rm -f pru-dma-leds-00A0.dtbo

overlay: pru-dma-leds-00A0.dtbo

