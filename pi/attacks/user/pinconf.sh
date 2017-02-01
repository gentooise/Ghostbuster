#!/bin/bash

# Set input button pin as output
gpio -g mode 24 out
# Write 0 to set the maximum speed
gpio -g write 24 0
