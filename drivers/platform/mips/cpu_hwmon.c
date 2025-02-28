// SPDX-License-Identifier: GPL-2.0-only
#include <linux/err.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/jiffies.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <loongson.h>
#include <boot_param.h>
#include <loongson_hwmon.h>
#include <loongson_regs.h>

static int csr_temp_enable = 0;

/* Allow other reference temperatures to fixup the original cpu temperature */
int __weak fixup_cpu_temp(int cpu, int cputemp)
{
	return cputemp;
}

/*
 * Loongson-3 series cpu has two sensors inside,
 * each of them from 0 to 255,
 * if more than 127, that is dangerous.
 * here only provide sensor1 data, because it always hot than sensor0
 */
int loongson3_cpu_temp(int cpu)
{
	int cputemp;
	u32 reg, prid_rev;

	if (csr_temp_enable) {
		reg = (csr_readl(LOONGSON_CSR_CPUTEMP) & 0xff);
		goto out;
	}

	reg = LOONGSON_CHIPTEMP(cpu);
	prid_rev = read_c0_prid() & PRID_REV_MASK;

	switch (prid_rev) {
	case PRID_REV_LOONGSON3A_R1:
		reg = (reg >> 8) & 0xff;
		break;
	case PRID_REV_LOONGSON3B_R1:
	case PRID_REV_LOONGSON3B_R2:
	case PRID_REV_LOONGSON3A_R2_0:
	case PRID_REV_LOONGSON3A_R2_1:
		reg = ((reg >> 8) & 0xff) - 100;
		break;
	case PRID_REV_LOONGSON3A_R3_0:
	case PRID_REV_LOONGSON3A_R3_1:
	default:
		reg = (reg & 0xffff)*731/0x4000 - 273;
		break;
	}

out:
	cputemp = (int)reg * 1000;
	return fixup_cpu_temp(cpu, cputemp);
}

static int nr_packages;
static struct device *cpu_hwmon_dev;

static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf);
static SENSOR_DEVICE_ATTR(name, S_IRUGO, get_hwmon_name, NULL, 0);

static struct attribute *cpu_hwmon_attributes[] = {
	&sensor_dev_attr_name.dev_attr.attr,
	NULL
};

/* Hwmon device attribute group */
static struct attribute_group cpu_hwmon_attribute_group = {
	.attrs = cpu_hwmon_attributes,
};

/* Hwmon device get name */
static ssize_t get_hwmon_name(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "cpu-hwmon\n");
}

static ssize_t get_cpu_temp(struct device *dev,
			struct device_attribute *attr, char *buf);
static ssize_t cpu_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf);

static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO, get_cpu_temp, NULL, 1);
static SENSOR_DEVICE_ATTR(temp1_label, S_IRUGO, cpu_temp_label, NULL, 1);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO, get_cpu_temp, NULL, 2);
static SENSOR_DEVICE_ATTR(temp2_label, S_IRUGO, cpu_temp_label, NULL, 2);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO, get_cpu_temp, NULL, 3);
static SENSOR_DEVICE_ATTR(temp3_label, S_IRUGO, cpu_temp_label, NULL, 3);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO, get_cpu_temp, NULL, 4);
static SENSOR_DEVICE_ATTR(temp4_label, S_IRUGO, cpu_temp_label, NULL, 4);

static const struct attribute *hwmon_cputemp[4][3] = {
	{
		&sensor_dev_attr_temp1_input.dev_attr.attr,
		&sensor_dev_attr_temp1_label.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_temp2_input.dev_attr.attr,
		&sensor_dev_attr_temp2_label.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_temp3_input.dev_attr.attr,
		&sensor_dev_attr_temp3_label.dev_attr.attr,
		NULL
	},
	{
		&sensor_dev_attr_temp4_input.dev_attr.attr,
		&sensor_dev_attr_temp4_label.dev_attr.attr,
		NULL
	}
};

static ssize_t cpu_temp_label(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	return sprintf(buf, "CPU %d Temperature\n", id);
}

