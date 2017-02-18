#!/bin/bash

#number of iterations
n=100
d=0

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then

	# Quirk: attack first time here, otherwise restoring debug register
	# is considered as another attack
	cd attacks/drk8/ 
	./loader.sh &> /dev/null
	cd ../../
	sleep 0.2
	rmmod drk

	# Clean environment
	./clean.sh
	dmesg -C

	for i in `seq 1 $n`
	do

		# Load defense at random time
		for j in `seq 1 5`; do
			./csleep $RANDOM
		done
		./loader.sh 10 # t = 10
		#./loader.sh 5 # t = 5
		#./loader.sh 2 # t = 2

		# Execute attack after random time
		for j in `seq 1 5`; do
			./csleep $RANDOM
		done
		cd attacks/drk8/ 
		./loader.sh &> /dev/null
		cd ../../
		./csleep 200000
		
		rmmod drk
		rmmod ghostbuster
		res=`dmesg -c | grep -i detected`
		if [ -n "$res" ]; then
			((d++))
		fi
		echo -ne "\r$i/$n"
	done
	echo -e "\nTest done!"
	rate=`awk "BEGIN { printf(\"%.2f\n\", (${d}) / (${n} / 100.0)) }"`
	echo "$d attacks detected out of $n: detection rate $rate%"
else
	echo "Must be root!"
fi
