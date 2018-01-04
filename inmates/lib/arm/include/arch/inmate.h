/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) ARM Limited, 2014
 *
 * Authors:
 *  Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 * Alternatively, you can use or redistribute this file under the following
 * BSD license:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define HEAP_BASE 0x0

// FIXME: Consider moving the PCI related definitions and declarations below to
// jailhouse/inmates/lib/pci.h instead?!?!?

#define PAGE_SIZE		(4 * 1024UL)
#define PAGE_MASK		(~(PAGE_SIZE - 1))

// The PCI configuration space provided by Jailhouse is 1MiB (1<<20).
#define PCI_CFG_SIZE		0x00100000
// Each PCI device has 256B (1<<8) of BDf (Bus, Device, Function)
// configuration space.
#define PCI_BDF_SIZE		0x00000100
#define PCI_CFG_VENDOR_ID	0x000
#define PCI_CFG_DEVICE_ID	0x002
#define PCI_CFG_COMMAND		0x004
# define PCI_CMD_IO		(1 << 0)
# define PCI_CMD_MEM		(1 << 1)
# define PCI_CMD_MASTER		(1 << 2)
# define PCI_CMD_INTX_OFF	(1 << 10)
#define PCI_CFG_STATUS		0x006
# define PCI_STS_INT		(1 << 3)
# define PCI_STS_CAPS		(1 << 4)
#define PCI_CFG_BAR		0x010
# define PCI_BAR_64BIT		0x4
#define PCI_CFG_CAP_PTR		0x034

#define PCI_ID_ANY		0xffff

#define PCI_DEV_CLASS_OTHER	0xff


/*
 * To ease the debugging, we can send a spurious hypercall, which should return
 * -ENOSYS, but appear in the hypervisor stats for this cell.
 */
static inline void heartbeat(void)
{
#ifndef CONFIG_BARE_METAL
	asm volatile (
	".arch_extension virt\n"
	"mov	r0, %0\n"
	"hvc	#0\n"
	: : "r" (0xbea7) : "r0");
#endif
}

void __attribute__((interrupt("IRQ"), used)) vector_irq(void);

int pci_cfg_find_device(u32 base, u16 vendor, u16 device, u16 start_bdf);
u32 pci_cfg_read(u32 base, u16 bdf, unsigned int offset, unsigned int size);
void pci_cfg_write(u32 base, u16 bdf, unsigned int offset, u32 value, unsigned int size);
