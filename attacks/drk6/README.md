drk6: Pin Control Attack implementation with LKM.
-------------------------------------------------------

*Simplified version used for performance tests.*

Method: GPIO base remap, Pin Configuration.
Effect: Input pin disabled, logic changed (LED stops blinking).

Target system: RaspberryPi with LED attached on GPIO pin 22 in output mode,
and button attached on GPIO pin 24 in input mode.
The PLC logic toggles the output every 4 seconds only if the input button
is high (not pressed).

This implementation remaps the GPIO base address (by using ioremap).
The attack changes the configuration of pin 24 from input to output,
factually changing the normal behavior.
The Codesys Runtime is not able to detect the attack.
