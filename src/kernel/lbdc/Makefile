
obj-m += Lbd.o
Lbd-objs += lbd.o sraid_common.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=`pwd` modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=`pwd` clean
