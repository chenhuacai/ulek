// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2009 Lemote Inc.
 * Author: Wu Zhangjin, wuzhangjin@gmail.com
 */

#include <linux/memblock.h>
#include <asm/bootinfo.h>
#include <asm/traps.h>
#include <asm/smp-ops.h>
#include <asm/cacheflush.h>

#include <loongson.h>
#include <loongson-pch.h>
#include <asm/time.h>
#include <linux/memblock.h>
#include <linux/dmi.h>
#include <asm/uasm.h>
#include <asm/efi.h>
#include <workarounds.h>
#include <boot_param.h>
#include <linux/acpi.h>
/* Loongson CPU address windows config space base address */
unsigned long __maybe_unused _loongson_addrwincfg_base;

extern u32 cpu_clock_freq;
extern char cpu_full_name[64];
static char *product_name;

char *bios_vendor;
char *bios_release_date;
char *board_manufacturer;
struct board_devices eboard_smbios;
struct interface_info einter_smbios;

static const char dmi_empty_string[] = "        ";

#define SMBIOS_BIOSSIZE_OFFSET		5
#define SMBIOS_BOISEXTERN_OFFSET 	15
#define SMBIOS_FREQLOW_OFFSET		18
#define SMBIOS_FREQHIGH_OFFSET		19
#define SMBIOS_FREQLOW_MASK		0xFF
#define SMBIOS_CORE_PACKAGE_OFFSET	31
#define LOONGSON_EFI_ENABLE     	(1 << 3)

static void __init mips_ebase_setup(void)
{
	ebase = CKSEG0;

	if (cpu_has_ebase_wg)
		write_c0_ebase(ebase | MIPS_EBASE_WG);

	write_c0_ebase(ebase);
}

static void __init mips_nmi_setup(void)
{
	void *base;
	extern char except_vec_nmi;

	base = (void *)(CAC_BASE + 0x380);
	memcpy(base, &except_vec_nmi, 0x80);
	flush_icache_range((unsigned long)base, (unsigned long)base + 0x80);
}

const char *dmi_string_parse(const struct dmi_header *dm, u8 s)
{
	const u8 *bp = ((u8 *) dm) + dm->length;

	if (s) {
		s--;
		while (s > 0 && *bp) {
			bp += strlen(bp) + 1;
			s--;
		}

		if (*bp != 0) {
			size_t len = strlen(bp)+1;
			size_t cmp_len = len > 8 ? 8 : len;

			if (!memcmp(bp, dmi_empty_string, cmp_len))
				return dmi_empty_string;
		return bp;
		}
	}

	return "";

}

static void __init parse_cpu_table(const struct dmi_header *dm)
{
	int freq_temp = 0;
	const char *cpuname;
	char *dmi_data = (char *)(dm + 1);

	freq_temp = ((*(dmi_data + SMBIOS_FREQHIGH_OFFSET) << 8) + \
			((*(dmi_data + SMBIOS_FREQLOW_OFFSET)) & SMBIOS_FREQLOW_MASK));
	cpu_clock_freq = freq_temp * 1000000;
	cpuname = dmi_string_parse(dm, dmi_data[12]);
	if (!strncmp(cpuname, "Loongson", 8))
		strncpy(cpu_full_name, cpuname, sizeof(cpu_full_name));
	if (cpu_full_name[0] == 0)
		strncpy(cpu_full_name, __cpu_full_name[0], sizeof(cpu_full_name));
	loongson_sysconf.cores_per_package = *(dmi_data + SMBIOS_CORE_PACKAGE_OFFSET);

	mips_cpu_frequency = cpu_clock_freq;
	pr_info("CpuClock = %u\n", cpu_clock_freq);

}

static void __init parse_bios_table(const struct dmi_header *dm)
{
	int bios_extern;
	char *dmi_data = (char *)(dm + 1);

	bios_extern = *(dmi_data + SMBIOS_BOISEXTERN_OFFSET);

	if (bios_extern & LOONGSON_EFI_ENABLE)
		set_bit(EFI_BOOT, &efi.flags);
	else
		clear_bit(EFI_BOOT, &efi.flags);

	einter_smbios.size = *(dmi_data + SMBIOS_BIOSSIZE_OFFSET);
}


static void __init find_tokens(const struct dmi_header *dm, void *dummy)
{
	switch (dm->type) {
	case 0x0: /* Extern BIOS */
		parse_bios_table(dm);
		break;
	case 0x4: /* Calling interface */
		parse_cpu_table(dm);
		break;
	}
}

static void __init smbios_parse(void)
{
	bios_vendor = (void *)dmi_get_system_info(DMI_BIOS_VENDOR);
	strcpy(einter_smbios.description,dmi_get_system_info(DMI_BIOS_VERSION));
	bios_release_date = (void *)dmi_get_system_info(DMI_BIOS_DATE);
	board_manufacturer = (void *)dmi_get_system_info(DMI_BOARD_VENDOR);
	strcpy(eboard_smbios.name, dmi_get_system_info(DMI_BOARD_NAME));
	dmi_walk(find_tokens, NULL);
	product_name = (void *)dmi_get_system_info(DMI_PRODUCT_NAME);
	strsep(&product_name,"-");
	strsep(&product_name,"-");
	product_name = strsep(&product_name,"-");
}

unsigned long max_low_pfn_mapped;
extern void __init parse_cmdline(void);
extern void __init early_memblock_init(void);

void __init prom_init(void)
{
	char freq[12];

#ifdef CONFIG_CPU_SUPPORTS_ADDRWINCFG
	_loongson_addrwincfg_base = (unsigned long)
		ioremap(LOONGSON_ADDRWINCFG_BASE, LOONGSON_ADDRWINCFG_SIZE);
#endif

	prom_init_cmdline();
	prom_init_env();

	/*init the uart base address */
	prom_init_uart_base();

#ifdef CONFIG_ACPI
	if (acpiboot)
		early_memblock_init();
	parse_cmdline();
#endif

	/* init base address of io space */
	set_io_port_base((unsigned long)
		ioremap(LOONGSON_PCIIO_BASE, LOONGSON_PCIIO_SIZE));


#ifdef CONFIG_ACPI
	if (acpiboot) {
#ifdef CONFIG_EFI
		efi_init();
#endif
#ifdef CONFIG_ACPI_TABLE_UPGRADE
		acpi_table_upgrade();
#endif
		acpi_gbl_use_default_register_widths = false;
		acpi_boot_table_init();
		acpi_boot_init();
	}
#endif

#ifdef CONFIG_NUMA
	prom_init_numa_memory();
#else
	if (acpiboot)
		prom_init_memory_new();
	else
		prom_init_memory_old();
#endif

#ifdef CONFIG_ACPI
 	dmi_setup();

	if (acpiboot)
		smbios_parse();
#endif

	/* Append default cpu frequency with round-off */
	sprintf(freq, " @ %uMHz", (cpu_clock_freq + 500000) / 1000000);
	strncat(cpu_full_name, freq, sizeof(cpu_full_name));
	__cpu_full_name[0] = cpu_full_name;

	if (loongson_pch->early_config)
		loongson_pch->early_config();

	register_smp_ops(&loongson3_smp_ops);
	board_ebase_setup = mips_ebase_setup;
	board_nmi_handler_setup = mips_nmi_setup;
}

void __init prom_free_prom_memory(void)
{
}
