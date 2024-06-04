UIO Dummy
=========

Very simple UIO driver that allocates a block of virtual memory and exposes it

Basically only useful for testing the DeviceAccess UIO backend

The module supports one parameter, `mem_size`, which is in bytes.

To trigger an UIO soft interrupt, write to the `/proc/uio-dummy` file by e.g. 

echo 1 > /proc/uio-dummy

Building the module
-------------------

The preferred way is to use cmake to generate the Kbuild Makefile and build the module.
If DKMS is found during configuration, the cmake build will also provide targets `dkms-install`
and `dkms-remove` to simplify the dkms-based installation.

If you want to opt not to use cmake, create a version.h file from version.h.in and set the
define `UIO_DUMMY_VERSION` to a sensible value and build using `make -f Makefile.in`
