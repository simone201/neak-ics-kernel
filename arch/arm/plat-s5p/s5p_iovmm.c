/* linux/arch/arm/plat-s5p/s5p_iovmm.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/slab.h>
#include <linux/scatterlist.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/ion.h>
#include <linux/iommu.h>
#include <linux/genalloc.h>
#include <linux/err.h>
#include <linux/spinlock.h>

#include <plat/iovmm.h>

struct s5p_vm_region {
	struct list_head node;
	dma_addr_t start;
	size_t size;
};

struct s5p_iovmm {
	struct list_head node;		/* element of s5p_iovmm_list */
	struct iommu_domain *domain;
	struct device *dev;
	struct gen_pool *vmm_pool;
	struct list_head regions_list;	/* list of s5p_vm_region */
	bool   active;
	spinlock_t lock;
};

static DEFINE_RWLOCK(iovmm_list_lock);
static LIST_HEAD(s5p_iovmm_list);

static struct s5p_iovmm *find_iovmm(struct device *dev)
{
	struct list_head *pos;
	struct s5p_iovmm *vmm = NULL;

	read_lock(&iovmm_list_lock);
	list_for_each(pos, &s5p_iovmm_list) {
		vmm = list_entry(pos, struct s5p_iovmm, node);
		if (vmm->dev == dev)
			break;
	}
	read_unlock(&iovmm_list_lock);
	return vmm;
}

static struct s5p_vm_region *find_region(struct s5p_iovmm *vmm, dma_addr_t iova)
{
	struct list_head *pos;
	struct s5p_vm_region *region;

	list_for_each(pos, &vmm->regions_list) {
		region = list_entry(pos, struct s5p_vm_region, node);
		if (region->start == iova)
			return region;
	}
	return NULL;
}

int iovmm_setup(struct device *dev)
{
	struct s5p_iovmm *vmm;
	int ret;

	vmm = kzalloc(sizeof(*vmm), GFP_KERNEL);
	if (!vmm) {
		ret = -ENOMEM;
		goto err_setup_alloc;
	}

	vmm->vmm_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!vmm->vmm_pool) {
		ret = -ENOMEM;
		goto err_setup_genalloc;
	}

	/* 1GB addr space from 0x80000000 */
	ret = gen_pool_add(vmm->vmm_pool, 0x80000000, 0x20000000, -1);
	if (ret)
		goto err_setup_domain;

	vmm->domain = iommu_domain_alloc();
	if (!vmm->domain) {
		ret = -ENOMEM;
		goto err_setup_domain;
	}

	vmm->dev = dev;

	spin_lock_init(&vmm->lock);

	INIT_LIST_HEAD(&vmm->node);
	INIT_LIST_HEAD(&vmm->regions_list);

	write_lock(&iovmm_list_lock);
	list_add(&vmm->node, &s5p_iovmm_list);
	write_unlock(&iovmm_list_lock);

	return 0;
err_setup_domain:
	gen_pool_destroy(vmm->vmm_pool);
err_setup_genalloc:
	kfree(vmm);
err_setup_alloc:
	return ret;
}

void iovmm_cleanup(struct device *dev)
{
	struct s5p_iovmm *vmm;

	vmm = find_iovmm(dev);

	WARN_ON(!vmm);
	if (vmm) {
		struct list_head *pos, *tmp;

		if (vmm->active)
			iommu_detach_device(vmm->domain, dev);

		iommu_domain_free(vmm->domain);

		list_for_each_safe(pos, tmp, &vmm->regions_list) {
			struct s5p_vm_region *region;

			region = list_entry(pos, struct s5p_vm_region, node);

			/* No need to unmap the region because
			 * iommu_domain_free() frees the page table */
			gen_pool_free(vmm->vmm_pool, region->start,
								region->size);

			kfree(list_entry(pos, struct s5p_vm_region, node));
		}

		gen_pool_destroy(vmm->vmm_pool);

		write_lock(&iovmm_list_lock);
		list_del(&vmm->node);
		write_unlock(&iovmm_list_lock);

		kfree(vmm);
	}
}

