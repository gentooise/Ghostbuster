#!/bin/bash

#
# Extract Linux patched sources for WAGO PFC200
# from ptxproj-2.4.22 board support package.
#
# Run this script in the same directory containing ptxproj-2.4.22.
#

if [ ! -d ptxproj-2.4.22 ]; then
	echo "directory ptxproj-2.4.22 not available"
	exit
fi

confirm () {
    # call with a prompt string or use a default
    read -r -p "${1:-Continue? [y/N]} " response
    case $response in
        [yY][eE][sS]|[yY]) 
            true
            ;;
        *)
            false
            ;;
    esac
}

if [ -e linux ]; then
	echo "linux directory already exists"
	confirm || exit
	rm -R linux
fi

mkdir -p linux

echo "Retrieving linux archive, configuration and patches..."
cp ptxproj-2.4.22/src/linux-3.18.13.tar.xz linux/
cp ptxproj-2.4.22/configs/wago-pfcXXX/kernelconfig-3.18.13 linux/
cp -R ptxproj-2.4.22/configs/wago-pfcXXX/patches/linux-3.18.13/generic linux/patches

echo "Extracting linux archive..."
cd linux
tar xf linux-3.18.13.tar.xz

echo "Configuring..."
cp kernelconfig-3.18.13 linux-3.18.13/.config

echo "Applying patches..."
cd linux-3.18.13
for i in $(ls ../patches/*.patch); do
	patch -p1 < $i &> /dev/null
done


echo "Done!"
