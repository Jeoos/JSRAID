obj-m := Submit.o
Submit-objs := submit.o blkdev-sys.o

KEV := /lib/modules/`uname -r`/build
PWD :=$(shell pwd)

all:
	$(MAKE) -C $(KEV) M=$(PWD) modules
	gcc ioctl.c -o ioctl
clean:
	$(MAKE) -C $(KEV) M=$(PWD) clean 
	rm ioctl

