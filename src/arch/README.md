Architecture-dependent files
============================

The `arch` directory is intended to include one subdirectory for each supported architecture,
plus another directory level for each SoC model you want to protect against I/O attack (or set of models whose implementation is the same).
Each model directory is supposed to contain implementation header files (`*.h`) as specified below.
Each architecture directory inside `arch` may be used to contain header files shared between the models of that specific architecture.
This structure allows Ghostbuster to be easily extensible to any SoC, and to improve code optimization.


Header files
------------

The name of each implementation directory inside `arch/<ARCH>/` should reflect the name of the SoC model.
Header inclusion is controlled at compile time by putting the model name into the `SOC_MODEL` variable.
Specifying a value for `ARCH` and `SOC_MODEL` variables is required to build the project.
When both ARCH and SOC_MODEL are specified, both `arch/<ARCH>/` and `arch/<ARCH>/<SOC_MODEL>/` directories are looked for header files inclusion.  

Example:
Given that `arch/arm/BCM2835/` exists, to build for the `BCM2835` SoC give the following command:  

`make ARCH=arm CROSS_COMPILE=arm-cortexa8-linux-gnueabihf- SOC_MODEL=BCM2835`  

The header files should implement at least:
- the interface for interacting with the portion of I/O memory which should be protected (see [io_conf.h](../inc/io_conf.h));
- the interface for monitoring debug registers, which can be used by attackers (see [dr_conf.h](../inc/dr_conf.h));
- the interface for hooking and restoring the mapping syscalls and the exit callback (see [map_conf.h](../inc/map_conf.h)).  

Where these implementations should be placed (either in the architecture or in the SoC directory) is up to the implementation itself.
As a general guideline, more common code as possible should be put directly inside the `arch` directory, leaving only the minimum
necessary code into the specific SoC model directory.  

See also the available implementation for further details.
