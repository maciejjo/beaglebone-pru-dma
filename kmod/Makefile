ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := pru_dma.o

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

endif

