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
jraid-objs += jraid-main.o jraid-thread.o

KEV := /lib/modules/`uname -r`/build
PWD :=$(shell pwd)

all:
	$(MAKE) -C $(KEV) M=$(PWD) modules
clean:
	$(MAKE) -C $(KEV) M=$(PWD) clean 