int iovmm_activate(struct device *dev)
{
	struct s5p_iovmm *vmm;
	int ret = 0;

	vmm = find_iovmm(dev);
	if (WARN_ON(!vmm))
		return -EINVAL;

	spin_lock(&vmm->lock);
	ret = iommu_attach_device(vmm->domain, vmm->dev);
	if (!ret)
		vmm->active = true;
	spin_unlock(&vmm->lock);

	return ret;
}

void iovmm_deactivate(struct device *dev)
{
	struct s5p_iovmm *vmm;
	unsigned long flags;

	vmm = find_iovmm(dev);
	if (WARN_ON(!vmm))
		return;

	spin_lock_irqsave(&vmm->lock, flags);
	iommu_detach_device(vmm->domain, vmm->dev);

	vmm->active = false;
	spin_unlock_irqrestore(&vmm->lock, flags);
}

dma_addr_t iovmm_map(struct device *dev, struct scatterlist *sg)
{
	size_t size = 0;
	off_t start_off = offset_in_page(sg_phys(sg));
	dma_addr_t addr, start = 0;
	struct s5p_vm_region *region;
	struct s5p_iovmm *vmm;
	struct scatterlist *tmpsg;
	int order;

	BUG_ON(!sg);

	vmm = find_iovmm(dev);
	if (WARN_ON(!vmm))
		goto err_map_nomem;

	tmpsg = sg;
	size = start_off;
	do {
		size += sg_dma_len(tmpsg);
	} while ((tmpsg = sg_next(tmpsg)));

	size = PAGE_ALIGN(size);

	spin_lock(&vmm->lock);

	order = __fls(min(size, (size_t)SZ_1M));
	start = (dma_addr_t)gen_pool_alloc_aligned(vmm->vmm_pool, size, order);
	if (!start)
		goto err_map_nomem_lock;

	addr = start;
	do {
		phys_addr_t phys;
		size_t len;

		phys = sg_phys(sg);
		len = PAGE_ALIGN(sg_dma_len(sg) + offset_in_page(phys));
		phys = round_down(phys, PAGE_SIZE);

		while (len > 0) {
			order = min3(__ffs(phys), __ffs(addr), __fls(len));
			if (iommu_map(vmm->domain, addr, phys,
							order - PAGE_SHIFT, 0))
				goto err_map_map;

			addr += (1 << order);
			phys += (1 << order);
			len -= (1 << order);
		}
	} while ((sg = sg_next(sg)));

	region = kmalloc(sizeof(*region), GFP_KERNEL);
	if (!region)
		goto err_map_map;

	region->start = start + start_off;
	region->size = size;
	INIT_LIST_HEAD(&region->node);

	list_add(&region->node, &vmm->regions_list);

	spin_unlock(&vmm->lock);

	return region->start;
err_map_map:
	while (addr >= start) {
		int order;
		size_t size = addr - start;

		order = min(__fls(size), __ffs(start));

		iommu_unmap(vmm->domain, start, order - PAGE_SHIFT);

		start += 1 << order;
		size -= 1 << order;
	}
	gen_pool_free(vmm->vmm_pool, start, size);

err_map_nomem_lock:
	spin_unlock(&vmm->lock);
err_map_nomem:
	return (dma_addr_t)0;
}

void iovmm_unmap(struct device *dev, dma_addr_t iova)
{
	struct s5p_vm_region *region;
	struct s5p_iovmm *vmm;
	unsigned long flags;

	vmm = find_iovmm(dev);

	if (WARN_ON(!vmm))
		return;

	spin_lock_irqsave(&vmm->lock, flags);

	region = find_region(vmm, iova);
	if (WARN_ON(!region))
		goto err_region_not_found;

	region->start = round_down(region->start, PAGE_SIZE);

	gen_pool_free(vmm->vmm_pool, region->start, region->size);
	list_del(&region->node);

	while (region->size != 0) {
		int order;

		order = min(__fls(region->size), __ffs(region->start));

		iommu_unmap(vmm->domain, region->start, order - PAGE_SHIFT);

		region->start += 1 << order;
		region->size -= 1 << order;
	}

	kfree(region);

err_region_not_found:
	spin_unlock_irqrestore(&vmm->lock, flags);
}

static int __init s5p_iovmm_init(void)
{
	return 0;
}
arch_initcall(s5p_iovmm_init);
