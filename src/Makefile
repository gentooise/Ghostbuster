obj-m += ghostbuster.o
ghostbuster-objs := main.o config_monitor.o

ccflags-y := -I$(src)/inc/
ccflags-y += -I$(src)/arch/$(ARCH)/
ccflags-y += -DSOC_MODEL=$(SOC_MODEL)
#ccflags-y += -DDEBUG

KDIR := ../../linux_pi
PWD := $(shell pwd)

default: check-model
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

check-model:
ifndef SOC_MODEL
	$(error SOC_MODEL is required)
endif
