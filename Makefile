obj-m += uio-dummy.o

SRC := $(shell pwd)
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

ifeq ($(PACKAGING_INSTALL),1)
install: packaging_install
else
install: kernel_install
endif

kernel_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) install

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

packaging_install:
	install -D *.c Makefile -t $(DESTDIR)/usr/src/$(DKMS_PKG_NAME)-$(DKMS_PKG_VERSION)/
	install -D uio-dummy.rules -t $(DESTDIR)/etc/udev/rules.d/

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
