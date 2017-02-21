#!/bin/bash

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then
	echo "Test without defense starting..."

	# Clean environment
	./clean.sh
	dmesg -C
	sleep 5 # Give time to prepare PLC logic upload from PLC software
	echo "Start!"
	sleep 1 # Wait to complete 'echo' without affecting measurements

	# Measure without defense
	insmod perf.ko
	sleep 6 # PLC logic upload should be done here
	rmmod perf

	echo "Test without defense finished"
	sleep 15 # Give time to change PLC configuration again
	echo "Test with defense starting..."

	# Load defense with t = 10
	./loader.sh 10
	sleep 5 # Give time to prepare PLC logic upload from PLC software
	echo "Start!"
	sleep 1 # Wait to complete 'echo' without affecting measurements

	# Measure with defense
	insmod perf.ko
	sleep 6 # PLC logic upload should be done here
	rmmod perf
	rmmod ghostbuster

	echo "Test with defense finished"
	echo "Tests done!"
else
	echo "Must be root!"
fi
