/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Configuration for gic-demo inmate on Banana Pi:
 * 1 CPU, 64K RAM, serial ports 4-7, CCU+GPIO
 *
 * Copyright (c) Siemens AG, 2014
 * Copyright (c) Retotech AB, 2017
 *
 * Authors:
 *  Jan Kiszka <jan.kiszka@siemens.com>
 *  Jonas Weståker <jonas@retotech.se>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include <jailhouse/types.h>
#include <jailhouse/cell-config.h>

#define ARRAY_SIZE(a) sizeof(a) / sizeof(a[0])

struct {
	struct jailhouse_cell_desc cell;
	__u64 cpus[1];
	struct jailhouse_memory mem_regions[5];
	struct jailhouse_irqchip irqchips[1];
	struct jailhouse_pci_device pci_devices[1];    
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "ivshmem-demo",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
		.num_irqchips = ARRAY_SIZE(config.irqchips),
		.num_pci_devices = ARRAY_SIZE(config.pci_devices),
	},

	.cpus = {
		0x2,
	},

	.mem_regions = {
		/* CCU */ {
			.phys_start = 0x01c20000,
			.virt_start = 0x01c20000,
			.size = 0x400,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO | JAILHOUSE_MEM_IO_32,
		},
		/* GPIO: port H */ {
			.phys_start = 0x01c208fc,
			.virt_start = 0x01c208fc,
			.size = 0x24,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO | JAILHOUSE_MEM_IO_32,
		},
		/* UART 4-7 */ {
			.phys_start = 0x01c29000,
			.virt_start = 0x01c29000,
			.size = 0x1000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_IO,
		},
		/* RAM */ {
			.phys_start = 0x7bfe0000,
			.virt_start = 0,
			.size = 0x00010000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* IVSHMEM */ {
			.phys_start = 0x7c000000,
			.virt_start = 0x7c000000,
			.size = 0x000100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_ROOTSHARED,
		},
	},
	.irqchips = {
		/* GIC */ {
			.address = 0x01c81000,
			.pin_base = 32,
			/* Interrupts:
			   52 of UART 7,
			   155 for IVSHMEM,
			   belong to the client */
			.pin_bitmap = {
				1ULL<<(52-32),
				0,
				0,
				1 << (155-128),
			},
		},
	},
	.pci_devices = {
		{
			.type = JAILHOUSE_PCI_TYPE_IVSHMEM,
			.bdf = 0x00,
			.bar_mask = {
				0xffffff00, 0xffffffff, 0x00000000,
				0x00000000, 0x00000000, 0x00000000,
			},
			.shmem_region = 4, /* IVSHMEM index in .mem_regions */
			.shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
		},
	},
};
