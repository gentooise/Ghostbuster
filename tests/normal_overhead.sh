#!/bin/bash

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then

	# Clean environment
	./clean.sh
	dmesg -C
	sleep 1

	# Measure without defense
	insmod perf.ko
	sleep 6
	rmmod perf

	# Load defense with t = 10
	./loader.sh 10
	sleep 2

	# Measure with defense
	insmod perf.ko
	sleep 6
	rmmod perf
	rmmod ghostbuster
	sleep 2

	# Load defense with t = 5
	./loader.sh 5
	sleep 2

	# Measure with defense
	insmod perf.ko
	sleep 6
	rmmod perf
	rmmod ghostbuster
	sleep 2

	# Load defense with t = 2
	./loader.sh 2
	sleep 2

	# Measure with defense
	insmod perf.ko
	sleep 6
	rmmod perf
	rmmod ghostbuster

	echo "Test done!"
else
	echo "Must be root!"
fi
