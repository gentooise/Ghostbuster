Exercise 2: Pin Control Attack implementation with LKM.
-------------------------------------------------------

Method: Debug registers.
Effect: LED blinking at specified interval.

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode.
The PLC logic switch the output every 4 seconds, making the LED blink.

This implementation uses debug registers to catch write operations to pin 22.
It switches the LED at the specified interval, factually changing the normal behavior.
In this scenario is not possible to set pin 22 to input mode because this will turn off the LED.
Therefore, only intervals less than 4 seconds are supported for now, and both PLC logic and drk2
will keep writing to pin 22, so race conditions might occur.
The Codesys Runtime is not able to detect the attack.
