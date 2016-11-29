Architecture-dependent files
============================

The `arch` directory is intended to include one subdirectory for each supported architecture.
Each subdir is supposed to contain a specific header file (`*.h`) for each SoC model you want to protect against I/O attack.
This structure allows Ghostbuster to be easily extensible to any SoC, and to improve code optimization.


Header files
------------

The name of the implementation header should reflect the name of the SoC model.
Header inclusion is controlled at compile time by putting the model name into the `SOC_MODEL` variable.
Specifying a value for `SOC_MODEL` variable is required to build the project.  

Example:
Given that `arch/arm/BCM2835.h` exists, to build for the `BCM2835` SoC give the following command:  

`make ARCH=arm CROSS_COMPILE=arm-cortexa8-linux-gnueabihf- SOC_MODEL=BCM2835`  

The header file should implement the interface to interact with the portion of I/O memory
which should be protected (see [io_conf.h](../inc/io_conf.h) and the available implementations for details).
