Exercise 5: Pin Control Attack implementation with LKM.
-------------------------------------------------------

**NOT WORKING**: Needs investigations to figure out why, maybe some problem at hardware level to be fixed.
See: https://learn.sparkfun.com/tutorials/i2c/i2c-at-the-hardware-level.

Method: Debug registers, Pin Multiplexing with I2C bitbang.
Effect: PWM tampering.

Target system: RaspberryPi with Adafruit PWM controller connected via I2C interface.
The I2C data goes through GPIO pin 2 multiplexed as alternate function 0 (ALT0),
which corresponds to I2C master 1 data line (SDA1), while pin 3 is used for the clock signal.
A servo motor is attached to PWM, and the PLC logic uses a sinus wave to make it
rotating from -45° to +45°.

This implementation uses debug registers to catch write operations to pin 2.
It sets pin mode back to GPIO, trying to use them in place of the I2C driver, by bitbanging the pins.
The Codesys Runtime is not able to detect it.
