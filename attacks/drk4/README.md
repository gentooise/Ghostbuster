Exercise 4: Pin Control Attack implementation with LKM.
-------------------------------------------------------

Method: Debug registers and Pin Configuration.
Effect: PWM disabling.

Target system: RaspberryPi with Adafruit PWM controller connected via I2C interface.
The I2C data goes through GPIO pin 2 multiplexed as alternate function 0 (ALT0),
which corresponds to I2C master 1 data line (SDA1).
A servo motor is attached to PWM, and the PLC logic uses a sinus wave to make it
rotating from -45° to +45°.

This implementation uses debug registers to catch write operations to pin 2.
It simply changes the pin mode into input, so the PLC logic could not write it
anymore and the servo motor stops moving.
The Codesys Runtime is not able to detect it.
