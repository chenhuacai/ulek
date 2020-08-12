/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Loongson Advanced SIMD Extension (ASX) context handling code for KVM.
 *
 * Copyright (C) 2019 Loongson Inc.
 * Author: Xing Li, lixing@loongson.cn
 * Author: Huacai Chen, chenhc@lemote.com
 */

#include <asm/asm.h>
#include <asm/asm-offsets.h>
#include <asm/asmmacro.h>
#include <asm/regdef.h>

	.set	noreorder
	.set	noat

LEAF(__kvm_save_asx)
	xvst_b	0,  VCPU_FPR0,  a0
	xvst_b	1,  VCPU_FPR1,  a0
	xvst_b	2,  VCPU_FPR2,  a0
	xvst_b	3,  VCPU_FPR3,  a0
	xvst_b	4,  VCPU_FPR4,  a0
	xvst_b	5,  VCPU_FPR5,  a0
	xvst_b	6,  VCPU_FPR6,  a0
	xvst_b	7,  VCPU_FPR7,  a0
	xvst_b	8,  VCPU_FPR8,  a0
	xvst_b	9,  VCPU_FPR9,  a0
	xvst_b	10, VCPU_FPR10, a0
	xvst_b	11, VCPU_FPR11, a0
	xvst_b	12, VCPU_FPR12, a0
	xvst_b	13, VCPU_FPR13, a0
	xvst_b	14, VCPU_FPR14, a0
	xvst_b	15, VCPU_FPR15, a0
	xvst_b	16, VCPU_FPR16, a0
	xvst_b	17, VCPU_FPR17, a0
	xvst_b	18, VCPU_FPR18, a0
	xvst_b	19, VCPU_FPR19, a0
	xvst_b	20, VCPU_FPR20, a0
	xvst_b	21, VCPU_FPR21, a0
	xvst_b	22, VCPU_FPR22, a0
	xvst_b	23, VCPU_FPR23, a0
	xvst_b	24, VCPU_FPR24, a0
	xvst_b	25, VCPU_FPR25, a0
	xvst_b	26, VCPU_FPR26, a0
	xvst_b	27, VCPU_FPR27, a0
	xvst_b	28, VCPU_FPR28, a0
	xvst_b	29, VCPU_FPR29, a0
	xvst_b	30, VCPU_FPR30, a0
	xvst_b	31, VCPU_FPR31, a0
	jr	ra
	 nop
	END(__kvm_save_asx)

LEAF(__kvm_restore_asx)
	xvld_b	0,  VCPU_FPR0,  a0
	xvld_b	1,  VCPU_FPR1,  a0
	xvld_b	2,  VCPU_FPR2,  a0
	xvld_b	3,  VCPU_FPR3,  a0
	xvld_b	4,  VCPU_FPR4,  a0
	xvld_b	5,  VCPU_FPR5,  a0
	xvld_b	6,  VCPU_FPR6,  a0
	xvld_b	7,  VCPU_FPR7,  a0
	xvld_b	8,  VCPU_FPR8,  a0
	xvld_b	9,  VCPU_FPR9,  a0
	xvld_b	10, VCPU_FPR10, a0
	xvld_b	11, VCPU_FPR11, a0
	xvld_b	12, VCPU_FPR12, a0
	xvld_b	13, VCPU_FPR13, a0
	xvld_b	14, VCPU_FPR14, a0
	xvld_b	15, VCPU_FPR15, a0
	xvld_b	16, VCPU_FPR16, a0
	xvld_b	17, VCPU_FPR17, a0
	xvld_b	18, VCPU_FPR18, a0
	xvld_b	19, VCPU_FPR19, a0
	xvld_b	20, VCPU_FPR20, a0
	xvld_b	21, VCPU_FPR21, a0
	xvld_b	22, VCPU_FPR22, a0
	xvld_b	23, VCPU_FPR23, a0
	xvld_b	24, VCPU_FPR24, a0
	xvld_b	25, VCPU_FPR25, a0
	xvld_b	26, VCPU_FPR26, a0
	xvld_b	27, VCPU_FPR27, a0
	xvld_b	28, VCPU_FPR28, a0
	xvld_b	29, VCPU_FPR29, a0
	xvld_b	30, VCPU_FPR30, a0
	xvld_b	31, VCPU_FPR31, a0
	jr	ra
	 nop
	END(__kvm_restore_asx)

	.macro	kvm_restore_asx_upper128 wr, off, base
	.set	push
	.set	noat
	ld $1, (\off+8)(\base)
	xinsert_d \wr, 2
	ld $1, (\off+16)(\base)
	xinsert_d \wr, 3
	.set	pop
	.endm

	.macro	kvm_restore_asx_upper192 wr, off, base
	.set	push
	.set	noat
	ld $1, \off(\base)
	xinsert_d \wr, 1
	ld $1, (\off+8)(\base)
	xinsert_d \wr, 2
	ld $1, (\off+16)(\base)
	xinsert_d \wr, 3
	.set	pop
	.endm

