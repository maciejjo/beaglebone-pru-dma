ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := pru_dma_test.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

pru-dma-test-00A0.dtbo: pru-dma-test-00A0.dts
	dtc -O dtb -o pru-dma-test-00A0.dtbo -b 0 -@ pru-dma-test-00A0.dts

overlay: pru-dma-test-00A0.dtbo

endif

