obj-m:= drk.o
KDIR := ../../ptxproj/linux/linux-3.18.13
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -f drk.mod.c
	rm -f *.ko
	rm -f *.o
	rm -f .*cmd
	rm -Rf .tmp_versions
	rm -f modules.order
	rm -f Module.symvers
