/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2019 Loongson Technology Corp.
 * Author: Jianmin Lv, lvjianmin@loongson.cn
 *
 */

#include <linux/pci.h>
#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/dmi.h>
#include <linux/slab.h>
#include <linux/pci-acpi.h>
#include <pci.h>

struct resource *acpi_io_resource;
struct resource *acpi_mem_resource;
struct pci_controller *acpi_controller;

struct pci_root_info {
	struct acpi_pci_root_info common;
	struct pci_controller pc;
};

void pcibios_add_bus(struct pci_bus *bus)
{
	acpi_pci_add_bus(bus);
}

int pcibios_root_bridge_prepare(struct pci_host_bridge *bridge)
{
	struct pci_controller *pc = bridge->bus->sysdata;

	ACPI_COMPANION_SET(&bridge->dev, pc->companion);
	return 0;
}

static void acpi_release_root_info(struct acpi_pci_root_info *info)
{
	kfree(container_of(info, struct pci_root_info, common));
}

static int acpi_prepare_root_resources(struct acpi_pci_root_info *ci)
{
	int status;
	struct acpi_device *device = ci->bridge;
	struct resource_entry *entry, *tmp;

	acpi_io_resource->start = LOONGSON_PCI_IO_START;

	status = acpi_pci_probe_root_resources(ci);
	if (status > 0)
		return status;

	resource_list_for_each_entry_safe(entry, tmp, &ci->resources) {
		dev_printk(KERN_DEBUG, &device->dev,
			   "host bridge window %pR (ignored)\n", entry->res);
		resource_list_destroy_entry(entry);
	}
	pci_add_resource(&ci->resources, acpi_mem_resource);
	pci_add_resource(&ci->resources, acpi_io_resource);

	return 0;
}

static struct acpi_pci_root_ops acpi_pci_root_ops = {
	.release_info = acpi_release_root_info,
	.prepare_resources = acpi_prepare_root_resources,
};

struct pci_bus *pci_acpi_scan_root(struct acpi_pci_root *root)
{
	struct pci_bus *bus;
	struct pci_root_info *info;
	struct pci_controller *controller;
	int domain = root->segment;
	int busnum = root->secondary.start;
	static int need_domain_info;
	struct acpi_device *device = root->device;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		printk(KERN_WARNING "pci_bus %04x:%02x: "
				"ignored (out of memory)\n", domain, busnum);
		return NULL;
	}

	controller = &info->pc;
	controller = acpi_controller;
	controller->companion = device;
	controller->index = domain;
	bus = pci_find_bus(domain, busnum);
	if (bus) {
		/*
		 * If the desired bus has been scanned already, replace
		 * its bus->sysdata.
		 */
		memcpy(bus->sysdata, controller, sizeof(*controller));
		kfree(info);
	} else {
		acpi_pci_root_ops.pci_ops = controller->pci_ops;
		bus = acpi_pci_root_create(root, &acpi_pci_root_ops,
					   &info->common, controller);
		if (bus) {

			need_domain_info = need_domain_info || pci_domain_nr(bus);
			set_pci_need_domain_info(controller, need_domain_info);

			/*
			 * We insert PCI resources into the iomem_resource and
			 * ioport_resource trees in either pci_bus_claim_resources()
			 * or pci_bus_assign_resources().
			 */
			if (pci_has_flag(PCI_PROBE_ONLY)) {
				pci_bus_claim_resources(bus);
			} else {
				struct pci_bus *child;

				pci_bus_size_bridges(bus);
				pci_bus_assign_resources(bus);
				list_for_each_entry(child, &bus->children, node)
					pcie_bus_configure_settings(child);
			}
		} else {
			kfree(info);
		}
	}
	return bus;
}
