obj-m += uio-dummy.o

all:
	make -C /lib/modules/$(shell uname -r)/build V=1 M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build V=1 M=$(PWD) clean
