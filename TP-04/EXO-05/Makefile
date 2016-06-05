.PHONY: all 
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

obj-m := hideModuleBis.o

else

  KERNELDIR = /tmp/linux-4.2.3
  #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

all :
	$(info obj-m : $(obj-m))
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
