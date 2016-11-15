obj-m:= drk.o
KDIR := ../../ptxproj/linux/linux-3.18.13
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm drk.mod.c
	rm *.ko
	rm *.o
	rm modules.order
	rm Module.symvers
