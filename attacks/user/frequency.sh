#!/bin/bash

# Set input button pin as output
gpio -g mode 24 out

for i in `seq 1 1000`
do
	# Write 0 and 1 alternatively, to get an average frequency
	gpio -g write 24 0
	# Wait for next scan cycle
	sleep 0.010
	gpio -g write 24 1
	# Wait for next scan cycle
	sleep 0.010
done
