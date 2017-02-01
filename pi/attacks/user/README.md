user: Pin Control Attack implementation with user code.
-------------------------------------------------------

Method: Pin Configuration.
Effect: tampering output behaviour.

Target system: RaspberryPi with push button attached on input pin 24,
LED on output pin 22 and Adafruit PWM controller connected via I2C interface.
A servo motor is attached to PWM, and the PLC logic uses a sinus wave to make it
rotating from -45° to +45°. If button is not pressed (1), the motor and the LED
are driven with an interval of 2 seconds. If button is pressed, they are driven
4 times faster.

This attack uses the simple gpio tool provided with Raspberry Pi,
but the same can be done from a C or a Python program.
Two versions of the attack are provided: the first one simply turns the
input pin of the button into output, and set it fixed at value 0, to let the
outputs go at the faster speed; the second version, after configuring the pin
as output, writes 1 and 0 alternatively in order to get an output speed which
is an average between the minimum and the maximum known by the logic.
The Codesys Runtime is not able to detect it, it shows the input provided
by the attack as the real one.
