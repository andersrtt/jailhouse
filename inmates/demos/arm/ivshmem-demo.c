/*
 * Jailhouse, a Linux-based partitioning hypervisor
 *
 * Copyright (c) Siemens AG, 2014-2016
 * Copyright (c) Retotech AB, 2017
 *
 * Authors:
 *  Henning Schild <henning.schild@siemens.com>
 *  Jonas Weståker <jonas@retotech.se>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */
#include <inmate.h>
#include <mach.h>

#define VENDORID	0x1af4
#define DEVICEID	0x1110

#define IVSHMEM_CFG_SHMEM_PTR	0x40
#define IVSHMEM_CFG_SHMEM_SZ	0x48

#define JAILHOUSE_SHMEM_PROTO_UNDEFINED	0x0000

#define MAX_NDEV	4
#define UART_BASE	0x3F8

static char str[] = "Hello from bare-metal ivshmem-demo inmate!!!  ";
static int irq_counter;

struct ivshmem_dev_data {
	u16 bdf;
	u32 *registers;
	void *shmem;
	u64 shmemsz;
};

static struct ivshmem_dev_data devs[MAX_NDEV];


static u64 pci_cfg_read64(u32 base, u16 bdf, unsigned int addr)
{
	u64 bar = (((u64)pci_cfg_read(base, bdf, addr + 4, 4) << 32) |
		   pci_cfg_read(base, bdf, addr, 4));
	return bar;
}

static void pci_cfg_write64(u32 base, u16 bdf, unsigned int addr, u64 val)
{
	pci_cfg_write(base, bdf, addr + 4, (u32)(val >> 32), 4);
	pci_cfg_write(base, bdf, addr, (u32)val, 4);
}

static int map_shmem_and_bars(u32 base, struct ivshmem_dev_data *d)
{
	d->shmemsz = pci_cfg_read64(base, d->bdf, IVSHMEM_CFG_SHMEM_SZ);
	d->shmem =
	    (void *)((u32)(0xffffffff &
			   pci_cfg_read64(base, d->bdf,
					  IVSHMEM_CFG_SHMEM_PTR)));
	printk("IVSHMEM: shmem is at %p\n", d->shmem);
	d->registers =
	    (u32 *)(((u32)(d->shmem + d->shmemsz + PAGE_SIZE - 1)) & PAGE_MASK);
	pci_cfg_write64(base, d->bdf, PCI_CFG_BAR, (u32)d->registers);
	printk("IVSHMEM: bar0 is at %p\n", d->registers);
	pci_cfg_write(base, d->bdf, PCI_CFG_COMMAND,
		      (PCI_CMD_MEM | PCI_CMD_MASTER), 2);
	return 0;
}

static u32 get_ivpos(struct ivshmem_dev_data *d)
{
	return mmio_read32(d->registers + 2);
}

static void send_irq(struct ivshmem_dev_data *d)
{
	printk("IVSHMEM: %02x:%02x.%x sending IRQ (by writing 1 to 0x%x)\n",
	       d->bdf >> 8, (d->bdf >> 3) & 0x1f, d->bdf & 0x3,
	       d->registers + 3);
	mmio_write32(d->registers + 3, 1);
}

static void enable_irq(struct ivshmem_dev_data *d)
{
	printk("IVSHMEM: Enabling IVSHMEM_IRQs\n");
	mmio_write32(d->registers, 0xffffffff);
}

static void handle_irq(unsigned int irqn)
{
	printk("IVSHMEM: handle_irq(irqn:%d) - interrupt #%d\n",
	       irqn, irq_counter++);
}

void inmate_main(void)
{
	unsigned int i = 0;
	int bdf = 0;
	unsigned int class_rev;
	struct ivshmem_dev_data *d;
	volatile char *shmem;
	int ndevices = 0;

	gic_setup(handle_irq);

	long long pci_cfg_base = cmdline_parse_int("pci-cfg-base", -1);
	if (-1 == pci_cfg_base) {
		printk("ERROR: Provide value for 'pci-cfg-base'-parameter\n"
		       "(using cmdline when loading ivshmem-demo inmate:\n"
		       "' -s \"pci-cfg-base=<pci_mmconfig_base>\" -a <address>').\n"
		       "Check root-cell configuration file:\n"
		       "config.header.platform_info.pci_mmconfig_base \n"
		       "for details on a value applicable to your target system.\n");
		return;
	}
	printk("IVSHMEM: pci-cfg-base:0x%llx\n", pci_cfg_base);

	long long ivshmem_irq = cmdline_parse_int("ivshmem-irq", -1);
	if (-1 == ivshmem_irq) {
		printk("ERROR: Provide value for 'ivshmem-irq'-parameter\n"
		       "(using cmdline when loading ivshmem-demo inmate:\n"
		       "' -s \"ivshmem-irq=<value>\" -a <address>').\n");
		return;
	}
	printk("IVSHMEM: ivshmem-irq:%d\n", ivshmem_irq);

	while ((ndevices < MAX_NDEV) &&
	       (-1 != (bdf =
		       pci_cfg_find_device(pci_cfg_base, VENDORID, DEVICEID,
						      bdf))))
	{
		printk("IVSHMEM: Found %04x:%04x at %02x:%02x.%x\n",
		       pci_cfg_read(pci_cfg_base, bdf, PCI_CFG_VENDOR_ID, 2),
		       pci_cfg_read(pci_cfg_base, bdf, PCI_CFG_DEVICE_ID, 2),
		       bdf >> 8, (bdf >> 3) & 0x1f, bdf & 0x3);
		class_rev = pci_cfg_read(pci_cfg_base, bdf, 0x8, 4);
		if (class_rev != (PCI_DEV_CLASS_OTHER << 24 |
				  JAILHOUSE_SHMEM_PROTO_UNDEFINED << 8)) {
			printk("IVSHMEM: class/revision %08x, not supported "
			       "skipping device\n", class_rev);
			bdf++;
			continue;
		}
		ndevices++;
		d = devs + ndevices - 1;
		d->bdf = bdf;
		if (map_shmem_and_bars(pci_cfg_base, d)) {
			printk("IVSHMEM: Failure mapping shmem and bars.\n");
			return;
		}

		printk("IVSHMEM: mapped shmem and bars, got position %p\n",
		       get_ivpos(d));

		gic_enable_irq(ivshmem_irq + ndevices - 1);
		printk("IVSHMEM: Enabled IRQ:0x%x\n",
		       ivshmem_irq +  ndevices -1);

		enable_irq(d);

		bdf++;
	}

	if (!ndevices) {
		printk("IVSHMEM: No PCI devices found .. nothing to do.\n");
		return;
	}

	printk("IVSHMEM: Done setting up...\n");

	{
		u8 buf[32];
		memcpy(buf, d->shmem, sizeof(buf)/sizeof(buf[0]));
		printk("IVSHMEM: %s\n", buf);
		memcpy(d->shmem, str, sizeof(str)/sizeof(str[0]) + 1);
	}

	while (1) {
		for (i = 0; i < ndevices; i++) {
			d = devs + i;
			//delay_us(1000*1000);
			shmem = d->shmem;
			shmem[sizeof(str)/sizeof(str[0])]++;
			send_irq(d);
		}
		printk("IVSHMEM: waiting for interrupt.\n");
		asm volatile("wfi" : : : "memory");
	}
}