static ssize_t get_cpu_temp(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int id = (to_sensor_dev_attr(attr))->index - 1;
	int value = loongson3_cpu_temp(id);
	return sprintf(buf, "%d\n", value);
}

static int create_sysfs_cputemp_files(struct kobject *kobj)
{
	int i, ret = 0;

	for (i=0; i<nr_packages; i++)
		ret = sysfs_create_files(kobj, hwmon_cputemp[i]);

	return ret;
}

static void remove_sysfs_cputemp_files(struct kobject *kobj)
{
	int i;

	for (i=0; i<nr_packages; i++)
		sysfs_remove_files(kobj, hwmon_cputemp[i]);
}

static int cpu_initial_threshold = 65000;
static int cpu_thermal_threshold = 96000;

static struct delayed_work thermal_work;

static void do_thermal_timer(struct work_struct *work)
{
	int i, value, temp_max = 0;

	for (i=0; i<nr_packages; i++) {
		value = loongson3_cpu_temp(i);
		if (value > temp_max)
			temp_max = value;
	}

	if (temp_max <= cpu_thermal_threshold)
		schedule_delayed_work(&thermal_work, msecs_to_jiffies(5000));
	else
		orderly_poweroff(true);
}

static int __init loongson_hwmon_init(void)
{
	int i, ret, value, temp_max = 0;

	pr_info("Loongson Hwmon Enter...\n");

	if (cpu_has_csr())
		csr_temp_enable = csr_readl(LOONGSON_CSR_FEATURES) & LOONGSON_CSRF_TEMP;

	cpu_hwmon_dev = hwmon_device_register(NULL);
	if (IS_ERR(cpu_hwmon_dev)) {
		ret = PTR_ERR(cpu_hwmon_dev);
		pr_err("hwmon_device_register fail!\n");
		goto fail_hwmon_device_register;
	}

	nr_packages = loongson_sysconf.nr_cpus /
		loongson_sysconf.cores_per_package;

	ret = sysfs_create_group(&cpu_hwmon_dev->kobj,
				&cpu_hwmon_attribute_group);
	if (ret) {
		pr_err("fail to create loongson hwmon!\n");
		goto fail_sysfs_create_group_hwmon;
	}

	ret = create_sysfs_cputemp_files(&cpu_hwmon_dev->kobj);
	if (ret) {
		pr_err("fail to create cpu temperature interface!\n");
		goto fail_create_sysfs_cputemp_files;
	}

	for (i=0; i<nr_packages; i++) {
		value = loongson3_cpu_temp(i);
		if (value > temp_max)
			temp_max = value;
	}

	pr_info("Initial CPU temperature is %d (highest).\n", temp_max);
	if (temp_max > cpu_initial_threshold)
		cpu_thermal_threshold += temp_max - cpu_initial_threshold;

	INIT_DEFERRABLE_WORK(&thermal_work, do_thermal_timer);
	schedule_delayed_work(&thermal_work, msecs_to_jiffies(20000));

	return ret;

fail_create_sysfs_cputemp_files:
	sysfs_remove_group(&cpu_hwmon_dev->kobj,
				&cpu_hwmon_attribute_group);

fail_sysfs_create_group_hwmon:
	hwmon_device_unregister(cpu_hwmon_dev);

fail_hwmon_device_register:
	return ret;
}

static void __exit loongson_hwmon_exit(void)
{
	cancel_delayed_work_sync(&thermal_work);
	remove_sysfs_cputemp_files(&cpu_hwmon_dev->kobj);
	sysfs_remove_group(&cpu_hwmon_dev->kobj,
				&cpu_hwmon_attribute_group);
	hwmon_device_unregister(cpu_hwmon_dev);
}

module_init(loongson_hwmon_init);
module_exit(loongson_hwmon_exit);

MODULE_AUTHOR("Yu Xiang <xiangy@lemote.com>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_DESCRIPTION("Loongson CPU Hwmon driver");
MODULE_LICENSE("GPL");
