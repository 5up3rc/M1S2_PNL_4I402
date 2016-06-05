.PHONY: all 
#.SECONDARY:

ifneq ($(KERNELRELEASE),)

  obj-m := uuname.o

$(info yyy)
else

  KERNELDIR = /tmp/linux-4.2.3
  #KERNELDIR ?= /lib/modules/$(shell uname -r)/build
  PWD := $(shell pwd)

$(info xxx)

all :
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif

