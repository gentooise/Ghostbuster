obj-m += ghostbuster.o
ghostbuster-y := main.o

###### Ghostbuster configuration #######

# Set the following to enable the corresponding monitor.
IO_MONITOR_ENABLED=y
DR_MONITOR_ENABLED=y

# Enable state dump for each monitor, for debug purposes.
# If the corresponding monitor is not enabled, it has no effect.
#IO_DEBUG=y
#DR_DEBUG=y

# A monitor is active if it actively restores the trusted state when a change is detected.
# Set the following to make the corresponding monitor active.
# Otherwise the monitor will be passive, giving only alarms about detections.
IO_MONITOR_ACTIVE=y
DR_MONITOR_ACTIVE=y

######   End of configuration    ######

ghostbuster-$(IO_MONITOR_ENABLED) += io_monitor.o
ghostbuster-$(DR_MONITOR_ENABLED) += dr_monitor.o

ccflags-y := -I$(src)/inc/
ccflags-y += -I$(src)/arch/$(ARCH)/$(SOC_MODEL)
ccflags-$(IO_DEBUG) += -DIO_DEBUG
ccflags-$(DR_DEBUG) += -DDR_DEBUG
ccflags-$(IO_MONITOR_ENABLED) += -DIO_MONITOR_ENABLED
ccflags-$(DR_MONITOR_ENABLED) += -DDR_MONITOR_ENABLED
ccflags-$(IO_MONITOR_ACTIVE) += -DIO_MONITOR_ACTIVE
ccflags-$(DR_MONITOR_ACTIVE) += -DDR_MONITOR_ACTIVE

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
