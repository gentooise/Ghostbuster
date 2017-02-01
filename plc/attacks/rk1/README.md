Pin Control Attack implementation with LKM on real PLC.
-------------------------------------------------------

Method: IRQ Configuration.
Effect: LED disabling.

Target system: PLC Wago PFC200-8202 with LED attached on external digital I/O module.
The PLC logic switch the output every 500 milliseconds, making the LED blink.

The PLC uses proprietary KBUS driver to communicate with digital I/O module.
KBUS leverages SPI device inside the AM3517 chip of the PLC.
This implementation changes the interrupt configuration used by KBUS,
by disabling the interrupt line #216 used for data transfers between PLC and I/O module.
The e!COCKPIT Runtime is not able to detect the attack.
