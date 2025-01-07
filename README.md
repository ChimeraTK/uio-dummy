UIO Dummy
=========

Very simple UIO driver that allocates a block of virtual memory and exposes it

Basically only useful for testing the DeviceAccess UIO backend

The module supports one parameter, `mem_size`, which is in bytes.

To trigger an UIO soft interrupt, write to the `/proc/uio-dummy` file by e.g. 

echo 1 > /proc/uio-dummy

Building the module
-------------------

You can either use Makefile.kernel directly to build the module or use cmake and `make install`
to install the module using dkms.
