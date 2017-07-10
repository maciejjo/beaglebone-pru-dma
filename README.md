# Beaglebone PRU DMA support

[![Build Status](https://travis-ci.org/maciejjo/beaglebone-pru-dma.svg?branch=master)](https://travis-ci.org/maciejjo/beaglebone-pru-dma)

This project aims to provide support for bidirectional DMA transfers between
PRU and Cortex-A8 on AM335x SoC. It is developed on BeagleBone Black.

I am developing this project for [GSoC 2017][1] for [beagleboard.org][2].

## Task description

*Goal*: Create a sample program to demonstrate using EDMA on the PRU to transfer
data to and from the main (DDR) memory with a Linux host. Most existing code
utilizes (wastes) the 2nd PRU on the PRUSS for data xfer. Using DMA can allow
the PRU to be used for other purposes.

## Software requirements

This application was developed and tested using following SW revisions:

 - BBB:
   - Beagleboard.org Debian, version 8.8
   - linux: 4.9.30-ti-r38
 - Host:
   - toolchain: arm-linux-gnueabihf gcc linaro 6.3.1-2017.05
   - dtc: 1.4.4
   - PRU CGT: 2.1.5
   - PRU Software Support Package: v5.1.0

To work properly, PRU remoteproc must be enabled in device tree.

## Implementation overview

Implementation of the project is split in two main parts, kernel module and PRU
firmware. For PRU management, remoteproc framework is employed, and for
communication (to pass configuration data for DMA transfers) remote processor
messaging framework (rpmsg) is utilized.

Following scenario is implemented:

 - Driver performs buffer allocation (using kzalloc call) to obtain contiguous
   buffer in memory,
 - Buffer is then mapped using dma_map_single call to ensure the address is in
   DMA able region and caching is turned off for the buffer,
 - Transfer descriptor (struct edma_tx_desc) is created, which contains buffer
   address and size, and EDMA channel parameters,
 - The descriptor is then passed to PRU using rpmsg,
 - PRU configures EDMA based on the transfer descriptor and schedules the
   transfer by writing to EDMA registers,
 - PRU polls periodically for the transfer completion event,
 - once the transfer is complete, driver is interrupted via rpmsg.

## Project structure

 - *kmod* - kernel module, responsible for allocation and mapping of linux-side
   DMA buffer and passing parameters to PRU firmware via rpmsg.

 - firmware - code for PRU software. Receives transfer parameters via rpmsg,
   configures EDMA for transfer and INTC to trigger event on EDMA interrupt.

 - *dts* - device tree overlay needed to run the code. Contains EDMA
   configuration parameters (channel number, PaRAM slot).

 - *examples* - every subdirectory is an example usage of pru-dma, demonstrating
   practical application.

 - *wip* - (work in progress) isolated examples created to test specific
   features (e.g. linux DMA API, PRU-ARM communication), not an essential part
   of the project.

[1]: https://summerofcode.withgoogle.com/projects/#5021339281784832
[2]: https://beagleboard.org/
