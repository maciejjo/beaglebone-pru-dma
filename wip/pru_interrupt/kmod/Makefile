ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := pru_irq.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

pru-irq-00A0.dtbo: pru-irq-00A0.dts
	dtc -O dtb -o pru-irq-00A0.dtbo -b 0 -@ pru-irq-00A0.dts

overlay: pru-irq-00A0.dtbo

endif

