PinControlDefense
=================

All the stuff used for my MSc project:

**Pin Control Protection: Protecting Linux Kernel Pin Control Subsystem from Pin Control Attack.**


Abstract
--------

Recently embedded systems have become more and more integrated with all aspects of our lives, and their security concerns have risen as well.
They spread in various fields such as automotive, electronic devices, home automation, manufacturing and mission critical applications. These systems, in particular PLCs
deployed within the context of an Industrial Control System, use Input/Output interfaces to interact with the physical world by means of sensors and actuators.
As demonstrated by a novel kind of attack called Pin Control Attack, one can tamper with the integrity or the availability of legitimate I/O operations,
factually changing how a PLC interacts with the outside world and possibly causing physical damage to people and environment.
In this thesis we design a possible countermeasure to the attack and implement it for Linux Kernel on an ARM-based Programmable Logic Controller,
showing its effectiveness and impact on PLCs which usually have very limited resources and strict timing requirements.


Defense implementation
-------------------------

The defense is called **Ghostbuster**, and the source code can be found in [src](src) directory.


Build modules and stuff
-----------------------

To build modules and applications for both Raspberry Pi and PLC the `arm-cortexa8-linux-gnueabihf` toolchain is needed.
For modules, kernel source directories must be put at proper relative locations (see inside makefiles).
