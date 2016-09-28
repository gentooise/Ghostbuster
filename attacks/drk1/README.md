Exercise 1: Pin Control Attack implementation with LKM.
-------------------------------------------------------

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode.
The PLC logic switch the output every 4 seconds, making the LED blink.

This implementation uses debug registers to catch write operations to pin 22.
It simply turns off the LED and then changes the pin mode into input, so the
PLC logic could not switch it anymore and the LED stops blinking.
The Codesys Runtime is not able to detect it.
