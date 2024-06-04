// SPDX-FileCopyrightText: Deutsches Elektronen-Synchrotron DESY, MSK, ChimeraTK Project <chimeratk-support@desy.de>
// SPDX-License-Identifier: GPL-2.0

// This kernel module is a simple UIO dummy driver mainly useful for debugging and testing
// the ChimeraTK UIO backend. It is based on the file mentioned below, retrieved from
// https://github.com/bgmerrell/driver-samples/blob/master/s_18/lab8_uio_api.c

/* **************** LF331:1.6 s_18/lab8_uio_api.c **************** */
/*
 * The code herein is: Copyright the Linux Foundation, 2011
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://training.linuxfoundation.org
 *     email:  trainingquestions@linuxfoundation.org
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
/*  
 * The UIO API
 *
 * In order to write a user-space driver using the UIO API with a
 * small kernel stub driver you'll have to do the following:
 *
 * Allocate space for a uio_info structure, defined
 * in /usr/src/linux/include/linux/uio_driver.h:
 * struct uio_info - UIO device capabilities
 * @uio_dev:            the UIO device this info belongs to
 * @name:               device name
 * @version:            device driver version
 * @mem:                list of mappable memory regions, size==0 for end of list
 * @port:               list of port regions, size==0 for end of list
 * @irq:                interrupt number or UIO_IRQ_CUSTOM
 * @irq_flags:          flags for request_irq()
 * @priv:               optional private data
 * @handler:            the device's irq handler
 * @mmap:               mmap operation for this uio device
 * @open:               open operation for this uio device
 * @release:            release operation for this uio device
 * @irqcontrol:         disable/enable irqs when 0/1 is written to /dev/uioX
 *
 *struct uio_info {
 *        struct uio_device       *uio_dev;
 *        const char              *name;
 *        const char              *version;
 *        struct uio_mem          mem[MAX_UIO_MAPS];
 *        struct uio_port         port[MAX_UIO_PORT_REGIONS];
 *        long                    irq;
 *        unsigned long           irq_flags;
 *        void                    *priv;
 *        irqreturn_t (*handler)(int irq, struct uio_info *dev_info);
 *        int (*mmap)(struct uio_info *info, struct vm_area_struct *vma);
 *        int (*open)(struct uio_info *info, struct inode *inode);
 *        int (*release)(struct uio_info *info, struct inode *inode);
 *        int (*irqcontrol)(struct uio_info *info, s32 irq_on);
 *};
 *
 * You'll need to fill in entries for at least name, irq, irq_flags
 * and handler, which should return IRQ_HANDLED.
 *  
 * The structure should be register and unregistered with:
 *      
 * int uio_register_device(struct device *parent, struct uio_info *info);
 * void uio_unregister_device(struct uio_info *info);
 *
@*/

#include "version.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/device.h>
#include <linux/uio_driver.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

static struct uio_info *info;
static struct device *dev;
static unsigned long long mem_size = 2692759552 + 32;
static bool irqs_enabled = false;

module_param(mem_size, ullong, S_IRUGO);

static void my_release(struct device *dev)
{
	printk(KERN_INFO "releasing my uio device\n");
}

static int uio_dummy_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "UIO Dummy driver v%s\n", UIO_DUMMY_VERSION);
	seq_printf(m, "    Allocated memory: %llu\n", mem_size);

	return 0;
}

static int uio_dummy_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, uio_dummy_proc_show, NULL);
}

static ssize_t uio_dummy_proc_write(struct file *file, const char __user *ubuf,
				    size_t count, loff_t *ppos)
{
	if (irqs_enabled) {
		printk(KERN_DEBUG
		       "Triggering event through write of proc file\n");
		uio_event_notify(info);
	} else {
		printk(KERN_DEBUG "User did not enable IRQs, not firing\n");
	}

	return count;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 6, 0)
static struct file_operations uio_dummy_proc_operations = {
	.owner = THIS_MODULE,
	.open = uio_dummy_proc_open,
	.read = seq_read,
	.write = uio_dummy_proc_write,
	.llseek = seq_lseek,
	.release = single_release
};
#else
static struct proc_ops uio_dummy_proc_operations = {
	.proc_open = uio_dummy_proc_open,
	.proc_read = seq_read,
	.proc_write = uio_dummy_proc_write,
	.proc_lseek = seq_lseek,
	.proc_release = single_release
};
#endif

static int uio_dummy_irq_control(struct uio_info *dev_info, s32 irq_on)
{
	printk(KERN_INFO "Userspace requested IRQ generation %d\n", irq_on);
	irqs_enabled = irq_on != 0;

	return 0;
}

static int __init uio_dummy_init(void)
{
	struct uio_mem *mem;
	dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	dev_set_name(dev, "uio_dummy_device");
	dev->release = my_release;
	if (device_register(dev) < 0) {
		put_device(dev);
		return -1;
	}

	info = kzalloc(sizeof(struct uio_info), GFP_KERNEL);
	info->name = "uio_dummy_device";
	info->version = UIO_DUMMY_VERSION;

	// CUSTOM -> We do not have a hardware IRQ, but we generate
	// IRQs towards the user otherwise, in this case by getting our proc
	// file written to
	info->irq = UIO_IRQ_CUSTOM;
	info->irqcontrol = uio_dummy_irq_control;

	// Just allocate a slab of virtual memory to be exposed through mmap
	mem = &info->mem[0];
	mem->memtype = UIO_MEM_VIRTUAL;
	mem->addr = (phys_addr_t)vmalloc(mem_size);
	mem->size = mem_size;
	mem->name = "UIO dummy memory block";

	if (uio_register_device(dev, info) < 0) {
		vfree((const void *)mem->addr);
		device_unregister(dev);
		kfree(dev);
		kfree(info);
		printk(KERN_INFO "Failing to register uio device\n");
		return -1;
	}
	printk(KERN_INFO "Allocating %llu bytes for mem0", mem_size);

	proc_create_data("uio-dummy", 0666, NULL, &uio_dummy_proc_operations,
			 dev);
	return 0;
}

static void __exit uio_dummy_exit(void)
{
	struct uio_mem *mem = &info->mem[0];
	uio_unregister_device(info);
	vfree((const void *)mem->addr);
	device_unregister(dev);
	remove_proc_entry("uio-dummy", NULL);
	printk(KERN_INFO "Un-Registered UIO handler\n");
	kfree(info);
	kfree(dev);
}

module_init(uio_dummy_init);
module_exit(uio_dummy_exit);

MODULE_AUTHOR("Jens Georg <jens.georg@desy.de>");
MODULE_DESCRIPTION("An UIO dummy driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(UIO_DUMMY_VERSION);
