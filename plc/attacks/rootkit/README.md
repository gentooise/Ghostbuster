Pin Control Attack implementation with LKM on real PLC.
-------------------------------------------------------

Method: I/O remap and Pin Multiplexing.
Effect: LED disabling.

Target system: PLC Wago PFC200-8202 with LED attached on external digital I/O module.
The PLC logic switch the output every 2 seconds, making the LED blink.

The PLC uses proprietary KBUS driver to communicate with digital I/O module.
KBUS leverages SPI device inside the AM3517 chip of the PLC.
This implementation remaps the SPI config address (by using ioremap),
and uses the mapped address to multiplex the SPI pins to GPIO,
so the SPI device cannot write to it anymore.
The e!COCKPIT Runtime is not able to detect the attack.
