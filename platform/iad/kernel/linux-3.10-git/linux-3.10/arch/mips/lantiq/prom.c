/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 * Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/export.h>
#include <linux/clk.h>
#include <linux/bootmem.h>
#include <linux/of_platform.h>
#include <linux/of_fdt.h>

#include <asm/bootinfo.h>
#include <asm/time.h>

#include <lantiq.h>

#include "prom.h"
#include "clk.h"

/* access to the ebu needs to be locked between different drivers */
DEFINE_SPINLOCK(ebu_lock);
EXPORT_SYMBOL_GPL(ebu_lock);

/*
 * this struct is filled by the soc specific detection code and holds
 * information about the specific soc type, revision and name
 */
static struct ltq_soc_info soc_info;

unsigned int ltq_get_cpu_id(void)
{
	return soc_info.partnum;
}
EXPORT_SYMBOL(ltq_get_cpu_id);

unsigned int ltq_get_soc_type(void)
{
	return soc_info.type;
}
EXPORT_SYMBOL(ltq_get_soc_type);

unsigned int ltq_get_soc_rev(void)
{
	return soc_info.rev;
}
EXPORT_SYMBOL(ltq_get_soc_rev);

const char *get_system_type(void)
{
	return soc_info.sys_type;
}

void prom_free_prom_memory(void)
{
}

static void __init prom_init_cmdline(void)
{
	int argc = fw_arg0;
	char **argv = (char **) KSEG1ADDR(fw_arg1);
	int i;

	for (i = 0; i < argc; i++) {
		char *p = (char *) KSEG1ADDR(argv[i]);

		if (CPHYSADDR(p) && *p) {
			if (strstr(p, "console"))
				continue;

#if defined(CONFIG_LANTIQ_STRIP_MEM_FROM_FWARGS)
			if (strstr(p, "mem"))
				continue;
			if (strstr(p, "wlanm"))
				continue;
			if (strstr(p, "phym"))
				continue;
#endif

			strlcat(arcs_cmdline, " ", sizeof(arcs_cmdline));
			strlcat(arcs_cmdline, p, sizeof(arcs_cmdline));
		}
	}
}

void __init plat_mem_setup(void)
{
	struct boot_param_header *bph;

	ioport_resource.start = IOPORT_RESOURCE_START;
	ioport_resource.end = IOPORT_RESOURCE_END;
	iomem_resource.start = IOMEM_RESOURCE_START;
	iomem_resource.end = IOMEM_RESOURCE_END;

	set_io_port_base((unsigned long) KSEG1);

	if (fw_arg0 == (unsigned long)-2) {
		bph = (struct boot_param_header *)fw_arg1;
		if (bph && be32_to_cpu(bph->magic) == OF_DT_HEADER) {
			pr_err("Using OF DTB provided by firmware (UHI)\n");
			early_init_devtree(bph);
			return;
		}
	} else {
		bph = (struct boot_param_header *)fw_arg3;
		if (bph && be32_to_cpu(bph->magic) == OF_DT_HEADER) {
			pr_err("Using OF DTB provided by firmware\n");
			early_init_devtree(bph);
			return;
		}
	}

	bph = initial_boot_params;
	if (bph && be32_to_cpu(bph->magic) == OF_DT_HEADER) {
		pr_err("Using appended OF DTB\n");
		early_init_devtree(bph);
		prom_init_cmdline();
		return;
	}

	bph = (struct boot_param_header *)&__dtb_start;
	if (bph && be32_to_cpu(bph->magic) == OF_DT_HEADER) {
		pr_err("Using built-in OF DTB\n");
		early_init_devtree(bph);
		prom_init_cmdline();
		return;
	}

	panic("device tree not present");
}

void __init device_tree_init(void)
{
	unsigned long base, size;
	void *fdt_copy;

	if (!initial_boot_params)
		return;

	base = virt_to_phys((void *)initial_boot_params);
	size = be32_to_cpu(initial_boot_params->totalsize);

	/* The strings in the flattened tree are referenced directly by the
	 * device tree, so copy the flattened device tree from init memory
	 * to regular memory.
	 */
	fdt_copy = alloc_bootmem(size);
	if (!fdt_copy)
		panic("failed to alloc bootmem for DT");

	memcpy(fdt_copy, initial_boot_params, size);
	initial_boot_params = fdt_copy;

	unflatten_device_tree();
}

void __init prom_init(void)
{
	/* call the soc specific detetcion code and get it to fill soc_info */
	ltq_soc_detect(&soc_info);
	snprintf(soc_info.sys_type, LTQ_SYS_TYPE_LEN - 1, "%s rev %s",
		soc_info.name, soc_info.rev_type);
	soc_info.sys_type[LTQ_SYS_TYPE_LEN - 1] = '\0';
	pr_info("SoC: %s\n", soc_info.sys_type);

#if defined(CONFIG_MIPS_MT_SMP)
	if (register_vsmp_smp_ops())
		panic("failed to register_vsmp_smp_ops()");
#endif
}

int __init plat_of_setup(void)
{
	static struct of_device_id of_ids[3];

	if (!of_have_populated_dt())
		panic("device tree not present");

	strncpy(of_ids[0].compatible, soc_info.compatible,
		sizeof(of_ids[0].compatible));
	strncpy(of_ids[1].compatible, "simple-bus",
		sizeof(of_ids[1].compatible));
	if (of_platform_populate(NULL, of_ids, NULL, NULL))
		panic("failed to poplate DT");
	 /* make sure ithat the reset controller is setup early */
	ltq_rst_init();
	return 0;
}

arch_initcall(plat_of_setup);
