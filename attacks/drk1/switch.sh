#!/bin/sh

# Switch between the two versions of drk1

if [ -f drk_wide.c ]; then
	mv drk.c drk_user.c
	mv drk_wide.c drk.c
	mv loader.sh loader_user.sh
	mv loader_wide.sh loader.sh
elif [ -f drk_user.c ]; then
	mv drk.c drk_wide.c
	mv drk_user.c drk.c
	mv loader.sh loader_wide.sh
	mv loader_user.sh loader.sh
fi
