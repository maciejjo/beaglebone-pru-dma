ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m  := pru_dma.o
ccflags-y := -I$(src)/include

else
# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD

endif

