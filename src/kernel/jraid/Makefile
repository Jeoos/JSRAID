#
# Makefile
#
# Contact: JeCortex@yahoo.com
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

obj-m := jraid.o
jraid-objs += jraid-main.o jraid-thread.o jraid-pool.o jraid-lbd.o \
	jraid-io.o jraid-dv.o jraid-bufio.o jraid-sys.o \
	persistent-data/jraid-btree.o \
	persistent-data/jraid-btree-remove.o \
	persistent-data/jraid-block-manager.o \
	persistent-data/jraid-transaction-manager.o \
	persistent-data/jraid-space-map-common.o \
	persistent-data/jraid-space-map-disk.o \
	persistent-data/jraid-space-map-metadata.o \
	
KEV := /lib/modules/`uname -r`/build
PWD :=$(shell pwd)

all:
	$(MAKE) -C $(KEV) M=$(PWD) modules
clean:
	$(MAKE) -C $(KEV) M=$(PWD) clean 
