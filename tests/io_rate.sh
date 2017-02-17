#!/bin/bash

#number of iterations
n=100

# Clear kernel buffer
dmesg -C
if [ $? -eq 0 ]
then

	# Clean environment
	./clean.sh
	dmesg -C

	for i in `seq 1 $n`
	do

		# Load defense
		./loader.sh 10 # t = 10
		#./loader.sh 5 # t = 5
		#./loader.sh 2 # t = 2
		r=`awk "BEGIN { srand(); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		sleep 1
		r=`awk "BEGIN { srand(); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r
		r=`awk "BEGIN { srand(${r}*10000); printf(\"%.5f\n\", rand()) }"`
		sleep $r

		# Attack after random time
		cd attacks/drk9/ 
		./loader.sh &> /dev/null
		cd ../../
		sleep 2.5
		rmmod drk

		rmmod ghostbuster
		echo $i
	done
	echo "Test done!"
	res=`dmesg | grep -i attack | wc -l`
	rate=`awk "BEGIN { printf(\"%.2f\n\", (${res}) / (${n} / 100.0)) }"`
	echo "$res successful attacks out of 1000: detection rate $rate %"
else
	echo "Must be root!"
fi
