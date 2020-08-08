#ifndef _ASM_EARLY_IOREMAP_H
#define _ASM_EARLY_IOREMAP_H

#include <linux/types.h>
#include <asm/pgtable.h>

static inline void __iomem *early_ioremap(resource_size_t phys_addr, unsigned long size)
{
	return ((void *)TO_CAC(phys_addr));
}

static inline void early_iounmap(void __iomem *addr, unsigned long size)
{
}

#define early_memremap early_ioremap
#define early_memunmap early_iounmap

static inline void *early_memremap_prot(resource_size_t phys_addr, unsigned long size,
		    unsigned long prot_val)
{
	return early_memremap(phys_addr, size);
}

#endif /* _ASM_EARLY_IOREMAP_H */
