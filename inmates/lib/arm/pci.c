/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2014
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <inmate.h>

int pci_cfg_find_device(u32 base, u16 vendor, u16 device, u16 start_bdf)
{
	unsigned int bdf;
	u32 addr;
	u16 id;

	for (addr = base + start_bdf * PCI_BDF_SIZE, bdf = start_bdf;
	     addr < base + PCI_CFG_SIZE;
	     addr += PCI_BDF_SIZE, bdf++)
	{
		id = pci_cfg_read(addr, bdf, PCI_CFG_VENDOR_ID, 2);
		if (id == PCI_ID_ANY || (vendor != PCI_ID_ANY && vendor != id))
		{
			continue;
		}
		if (device == PCI_ID_ANY ||
		    pci_cfg_read(addr, bdf, PCI_CFG_DEVICE_ID, 2) == device)
		{
			return bdf;
		}
	}
	return -1;
}

u32 pci_cfg_read(u32 base, u16 bdf, unsigned int addr, unsigned int size)
{
	u32 reg_addr = base | ((u32)bdf << 8) | (addr & 0xfc);
	//printk("%s(bdf:0x%x, addr:%p, size:0x%x), reg_addr0x%x\n", __func__, bdf, addr, size, reg_addr);
	switch (size) {
	case 1:
		return mmio_read8((u8 *)(reg_addr + (addr & 0x3)));
	case 2:
		return mmio_read16((u16 *)(reg_addr + (addr & 0x3)));
	case 4:
		return mmio_read32((u32 *)(reg_addr));
	default:
		return -1;
	}
}

void pci_cfg_write(u32 base, u16 bdf, unsigned int addr, u32 value, unsigned int size)
{
	u32 reg_addr = base | ((u32)bdf << 8) | (addr & 0xfc);
	//printk("%s(bdf:0x%x, addr:%p, value:0x%x, size:0x%x), reg_addr0x%x\n", __func__, bdf, addr, value, size, reg_addr);
	switch (size) {
	case 1:
		mmio_write8((u8 *)(reg_addr + (addr & 0x3)), value);
		break;
	case 2:
		mmio_write16((u16 *)(reg_addr + (addr & 0x3)), value);
		break;
	case 4:
		mmio_write32((u32 *)(reg_addr), value);
		break;
	}
}
