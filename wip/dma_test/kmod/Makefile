ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := dma_test.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

dma-test-00A0.dtbo: dma-test-00A0.dts
	dtc -O dtb -o dma-test-00A0.dtbo -b 0 -@ dma-test-00A0.dts

overlay: dma-test-00A0.dtbo

endif

