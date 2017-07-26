ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := pru_dma_hcsr04.o
ccflags-y := -I$(src)/../../../kmod/include/

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

endif

