obj-m += ghostbuster.o
ghostbuster-objs := main.o config_monitor.o dr_monitor.o

#DEBUG=y

ccflags-y := -I$(src)/inc/
ccflags-y += -I$(src)/arch/$(ARCH)/$(SOC_MODEL)
ccflags-$(DEBUG) += -DDEBUG

KDIR := ../../linux
PWD := $(shell pwd)

default: check-model
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

check-model:
ifndef SOC_MODEL
	$(error SOC_MODEL is required)
endif
