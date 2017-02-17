#!/bin/bash

clean=`lsmod | grep snd_bcm2835`

if [ -n "$clean" ]; then
	rmmod snd_bcm2835
	rmmod snd_pcm
	rmmod snd_timer
	rmmod snd
	rmmod can_raw
	rmmod can
	rmmod w1_therm
	rmmod w1_gpio
	rmmod bcm2835_gpiomem
	rmmod bcm2835_wdt
	rmmod uio_pdrv_genirq
	rmmod uio
	rmmod wire
	rmmod cn
fi
