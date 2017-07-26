# Example 02: Transfer data from sensor

Second example uses HCSR04 ultrasonic distance meter. Code for the sensor
itself and HW setup were copied from [this project][1]. Buffer in PRU memory is
filled with data from sensor, and than transfered using DMA. On linux side, in
sysfs 2 files are created, one that when written to starts the DMA transfer,
and another that contains contents of the buffer copied from DMA (sensor readouts).

[1]: https://github.com/dinuxbg/pru-gcc-examples/tree/master/hc-sr04-range-sensor
