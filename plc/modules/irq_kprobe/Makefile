obj-m:= drk.o
KDIR := ../../ptxproj/linux/linux-3.18.13
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
