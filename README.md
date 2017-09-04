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
firmware. PRU firmware and kernel module communicate via interrupts.

Following scenario is implemented:

 - During loading procedure of PRU firmware, driver performs buffer allocation
   (using kzalloc call) to obtain contiguous buffer in memory based on requirements
   specified in PRU DMA entry in resource table,
 - Buffer is then mapped using dma\_map\_single call to ensure the address is in
   DMA able region and caching is turned off for the buffer,
 - Resource table is modified to include address of the mapped buffer,
 - After boot, PRU configures EDMA based on parameters passed in resource table
   (buffer address and size, EDMA channel, PaRAM slot)
 - PRU schedules the transfer by writing to EDMA registers,
 - PRU polls periodically for the transfer completion event,
 - Once the transfer is complete, interrupt is optionally sent to the driver
   (configurable via option in resource table entry)
 - In interrupt routine driver signals finished transfer using Linux completion
   mechanism

## Project structure
 
 - *firmware* - code for PRU software. Receives transfer parameters via resource table,
   configures EDMA for transfer and INTC to trigger event on EDMA interrupt

 - *kernel-patch* - kernel-side implementation of the project, contains patches
   to ti-linux-kernel-dev repository (based on 4.9.36-ti-r46 release)
 
 - *pru-swpkg-patch* - contains patch that needs to be applied to pru software support
   package (based on v5.1.0 release)

 - *dts* - basic device tree overlay needed to load the module. More concrete dt
   overlays exist for each example

 - *examples* - every subdirectory is an example usage of pru-dma, demonstrating
   practical application, consisting of PRU firmware, kernel module and dt overlay

 - *Documentation* - contains detailed description of the project, build instructions
   and description of API for PRU firmware and kernel modules

 - *wip* - (work in progress) isolated examples created to test specific
   features (e.g. linux DMA API, PRU-ARM communication), not an essential part
   of the project.

[1]: https://summerofcode.withgoogle.com/projects/#5021339281784832
[2]: https://beagleboard.org/
