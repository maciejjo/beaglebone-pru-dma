# Beaglebone PRU DMA support

This project aims to provide support for bidirectional DMA transfers between
PRU and Cortex-A8 on AM335x SoC. It is developed on BeagleBone Black.

I am developing this project for [GSoC 2017][1] for [beagleboard.org][2].

## Task description

*Goal*: Create a sample program to demostrate using EDMA on the PRU to transfer
data to and from the main (DDR) memory with a Linux host. Most existing code
utilizes (wastes) the 2nd PRU on the PRUSS for data xfer. Using DMA can allow
the PRU to be used for other purposes.

## Project structure

wip - (work in progress) contains isolated examples created to test specific
feature.

kmod - contains kernel part of the project

firmware - contains PRU part of the project

[1]: https://summerofcode.withgoogle.com/projects/#5021339281784832
[2]: https://beagleboard.org/
