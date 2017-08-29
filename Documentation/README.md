# Beaglebone PRU DMA Documentation

This file contains documentation explaining how to use BeagleBone PRU DMA
project. To incorporate it in a project there are 2 main parts that must be
performed: PRU firmware set-up and kernel module set-up. They are described in
following sections.

## Set-up on PRU firmware side

First, pru\_dma\_lib must be compiled. It depends on PRU Code Geneation Tools
and PRU Software Support Package, which can be downloaded from TI website, for
example for x86 host:

```
    wget http://software-dl.ti.com/codegen/esd/cgt_public_sw/PRU/2.1.5/ti_cgt_pru_2.1.5_linux_installer_x86.bin
    wget https://git.ti.com/pru-software-support-package/pru-software-support-package/archive-tarball/v5.1.0
```

CGT should be installed by executing the bin file, SSP archive should be
extracted. Then, environmental variables pointing to installation directories
for this componnents should be set, for example:

```
    export PRU_CGT=${HOME}/devel/ti-cgt/ti-cgt-pru_2.1.5
    export PRU_SSP=${HOME}/devel/pru-software-support-package
```

To use PRU DMA, PRU software package must be patched with a patch supplied in
pru\_dma repository. To apply it go to PRU\_SSP directory and issue following:

```
   $ patch -p1 < ${PATH_TO_PRU_DMA_REPO}/pru-swpkg-patch/0001-Add-PRU-DMA-custom-resource-to-resource-table.patch
```

Then, go to firmware/lib/pru\_dma\_lib directory and build the library by
issuing 'make' command. After succesful build, library can be used in PRU
applications.

## Set-up on kernel module side - patching  kernel

To use PRU DMA, we need to apply patches to kernel. To do that, we need kernel
tree. Easiest way to obtain it is using Robert Nelson's repository. To get it:

```
   $ git clone https://github.com/RobertCNelson/ti-linux-kernel-dev
```

To add patches requried for PRU DMA:

```
   $ cd ti-linux-kernel-dev
   $  git checkout 4.9.36-ti-r46
   $ git am ${PATH_TO_PRU_DMA_REPO}/kernel-patch/rcn-ee-tree-patch/0001-Add-pru-dma-patches.patch
```

After that kernel can be build using ./build\_deb.sh command, which will
generate package file for kernel. This package needs to be copied to BBB and
installed by issuing

```
   $ dpkg -i linux-image-4.9.36-ti-r46_1cross_armhf.deb
```

After installation of kernel bbb needs to be rebooted.

## Building example applications

When previous steps are completed, example applications can be build. Each
example consists of device tree overlay, kernel driver and PRU firmware. Each
part must be build for application to work.

To build e.g. 01\_leds application, first let's build kernel module:

```
   $ cd examples/01_leds/kmod/
   $ export KDIR=/home/maciejjo/devel/rcn-ee/ti-linux-kernel-dev/KERNEL
   $ export ARCH=arm
   $ export CROSS_COMPILE="ccache /home/maciejjo/devel/rcn-ee/ti-linux-kernel-dev/dl/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-"
   $ make -C $KDIR M=$PWD clean && make -C $KDIR M=$PWD
```

Now we can use sshfs to mount beaglebone root filesystem and install module to
it:
   
```
   $ sshfs ~/mnt/bbb/ root@192.168.111.7:/
   $ export INSTALL_MOD_PATH=~/mnt/bbb
   $ make -C $KDIR M=$PWD modules_install
```

Next, we need PRU firmware, which needs to be compiled using PRU toolchain and
previously patched PRU Software package:

```
  $ cd examples/01_leds/firmware
  $ export PRU_CGT=${HOME}/devel/ti-cgt/ti-cgt-pru_2.1.5
  $ export PRU_SSP=${HOME}/devel/pru-software-support-package
  $ make clean && make
```

After successful compilation, firmware can be copied to BBB:

```
  $ cp gen/firmware.out ~/mnt/bbb/lib/firmware/am335x-pru1-fw
```
  
Last part needed to work is the device tree overlay, to compile it dtc compiler
needs to be installed.

```
  $ cd examples/01_leds/dts
  $ make clean && make && cp pru-dma-leds-00A0.dtbo ~/mnt/bbb/lib/firmware/
```

We need to enable the overlay in /boot/uEnv.txt file by adding following line:

```
  uboot_overlay_addr0=/lib/firmware/pru-dma-leds-00A0.dtbo
```

After reboot, pru\_dma\_leds node should show up in device tree, and after
loading the pru\_dma\_leds driver sysfs file: 

```
/sys/bus/platform/devices/4a300000.pruss:pru_dma_leds/tx
```

should be available. Writing anything to TX file will start DMA transfer from
kernel buffer to PRU, which will then present content of buffer on 8 LEDs which
should be attached to pins 37-46 on P8 header.

Other examples are build exactly the same way. Each example is further
described in a README file in it's own directory.

## Programming interface

To create own applications using PRU DMA there are two parts that need to be taken care of.

### Firmware side

#### Resource table

PRU DMA parameters are supplied to kernel and firmware via resource table which
is part of firmware binary blob.  It has form of header file included in main
application file. To use PRU DMA, following section needs to be added as a
resource table structure:
 
	struct fw_rsc_custom pru_dmas;

