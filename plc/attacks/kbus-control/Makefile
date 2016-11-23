DIST_DIR = ../../ptxproj/ptxproj-2.4.22/platform-wago-pfcXXX
LINUX_DIR = ../../ptxproj/linux/linux-3.18.13

CC = $(CROSS_COMPILE)gcc
CFLAGS = -O0 -g3  -Wall -I$(DIST_DIR)/sysroot-target/usr/include/ -I$(LINUX_DIR)/include/ -I$(LINUX_DIR)/tools/include/ -I$(LINUX_DIR)/arch/arm/include

LDFLAGS = -L$(DIST_DIR)/sysroot-target/usr/lib/ 


all: ioattack

ioattack: ioattack.o
	$(CC) -o ioattack ioattack.o $(LDFLAGS)


clean:
	-rm -f ioattack
	-rm -f *.o

install:


.PHONY: all install clean

