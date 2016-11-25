#!/bin/sh
if [ $# -ne 1 ]
then
	echo "Usage:"
	echo "\t./loader.sh [hijack]"
	echo "Example: "
	echo "\t ./loader.sh 1"
	exit
else
	cHijack=$1
fi

insmod reset.ko hijack=$cHijack
if [ $? = 0 ]
then
	echo "[*] Loading reset module... Done"
else
	echo "[*] Loading reset module... FAILED!"
fi
echo "[*] Done."
