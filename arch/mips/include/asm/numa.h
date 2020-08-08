/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2019 Loongson Technology Corp.
 * Author: Jianmin Lv, lvjianmin@loongson.cn
 *
 */

#ifndef _ASM_MIPS_NUMA_H
#define _ASM_MIPS_NUMA_H

#include <linux/nodemask.h>

#ifdef CONFIG_NUMA
extern nodemask_t numa_nodes_parsed __initdata;
struct numa_memblk {
	u64			start;
	u64			end;
	int			nid;
};

#define NR_NODE_MEMBLKS		(MAX_NUMNODES*2)
struct numa_meminfo {
	int			nr_blks;
	struct numa_memblk	blk[NR_NODE_MEMBLKS];
};
extern int __init numa_add_memblk(int nodeid, u64 start, u64 end);
extern s16 __cpuid_to_node[CONFIG_NR_CPUS];
extern void __init numa_add_cpu(int cpuid, s16 node);
static inline void numa_clear_node(int cpu)
{
}
static inline void  set_cpuid_to_node(int cpuid, s16 node)
{
	__cpuid_to_node[cpuid] = node;
}
extern int numa_off;

#endif	/* CONFIG_NUMA */

#endif	/* _ASM_MIPS_NUMA_H */
