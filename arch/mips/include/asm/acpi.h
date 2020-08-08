#ifndef _ASM_MIPS_ACPI_H
#define _ASM_MIPS_ACPI_H

/*
 *  Lvjianmin <lvjianmin@loongson.cn>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <asm/early_ioremap.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/numa.h>
#include <acpi/pdc_loongson.h>

#ifdef CONFIG_ACPI

extern int acpi_strict;
extern int acpi_disabled;
extern int acpi_pci_disabled;
extern int acpi_noirq;

static inline void disable_acpi(void)
{
	acpi_disabled = 1;
	acpi_pci_disabled = 1;
	acpi_noirq = 1;
}

static inline void acpi_noirq_set(void) { acpi_noirq = 1; }

static inline void acpi_disable_pci(void)
{
	acpi_pci_disabled = 1;
	acpi_noirq_set();
}

static inline bool acpi_has_cpu_in_madt(void)
{
	return true;
}

extern int acpi_gsi_to_irq(u32 gsi, unsigned int *irq);

/* Low-level suspend routine. */
extern void wakeup_start(void);
extern int (*acpi_suspend_lowlevel)(void);
extern unsigned long long arch_acpi_wakeup_start;

/* Physical address to resume after wakeup */
#define acpi_wakeup_address arch_acpi_wakeup_start
#define ACPI_TABLE_UPGRADE_MAX_PHYS (max_low_pfn_mapped << PAGE_SHIFT)

/*
 * Check if the CPU can handle C2 and deeper
 */
static inline unsigned int acpi_processor_cstate_check(unsigned int max_cstate)
{
	return max_cstate;
}

static inline bool arch_has_acpi_pdc(void)
{
	return false;
}

static inline void arch_acpi_set_pdc_bits(u32 *buf)
{
}
#else /* !CONFIG_ACPI */

static inline void acpi_noirq_set(void) { }
static inline void acpi_disable_pci(void) { }
static inline void disable_acpi(void) { }

#endif /* !CONFIG_ACPI */

#define acpi_unlazy_tlb(x)

#endif /* _ASM_MIPS_ACPI_H */
