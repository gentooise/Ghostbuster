drk1: Pin Control Attack implementation with LKM.
-------------------------------------------------------

Method: Debug registers and Pin Configuration.
Effect: LED disabling (DoS).

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode.
The PLC logic switch the output every 4 seconds, making the LED blink.

This implementation uses debug registers to catch Codesys write operation
and get access to its address space, then changes the pin mode into input,
so the PLC logic could not switch it anymore and the LED stops blinking.
The Codesys Runtime is not able to detect it.