Here is how example entry for PRU DMA looks in code:

```
	/* PRU DMA entry */
	{
		TYPE_CUSTOM, TYPE_PRU_DMA,
		sizeof(struct fw_rsc_custom_dma_ch),
		.rsc.pru_dma = {
			// Version number
			0x0000,
			1, // Number of PRU DMA buffers
			{
				{0,  // Buffer address, to be filled by kernel
				100, // Size of the buffer
				12,  // EDMA channel
				200, // PaRAM slot
				0},  // completion notification
			},
		},
	},
```

DMA buffers are represented by flexible size array. Each buffer needs to have
EDMA channel which must not be used by other device, PaRAM slot, which must be
excluded from use by kernel (it will be explained in Kernel side
configuration), and flag determining whether interrupt is needed on completed
transfer (1 means interrupt is enabled).

## PRU DMA API

With prepared resource table, we can use following functions to control DMA operations on firmware side:


```
void pru_dma_init(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num);
```


This function is used to initalize EDMA controler for new buffer. It must be
called before performing any operations. It takes as parameters pointer to
dma\_data structure which it will fill with data for user, direction parameter
which can be either PRU\_DMA\_DIR\_ARM\_TO\_PRU or PRU\_DMA\_DIR\_PRU\_TO\_ARM, pointer
to DMA node in resource table, and channel number which we want to configure
(channels are counted from 0 in order as defined in resource table).

```
void pru_dma_set_dir(struct pru_dma_data *dma_data,
			enum pru_dma_direction dir,
			struct fw_rsc_custom_dma_ch *pru_dmas,
			int chan_num);
```

This function has identical parameters as pru\_dma\_init and is used to change
direction of already configured channel.

```
void pru_dma_wait_host();
```

This function is used to wait for signal from kernel side. It can be used to
signalize from kernel module to PRU that data is ready to be transfered. It
will wait for INTC signal.

```
void pru_dma_trigger();
```

This function is used to start DMA transfer on channel earlier configured with
pru\_dma\_init(). It will trigger EDMA to start transfering data.

```
void pru_dma_wait();
```

This function will wait until DMA transfer has completed. pru\_dma\_trigger
needs to be called first.

```
struct pru_dma_data {
	uint32_t src;
	uint32_t dst;
	uint32_t size;
};
```

To access data from buffers, struct pru\_dma\_data is used. It has src and dst
buffers as well as size. It must be defined and passed to pru\_dma\_init which
will fill the data.


### Kernel side

### Kernel module API

On kernel side a module needs to be prepared which will make use of functions
exported from pru\_dma driver. Following functions are available:

```
struct pru_dma *pru_dma_get(char *chan_name);
```

This function is used to get pointer to the pru\_dma struct. This pointer is
used in all other pru\_dma functions, so this must be called first in the driver.

```
void pru_dma_put(struct pru_dma *pru_dma);
```

This fucntion must be called to signalize pru\_dma will no longer be used by the
driver. It must be called when removing the driver to take care of reference
counting.

```
uint32_t *pru_dma_get_buffer(struct pru_dma *pru_dma, int buf_num);
```

This function is used to obtain address of a kernel buffer which will be used
for DMA operations on PRU. It takes buffer number as parameter. Buffers are
numbered as in resource table, first buffer number is 0.

```
uint32_t pru_dma_get_buffer_size(struct pru_dma *pru_dma, int buf_num);
```

This function returns size of the kernel buffer which will be used for DMA
operations on PRU. It takes buffer number as parameter. Buffers are numbered as
in resource table, first buffer number is 0.


```
int pru_dma_map_buffer(struct pru_dma *pru_dma, int buf_num);
```

pru\_dma\_map\_buffer is used to map buffer to DMA able address, but
all buffers are mapped before PRU boots to enable passing all kernel
buffer adresses via resource table, so this function doesn't normally need to
be used.

```
void pru_dma_unmap_buffer(struct pru_dma *pru_dma, int buf_num);
```

Unmap buffer will delete the mapping to DMA able address. All buffers will
be unmapped when pru\_dma device will be removed, so this function doesn't
normally need to be called.

```
int pru_dma_tx_trigger(struct pru_dma *pru_dma, int buf_num);
```

This function is used to trigger DMA operation on PRU. It can be used to inform
PRU that data in buffer is ready for transfer. In PRU firmware
pru\_dma\_wait\_host() should be used to wait for the signal.


```
int pru_dma_tx_completion_wait(struct pru_dma *pru_dma, int buf_num);
```

This function will wait for signal from PRU that DMA transfer has finished.
Given buffer must have notification completion flag set in resource table,
otherwise the function will return -1 instead of waiting.

### DTS configuration

pru\_dma requires additional node in device tree which must have following format:

```
	pru_dma {
		compatible = "pru-dma";
		interrupt-parent = <&pruss_intc>;
		interrupts = <22>, <23>;
		interrupt-names = "irq_from_pru", "irq_to_pru";
		pruss = <&pruss>;
	};
```

Other interrupt numbers can be used, but must be reflected in resource table configuration.

## Example applications

To see how all parts fit together, there are example applications prepared. Each application has
it's own descripiton in main directory of the example. Each consists of kernel
module, PRU firmware and appropriate dts file.

