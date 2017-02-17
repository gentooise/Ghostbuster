drk9: Debug Register Attack implementation with LKM.
----------------------------------------------------

*Simplified version used for detection rate tests.*

Method: Pin multiplexing with manual synchronization.
Effect: None.

Target system: RaspberryPi.

This implementation polls I/O value on the output pin to catch Codesys write operation,
then it executes pin multiplexing on the next I/O operation. The modification is made
(on average) 0.5 ms before the next I/O, and it is restored 0.5 ms after I/O.
Used for measuring the I/O monitor detection rate.
The outcome depends on the relative timing between attack, monitor and PLC scan cycle.
