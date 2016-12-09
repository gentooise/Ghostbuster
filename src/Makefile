obj-m += ghostbuster.o
ghostbuster-y := main.o

# Ghostbuster configuration
#IO_DEBUG=y
#DR_DEBUG=y
IO_MONITOR_ENABLED=y
DR_MONITOR_ENABLED=y

ghostbuster-$(IO_MONITOR_ENABLED) += io_monitor.o
ghostbuster-$(DR_MONITOR_ENABLED) += dr_monitor.o

ccflags-y := -I$(src)/inc/
ccflags-y += -I$(src)/arch/$(ARCH)/$(SOC_MODEL)
ccflags-$(IO_DEBUG) += -DIO_DEBUG
ccflags-$(DR_DEBUG) += -DDR_DEBUG
ccflags-$(IO_MONITOR_ENABLED) += -DIO_MONITOR_ENABLED
ccflags-$(DR_MONITOR_ENABLED) += -DDR_MONITOR_ENABLED

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
