Exercise 2: Pin Control Attack implementation with LKM.
-------------------------------------------------------

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode.
The PLC logic switch the output every 4 seconds, making the LED blink.

This implementation remaps the GPIO base address (by using ioremap),
and uses the new address instead of the Codesys one. The old address
remains valid, and the logic can still write to it and toggle the LED.
Instead of using debug registers, the write operations to pin 22
are catched by manually polling its value.
The attack switches the LED at the specified interval, factually changing the normal behavior.
In this scenario is not possible to set pin 22 to input mode because this will turn off the LED.
Therefore, only intervals less than 4 seconds are supported for now, and both PLC logic and drk2
will keep writing to pin 22, so race conditions might occur.
The Codesys Runtime is not able to detect the attack.
