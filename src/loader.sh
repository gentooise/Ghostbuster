#!/bin/sh
ppid=`pidof codesyscontrol.bin | cut -d' ' -f 1`
vaddr=`cat /proc/$ppid/maps | grep /dev/mem | cut -d'-' -f 1 | cut -d' ' -f 1`

insmod ghostbuster.ko p_pid=$ppid vaddr_base=0x$vaddr
if [ $? = 0 ]
then
	echo "Loading Ghostbuster... done!"
else
	echo "Loading Ghostbuster... failed!"
fi