LEAF(__kvm_restore_asx_upper128)
	kvm_restore_asx_upper128	0,  VCPU_FPR0 +8, a0
	kvm_restore_asx_upper128	1,  VCPU_FPR1 +8, a0
	kvm_restore_asx_upper128	2,  VCPU_FPR2 +8, a0
	kvm_restore_asx_upper128	3,  VCPU_FPR3 +8, a0
	kvm_restore_asx_upper128	4,  VCPU_FPR4 +8, a0
	kvm_restore_asx_upper128	5,  VCPU_FPR5 +8, a0
	kvm_restore_asx_upper128	6,  VCPU_FPR6 +8, a0
	kvm_restore_asx_upper128	7,  VCPU_FPR7 +8, a0
	kvm_restore_asx_upper128	8,  VCPU_FPR8 +8, a0
	kvm_restore_asx_upper128	9,  VCPU_FPR9 +8, a0
	kvm_restore_asx_upper128	10, VCPU_FPR10+8, a0
	kvm_restore_asx_upper128	11, VCPU_FPR11+8, a0
	kvm_restore_asx_upper128	12, VCPU_FPR12+8, a0
	kvm_restore_asx_upper128	13, VCPU_FPR13+8, a0
	kvm_restore_asx_upper128	14, VCPU_FPR14+8, a0
	kvm_restore_asx_upper128	15, VCPU_FPR15+8, a0
	kvm_restore_asx_upper128	16, VCPU_FPR16+8, a0
	kvm_restore_asx_upper128	17, VCPU_FPR17+8, a0
	kvm_restore_asx_upper128	18, VCPU_FPR18+8, a0
	kvm_restore_asx_upper128	19, VCPU_FPR19+8, a0
	kvm_restore_asx_upper128	20, VCPU_FPR20+8, a0
	kvm_restore_asx_upper128	21, VCPU_FPR21+8, a0
	kvm_restore_asx_upper128	22, VCPU_FPR22+8, a0
	kvm_restore_asx_upper128	23, VCPU_FPR23+8, a0
	kvm_restore_asx_upper128	24, VCPU_FPR24+8, a0
	kvm_restore_asx_upper128	25, VCPU_FPR25+8, a0
	kvm_restore_asx_upper128	26, VCPU_FPR26+8, a0
	kvm_restore_asx_upper128	27, VCPU_FPR27+8, a0
	kvm_restore_asx_upper128	28, VCPU_FPR28+8, a0
	kvm_restore_asx_upper128	29, VCPU_FPR29+8, a0
	kvm_restore_asx_upper128	30, VCPU_FPR30+8, a0
	kvm_restore_asx_upper128	31, VCPU_FPR31+8, a0
	jr	ra
	 nop
	END(__kvm_restore_asx_upper128)

LEAF(__kvm_restore_asx_upper192)
	kvm_restore_asx_upper192	0,  VCPU_FPR0 +8, a0
	kvm_restore_asx_upper192	1,  VCPU_FPR1 +8, a0
	kvm_restore_asx_upper192	2,  VCPU_FPR2 +8, a0
	kvm_restore_asx_upper192	3,  VCPU_FPR3 +8, a0
	kvm_restore_asx_upper192	4,  VCPU_FPR4 +8, a0
	kvm_restore_asx_upper192	5,  VCPU_FPR5 +8, a0
	kvm_restore_asx_upper192	6,  VCPU_FPR6 +8, a0
	kvm_restore_asx_upper192	7,  VCPU_FPR7 +8, a0
	kvm_restore_asx_upper192	8,  VCPU_FPR8 +8, a0
	kvm_restore_asx_upper192	9,  VCPU_FPR9 +8, a0
	kvm_restore_asx_upper192	10, VCPU_FPR10+8, a0
	kvm_restore_asx_upper192	11, VCPU_FPR11+8, a0
	kvm_restore_asx_upper192	12, VCPU_FPR12+8, a0
	kvm_restore_asx_upper192	13, VCPU_FPR13+8, a0
	kvm_restore_asx_upper192	14, VCPU_FPR14+8, a0
	kvm_restore_asx_upper192	15, VCPU_FPR15+8, a0
	kvm_restore_asx_upper192	16, VCPU_FPR16+8, a0
	kvm_restore_asx_upper192	17, VCPU_FPR17+8, a0
	kvm_restore_asx_upper192	18, VCPU_FPR18+8, a0
	kvm_restore_asx_upper192	19, VCPU_FPR19+8, a0
	kvm_restore_asx_upper192	20, VCPU_FPR20+8, a0
	kvm_restore_asx_upper192	21, VCPU_FPR21+8, a0
	kvm_restore_asx_upper192	22, VCPU_FPR22+8, a0
	kvm_restore_asx_upper192	23, VCPU_FPR23+8, a0
	kvm_restore_asx_upper192	24, VCPU_FPR24+8, a0
	kvm_restore_asx_upper192	25, VCPU_FPR25+8, a0
	kvm_restore_asx_upper192	26, VCPU_FPR26+8, a0
	kvm_restore_asx_upper192	27, VCPU_FPR27+8, a0
	kvm_restore_asx_upper192	28, VCPU_FPR28+8, a0
	kvm_restore_asx_upper192	29, VCPU_FPR29+8, a0
	kvm_restore_asx_upper192	30, VCPU_FPR30+8, a0
	kvm_restore_asx_upper192	31, VCPU_FPR31+8, a0
	jr	ra
	 nop
	END(__kvm_restore_asx_upper192)
