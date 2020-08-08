/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2019 Loongson Technology Corp.
 * Author: Jianmin Lv, lvjianmin@loongson.cn
 */

#ifndef _ASM_MIPS_EFI_H
#define _ASM_MIPS_EFI_H

#include <linux/efi.h>

extern void __init efi_init(void);

static inline void efifb_setup_from_dmi(struct screen_info *si, const char *opt)
{
}

#define ARCH_EFI_IRQ_FLAGS_MASK  0x00000001  /* bit0: CP0 Status.IE */

#define arch_efi_call_virt_setup()               \
({                                               \
})

#define arch_efi_call_virt(p, f, args...)        \
({                                               \
	efi_##f##_t * __f;                       \
	__f = p->f;                              \
	__f(args);                               \
})

#define arch_efi_call_virt_teardown()            \
({                                               \
})

#endif /* _ASM_MIPS_EFI_H */
