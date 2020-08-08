/*
 * EFI partition
 *
 * Just for ACPI here, complete it when implementing EFI runtime.
 *
 * lvjianmin: <lvjianmin@loongson.cn>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/efi.h>
#include <linux/efi-bgrt.h>
#include <linux/export.h>
#include <linux/acpi.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/reboot.h>
#include <linux/bcd.h>

#include <asm/efi.h>
#include <boot_param.h>

extern unsigned int has_systab;
extern unsigned long systab_addr;
extern struct bootparamsinterface *efi_bp;

static efi_config_table_type_t arch_tables[] __initdata = {
	{NULL_GUID, NULL, NULL},
};

static int __init efi_runtime_init(void)
{
	if (efi_runtime_disabled()) {
		pr_info("EFI runtime services will be disabled.\n");
		return -1;
	}

	efi_native_runtime_setup();
	set_bit(EFI_RUNTIME_SERVICES, &efi.flags);

	return 0;
}

static int __init efi_systab_init(void)
{
	efi.systab = (efi_system_table_t *)efi_bp->systemtable;
	if (efi.systab == NULL) {
		panic("Can't find EFI system table.\n");
		return -1;
	}

	set_bit(EFI_64BIT, &efi.flags);
	efi.config_table = (unsigned long)efi.systab->tables;
	efi.runtime	= (unsigned long)efi.systab->runtime;
	efi.runtime_version = (unsigned int)efi.systab->runtime->hdr.revision;

	if (efi_config_init(arch_tables))
		return -1;

	if (efi.smbios3 != EFI_INVALID_TABLE_ADDR)
		systab_addr =  efi.smbios3;
	else if (efi.smbios != EFI_INVALID_TABLE_ADDR)
		systab_addr =  efi.smbios;
	else {
		pr_err("%s : ERROR: smbios addr invaild\n",__func__);
		return -1;
	}
	has_systab = 1;

	set_bit(EFI_CONFIG_TABLES, &efi.flags);

	if (efi_runtime_init())
		return -1;

	return 0;
}

void __init efi_init(void)
{
	efi_systab_init();
}
