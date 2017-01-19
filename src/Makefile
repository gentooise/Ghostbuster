obj-m += ghostbuster.o
ghostbuster-y := main.o

###### Ghostbuster configuration #######

# Set the following to enable the corresponding monitor.
IO_MONITOR_ENABLED=y
DR_MONITOR_ENABLED=y
MAP_MONITOR_ENABLED=y

# A monitor can be compiled either in active or in passive mode.
# Set the following flags to make the corresponding monitor active,
# otherwise it defaults to passive.
# If the corresponding monitor is not enabled, the flag has no effect.
#
# When I/O monitor is in active mode, it restores the I/O trusted state
# when a change is detected, otherwise it only logs detection events.
# Default: active
IO_MONITOR_ACTIVE=y
# When DR monitor is in active mode, it restores the DR trusted state
# when a change is detected, otherwise it only logs detection events.
# Default: active
DR_MONITOR_ACTIVE=y
# When MAP monitor is in active mode, it prevents programs in user space from
# mapping I/O configuration addresses, otherwise it only logs mapping requests.
# Default: passive
#MAP_MONITOR_ACTIVE=y

# Enable state dump for each monitor, for debug purposes.
# If the corresponding monitor is not enabled, it has no effect.
#IO_DEBUG=y
#DR_DEBUG=y
#MAP_DEBUG=y

######   End of configuration    ######

ghostbuster-$(IO_MONITOR_ENABLED) += io_monitor.o
ghostbuster-$(DR_MONITOR_ENABLED) += dr_monitor.o
ghostbuster-$(MAP_MONITOR_ENABLED) += map_monitor.o

ccflags-y := -I$(src)/inc/
ccflags-y += -I$(src)/arch/$(ARCH)
ccflags-y += -I$(src)/arch/$(ARCH)/$(SOC_MODEL)
ccflags-$(IO_MONITOR_ENABLED) += -DIO_MONITOR_ENABLED
ccflags-$(DR_MONITOR_ENABLED) += -DDR_MONITOR_ENABLED
ccflags-$(MAP_MONITOR_ENABLED) += -DMAP_MONITOR_ENABLED
ccflags-$(IO_MONITOR_ACTIVE) += -DIO_MONITOR_ACTIVE
ccflags-$(DR_MONITOR_ACTIVE) += -DDR_MONITOR_ACTIVE
ccflags-$(MAP_MONITOR_ACTIVE) += -DMAP_MONITOR_ACTIVE
ccflags-$(IO_DEBUG) += -DIO_DEBUG
ccflags-$(DR_DEBUG) += -DDR_DEBUG
ccflags-$(MAP_DEBUG) += -DMAP_DEBUG

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
