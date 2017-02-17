drk8: Debug Register Attack implementation with LKM.
----------------------------------------------------

*Simplified version used for detection rate tests.*

Method: Debug registers.
Effect: None.

Target system: RaspberryPi.

This implementation uses debug registers to catch Codesys read operation,
to measure the DR monitor detection rate. If the modification is detected
before the watchpoint is triggered, the attack fails. Otherwise, if the watchpoint
is triggered and removed without being detected, the attack is considered successful.
The outcome depends on the relative timing between attack, monitor and PLC scan cycle.
