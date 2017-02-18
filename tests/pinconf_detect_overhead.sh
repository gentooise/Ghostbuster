#!/bin/bash

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then

	# Clean environment
	./clean.sh
	dmesg -C
	sleep 2

	# Measure without defense
	insmod perf.ko
	sleep 2
	insmod attacks/drk6/drk.ko
	sleep 4
	rmmod perf
	rmmod drk

	# Load defense
	./loader.sh 10
	sleep 2

	# Measure with defense
	insmod perf.ko
	sleep 2
	insmod attacks/drk6/drk.ko
	sleep 4
	rmmod perf
	rmmod drk
	rmmod ghostbuster

	echo "Test done!"
else
	echo "Must be root!"
fi
