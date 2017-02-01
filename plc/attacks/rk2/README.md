Pin Control Attack implementation with LKM on real PLC.
-------------------------------------------------------

Method: Pin Configuration or Pin Multiplexing.
Effect: DoS condition on I/O.

Target system: PLC Wago PFC200-8202 with LED attached on external digital I/O module.
The PLC logic switch the output every 500 milliseconds, making the LED blink.

The PLC uses proprietary KBUS driver to communicate with digital I/O module.
KBUS uses pin 97 multiplexed as GPIO and configured as output, to send the "start"
signal to I/O module.
This implementation either multiplex pin 97 on a different controller (e.g. UART),
or configure it as input pin. Both ways lead to a Denial of Service condition:
the KBUS driver cannot write to the GPIO pin anymore.
The e!COCKPIT Runtime is not able to detect the attack.
