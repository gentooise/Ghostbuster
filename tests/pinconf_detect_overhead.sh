#!/bin/bash

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then

	# Clean environment
	./clean.sh
	dmesg -C
	
	# Measure without defense
	insmod perf.ko
	sleep 5
	insmod attacks/drk6/drk.ko
	sleep 7
	rmmod perf

	rmmod drk

	# Load defense
	./loader.sh 10
	sleep 5

	# Measure with defense
	insmod perf.ko
	sleep 5
	insmod attacks/drk6/drk.ko
	sleep 7
	rmmod perf

	rmmod drk
	rmmod ghostbuster
	sleep 5

	echo "Test done!"
else
	echo "Must be root!"
fi
