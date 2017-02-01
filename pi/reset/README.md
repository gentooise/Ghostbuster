Utility to reset state of hijacked pins.
----------------------------------------

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode
and Adafruit PWM Controller connected via I2C interface.
The I2C data goes through GPIO pin 2 multiplexed as alternate function 0 (ALT0),
which corresponds to I2C master 1 data line (SDA1).

Reset pin 2 as output and pin 22 as alternate function 0 (undo the attack).
