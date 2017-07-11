# Example 01: blinking LEDs

First example shows the most basic and most visuallly impresive
task any microcontroller can perform: blink some LEDs.

Patterns to be displayed are stored in the kernel buffer and copied with
pru\_dma to PRU memory.

# HW setup

8 LEDs connected to pins 39-46 on P8 header with appropriate resistors.
