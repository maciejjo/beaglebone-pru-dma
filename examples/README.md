# Beaglebone PRU DMA examples

Every subdirectory contains one example application.

 - *01\_leds* - pattern sequence is defined in kernel driver, transfered to PRU
   via DMA, and displayed on LEDs connected to PRU1 pins 0-7 (P8.39 - P8.46 on
   BBB header).
