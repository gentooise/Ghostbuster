#!/bin/bash

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then
	# Measure without defense
	insmod perf.ko
	sleep 12
	rmmod perf

	# Load defense
	./loader.sh &> /dev/null
	sleep 5

	# Measure with defense
	insmod perf.ko
	sleep 12
	rmmod perf

	rmmod ghostbuster
	sleep 5

	echo "Test done!"
else
	echo "Must be root!"
fi
