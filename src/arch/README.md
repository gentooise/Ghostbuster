Architecture-dependent files
============================

The `arch` directory is intended to include one subdirectory for each supported architecture,
plus another directory level for each SoC model you want to protect against I/O attack (or set of models whose implementation is the same).
Each model directory is supposed to contain implementation header files (`*.h`) as specified below.
This structure allows Ghostbuster to be easily extensible to any SoC, and to improve code optimization.


Header files
------------

The name of each implementation directory inside `arch/<arch_name>/` should reflect the name of the SoC model.
Header inclusion is controlled at compile time by putting the model name into the `SOC_MODEL` variable.
Specifying a value for `SOC_MODEL` variable is required to build the project.  

Example:
Given that `arch/arm/BCM2835/` exists, to build for the `BCM2835` SoC give the following command:  

`make ARCH=arm CROSS_COMPILE=arm-cortexa8-linux-gnueabihf- SOC_MODEL=BCM2835`  

The header files contained in the SoC model directory should implement at least:
- the interface to interact with the portion of I/O memory which should be protected (see [io_conf.h](../inc/io_conf.h));
- the interface for monitoring debug registers, which can be used by attackers (see [dr_conf.h](../inc/dr_conf.h)).

See also the available implementation for further details.
