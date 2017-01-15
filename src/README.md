Ghostbuster
===========

Protecting Linux Kernel Pin Control Subsystem from Pin Control Attack
---------------------------------------------------------------------

Ghostbuster is an implementation of Pin Control Protection as a Loadable Kernel Module.

The module is divided into the following parts:

- [x] Configuration monitor
- [x] Debug registers monitor
- [x] Memory mapping monitor

Ghostbuster is designed to be highly configurable (see the [Makefile](Makefile)) and architecture-independent (see [arch/README.md](arch/README.md)).
