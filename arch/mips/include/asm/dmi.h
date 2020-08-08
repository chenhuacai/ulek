/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_DMI_H
#define _ASM_DMI_H

#include <asm/early_ioremap.h>

/* MIPS initialize DMI scan before SLAB is ready, so we use memblock here */
#define dmi_alloc(l)			memblock_alloc_low(l, PAGE_SIZE)

static inline void __iomem *dmi_ioremap(resource_size_t phys_addr, unsigned long size)
{
	return ((void *)TO_CAC(phys_addr));
}

static inline void dmi_iounmap(volatile void __iomem *addr)
{
}

#define dmi_early_remap		early_ioremap
#define dmi_early_unmap		early_iounmap
#define dmi_remap		dmi_ioremap
#define dmi_unmap		dmi_iounmap

#if defined(CONFIG_MACH_LOONGSON64)
#define SMBIOS_ENTRY_POINT_SCAN_START	0xFFFE000
#endif

#endif /* _ASM_DMI_H */
