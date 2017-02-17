#!/bin/sh

if [ $# -ne 1 ]; then
	echo "Usage ./loader.sh <interval>"
	echo -e "\twhere <interval> is 10, 5 or 2 (ms)"
	exit
fi

ppid=`pidof codesyscontrol.bin | cut -d' ' -f 1`
vaddr=`cat /proc/$ppid/maps | grep /dev/mem | cut -d'-' -f 1 | cut -d' ' -f 1`

insmod ghostbuster${1}.ko p_pid=$ppid vaddr_base=0x$vaddr
if [ $? -ne 0 ]; then
	echo "Loading Ghostbuster... failed!"
fi
