/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Configuration for ivshmem-demo on Banana Pi.
 * Bananapi-gic-demo from Siemens
 * 1 CPU, 64K RAM, serial ports 4-7, CCU+GPIO
 *
 * Copyright (c) Retotech AB, 2017
 * Copyright (c) Siemens AG, 2014
 *
 * Authors:
 *  Ola Redell <ola.redell@retotech.se>
 *  Jan Kiszka <jan.kiszka@siemens.com>
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
	struct jailhouse_pci_device pci_devices[1];
//	struct jailhouse_pci_capability pci_caps[0];
} __attribute__((packed)) config = {
	.cell = {
		.signature = JAILHOUSE_CELL_DESC_SIGNATURE,
		.revision = JAILHOUSE_CONFIG_REVISION,
		.name = "bpi-shmem-demo",
		.flags = JAILHOUSE_CELL_PASSIVE_COMMREG,

		.cpu_set_size = sizeof(config.cpus),
		.num_memory_regions = ARRAY_SIZE(config.mem_regions),
                .num_pci_devices = ARRAY_SIZE(config.pci_devices),
/*                  .num_pci_caps = ARRAY_SIZE(config.pci_caps),*/
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
			.phys_start = 0x7d000000,
			.virt_start = 0,
			.size = 0x00010000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE |
				JAILHOUSE_MEM_EXECUTE | JAILHOUSE_MEM_LOADABLE,
		},
		/* IVSHMEM shared memory region */  {
			.phys_start = 0x7bf00000,
			.virt_start = 0x7bf00000,
			.size = 0x100000,
			.flags = JAILHOUSE_MEM_READ | JAILHOUSE_MEM_WRITE,
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
			.shmem_region = 4, /* must be no of IVSHMEM region above */
			.shmem_protocol = JAILHOUSE_SHMEM_PROTO_UNDEFINED,
		},
                },
};
