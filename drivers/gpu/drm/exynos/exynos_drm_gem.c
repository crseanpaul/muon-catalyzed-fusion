/* exynos_drm_gem.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Author: Inki Dae <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <drm/drmP.h>

#include <linux/completion.h>
#include <linux/dma-buf.h>
#include <linux/kds.h>
#include <linux/shmem_fs.h>
#include <drm/exynos_drm.h>

#include "exynos_drm_drv.h"
#include "exynos_drm_gem.h"
#include "exynos_drm_buf.h"

#ifdef CONFIG_DRM_EXYNOS_DEBUG
static void exynos_gem_info_add_obj(struct drm_gem_object *obj)
{
	struct exynos_drm_private *dev_priv = obj->dev->dev_private;

	atomic_inc(&dev_priv->mm.object_count);
	atomic_add(obj->size, &dev_priv->mm.object_memory);
}

static void exynos_gem_info_remove_obj(struct drm_gem_object *obj)
{
	struct exynos_drm_private *dev_priv = obj->dev->dev_private;

	atomic_dec(&dev_priv->mm.object_count);
	atomic_sub(obj->size, &dev_priv->mm.object_memory);
}
#else
static void exynos_gem_info_add_obj(struct drm_gem_object *obj)
{
}

static void exynos_gem_info_remove_obj(struct drm_gem_object *obj)
{
}
#endif


static unsigned int convert_to_vm_err_msg(int msg)
{
	unsigned int out_msg;

	switch (msg) {
	case 0:
	case -ERESTARTSYS:
	case -EINTR:
		out_msg = VM_FAULT_NOPAGE;
		break;

	case -ENOMEM:
		out_msg = VM_FAULT_OOM;
		break;

	default:
		out_msg = VM_FAULT_SIGBUS;
		break;
	}

	return out_msg;
}

static int check_gem_flags(unsigned int flags)
{
	if (flags & ~(EXYNOS_BO_MASK)) {
		DRM_ERROR("invalid flags.\n");
		return -EINVAL;
	}

	return 0;
}

static void update_vm_cache_attr(struct exynos_drm_gem_obj *obj,
					struct vm_area_struct *vma)
{
	DRM_DEBUG_KMS("flags = 0x%x\n", obj->flags);

	/* non-cachable as default. */
	if (obj->flags & EXYNOS_BO_CACHABLE)
		vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
	else if (obj->flags & EXYNOS_BO_WC)
		vma->vm_page_prot =
			pgprot_writecombine(vm_get_page_prot(vma->vm_flags));
	else
		vma->vm_page_prot =
			pgprot_noncached(vm_get_page_prot(vma->vm_flags));
}

static unsigned long roundup_gem_size(unsigned long size, unsigned int flags)
{
	/* TODO */

	return roundup(size, PAGE_SIZE);
}

static int exynos_drm_gem_map_buf(struct drm_gem_object *obj,
					struct vm_area_struct *vma,
					unsigned long f_vaddr,
					pgoff_t page_offset)
{
	struct exynos_drm_gem_obj *exynos_gem_obj = to_exynos_gem_obj(obj);
	struct exynos_drm_gem_buf *buf = exynos_gem_obj->buffer;
	struct scatterlist *sgl;
	unsigned long pfn;
	int i;

	if (!buf->sgt)
		return -EINTR;

	if (page_offset >= (buf->size >> PAGE_SHIFT)) {
		DRM_ERROR("invalid page offset\n");
		return -EINVAL;
	}

	sgl = buf->sgt->sgl;
	for_each_sg(buf->sgt->sgl, sgl, buf->sgt->nents, i) {
		if (page_offset < (sgl->length >> PAGE_SHIFT))
			break;
		page_offset -=	(sgl->length >> PAGE_SHIFT);
	}

	pfn = __phys_to_pfn(sg_phys(sgl)) + page_offset;

	return vm_insert_mixed(vma, f_vaddr, pfn);
}

static int exynos_drm_gem_handle_create(struct drm_gem_object *obj,
					struct drm_file *file_priv,
					unsigned int *handle)
{
	int ret;

	/*
	 * allocate a id of idr table where the obj is registered
	 * and handle has the id what user can see.
	 */
	ret = drm_gem_handle_create(file_priv, obj, handle);
	if (ret)
		return ret;
	DRM_DEBUG_KMS("Created handle: [BO:%u]\n", *handle);

	/* drop reference from allocate - handle holds it now. */
	drm_gem_object_unreference_unlocked(obj);

	return 0;
}

void exynos_drm_gem_destroy(struct exynos_drm_gem_obj *exynos_gem_obj)
{
	struct drm_gem_object *obj;
	struct exynos_drm_gem_buf *buf;

	DRM_DEBUG_KMS("[GEM:%d]\n", exynos_gem_get_name(exynos_gem_obj));

	obj = &exynos_gem_obj->base;
	buf = exynos_gem_obj->buffer;

	DRM_DEBUG_KMS("handle count = %d\n", atomic_read(&obj->handle_count));

	if (exynos_gem_obj->resource_set != NULL) {
		/* kds_resource_set_release NULLs the pointer */
		kds_resource_set_release(&exynos_gem_obj->resource_set);
	}

	/*
	 * do not release memory region from exporter.
	 *
	 * the region will be released by exporter
	 * once dmabuf's refcount becomes 0.
	 */
	if (obj->import_attach)
		goto out;

	exynos_drm_free_buf(obj->dev, exynos_gem_obj->flags, buf);

	exynos_gem_info_remove_obj(obj);
out:
	exynos_drm_fini_buf(obj->dev, buf);
	exynos_gem_obj->buffer = NULL;

	if (obj->map_list.map)
		drm_gem_free_mmap_offset(obj);

	/* release file pointer to gem object. */
	drm_gem_object_release(obj);

	kfree(exynos_gem_obj);
}

struct exynos_drm_gem_obj *exynos_drm_gem_init(struct drm_device *dev,
						      unsigned long size)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;
	int ret;

	DRM_DEBUG_KMS("size: %lu\n", size);

	exynos_gem_obj = kzalloc(sizeof(*exynos_gem_obj), GFP_KERNEL);
	if (!exynos_gem_obj) {
		DRM_ERROR("failed to allocate exynos gem object\n");
		return NULL;
	}

	obj = &exynos_gem_obj->base;

	ret = drm_gem_object_init(dev, obj, size);
	if (ret < 0) {
		DRM_ERROR("failed to initialize gem object\n");
		kfree(exynos_gem_obj);
		return NULL;
	}

	DRM_DEBUG_KMS("created file object = 0x%x\n", (unsigned int)obj->filp);

	return exynos_gem_obj;
}

struct exynos_drm_gem_obj *exynos_drm_gem_create(struct drm_device *dev,
						unsigned int flags,
						unsigned long size)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buf;
	int ret;

	DRM_DEBUG_KMS("flags: 0x%x size: %lu\n", flags, size);

	if (!size) {
		DRM_ERROR("invalid size.\n");
		return ERR_PTR(-EINVAL);
	}

	size = roundup_gem_size(size, flags);

	ret = check_gem_flags(flags);
	if (ret)
		return ERR_PTR(ret);

	buf = exynos_drm_init_buf(dev, size);
	if (!buf)
		return ERR_PTR(-ENOMEM);

	exynos_gem_obj = exynos_drm_gem_init(dev, size);
	if (!exynos_gem_obj) {
		ret = -ENOMEM;
		goto err_fini_buf;
	}

	exynos_gem_obj->buffer = buf;

	/* set memory type and cache attribute from user side. */
	exynos_gem_obj->flags = flags;

	ret = exynos_drm_alloc_buf(dev, buf, flags);
	if (ret < 0) {
		drm_gem_object_release(&exynos_gem_obj->base);
		goto err_fini_buf;
	}

	exynos_gem_info_add_obj(&exynos_gem_obj->base);

	return exynos_gem_obj;

err_fini_buf:
	exynos_drm_fini_buf(dev, buf);
	return ERR_PTR(ret);
}

int exynos_drm_gem_create_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file_priv)
{
	struct drm_exynos_gem_create *args = data;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret;

	DRM_DEBUG_KMS("size: %llu flags: 0x%x\n", args->size, args->flags);

	exynos_gem_obj = exynos_drm_gem_create(dev, args->flags, args->size);
	if (IS_ERR(exynos_gem_obj))
		return PTR_ERR(exynos_gem_obj);

	ret = exynos_drm_gem_handle_create(&exynos_gem_obj->base, file_priv,
			&args->handle);
	if (ret) {
		exynos_drm_gem_destroy(exynos_gem_obj);
		return ret;
	}
	DRM_DEBUG_KMS("Created [BO:%u]\n", args->handle);

	return 0;
}

dma_addr_t *exynos_drm_gem_get_dma_addr(struct drm_device *dev,
					unsigned int gem_handle,
					struct drm_file *filp)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;

	obj = drm_gem_object_lookup(dev, filp, gem_handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		return ERR_PTR(-EINVAL);
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	return &exynos_gem_obj->buffer->dma_addr;
}

void exynos_drm_gem_put_dma_addr(struct drm_device *dev,
					unsigned int gem_handle,
					struct drm_file *filp)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;

	obj = drm_gem_object_lookup(dev, filp, gem_handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		return;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	drm_gem_object_unreference_unlocked(obj);

	/*
	 * decrease obj->refcount one more time because we has already
	 * increased it at exynos_drm_gem_get_dma_addr().
	 */
	drm_gem_object_unreference_unlocked(obj);
}

int exynos_drm_gem_map_offset_ioctl(struct drm_device *dev, void *data,
				    struct drm_file *file_priv)
{
	struct drm_exynos_gem_map_off *args = data;
	int ret;

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		return -ENODEV;
	}

	ret = exynos_drm_gem_dumb_map_offset(file_priv, dev, args->handle,
			&args->offset);
	if (ret) {
		DRM_ERROR("failed to map [BO:%u], %d\n", args->handle, ret);
		return ret;
	}
	DRM_DEBUG_KMS("[BO:%u] mapped to 0x%llx\n", args->handle, args->offset);
	return ret;
}

static struct drm_file *exynos_drm_find_drm_file(struct drm_device *drm_dev,
							struct file *filp)
{
	struct drm_file *file_priv;

	mutex_lock(&drm_dev->struct_mutex);

	/* find current process's drm_file from filelist. */
	list_for_each_entry(file_priv, &drm_dev->filelist, lhead) {
		if (file_priv->filp == filp) {
			mutex_unlock(&drm_dev->struct_mutex);
			return file_priv;
		}
	}

	mutex_unlock(&drm_dev->struct_mutex);
	WARN_ON(1);

	return ERR_PTR(-EFAULT);
}

static int exynos_drm_gem_mmap_buffer(struct file *filp,
				      struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = filp->private_data;
	struct exynos_drm_gem_obj *exynos_gem_obj = to_exynos_gem_obj(obj);
	struct drm_device *drm_dev = obj->dev;
	struct exynos_drm_gem_buf *buffer;
	struct drm_file *file_priv;
	unsigned long vm_size;
	int ret;

	DRM_DEBUG_KMS("[GEM:%d] vma->pgoff: 0x%lx vma: 0x%lx -> 0x%lx\n",
			gem_get_name(obj), vma->vm_pgoff, vma->vm_start,
			vma->vm_end);

	vma->vm_flags |= VM_IO | VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = obj;
	vma->vm_ops = drm_dev->driver->gem_vm_ops;

	/* restore it to driver's fops. */
	filp->f_op = fops_get(drm_dev->driver->fops);

	file_priv = exynos_drm_find_drm_file(drm_dev, filp);
	if (IS_ERR(file_priv))
		return PTR_ERR(file_priv);

	/* restore it to drm_file. */
	filp->private_data = file_priv;

	update_vm_cache_attr(exynos_gem_obj, vma);

	vm_size = vma->vm_end - vma->vm_start;

	/*
	 * a buffer contains information to physically continuous memory
	 * allocated by user request or at framebuffer creation.
	 */
	buffer = exynos_gem_obj->buffer;

	/* check if user-requested size is valid. */
	if (vm_size > buffer->size)
		return -EINVAL;

	ret = dma_mmap_attrs(drm_dev->dev, vma, buffer->pages,
				buffer->dma_addr, buffer->size,
				&buffer->dma_attrs);
	if (ret < 0) {
		DRM_ERROR("failed to mmap.\n");
		return ret;
	}

	/*
	 * take a reference to this mapping of the object. And this reference
	 * is unreferenced by the corresponding vm_close call.
	 */
	drm_gem_object_reference(obj);

	mutex_lock(&drm_dev->struct_mutex);
	drm_vm_open_locked(drm_dev, vma);
	mutex_unlock(&drm_dev->struct_mutex);

	return 0;
}

static const struct file_operations exynos_drm_gem_fops = {
	.mmap = exynos_drm_gem_mmap_buffer,
};

int exynos_drm_gem_mmap_ioctl(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_exynos_gem_mmap *args = data;
	struct drm_gem_object *obj;
	unsigned int addr;

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		return -ENODEV;
	}

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		return -EINVAL;
	}

	/*
	 * Set specific mmper's fops. And it will be restored by
	 * exynos_drm_gem_mmap_buffer to dev->driver->fops.
	 * This is used to call specific mapper temporarily.
	 */
	file_priv->filp->f_op = &exynos_drm_gem_fops;

	/*
	 * Set gem object to private_data so that specific mmaper
	 * can get the gem object. And it will be restored by
	 * exynos_drm_gem_mmap_buffer to drm_file.
	 */
	file_priv->filp->private_data = obj;

	addr = vm_mmap(file_priv->filp, 0, args->size,
			PROT_READ | PROT_WRITE, MAP_SHARED, 0);

	drm_gem_object_unreference_unlocked(obj);

	if (IS_ERR((void *)addr)) {
		file_priv->filp->private_data = file_priv;
		return PTR_ERR((void *)addr);
	}

	args->mapped = addr;

	DRM_DEBUG_KMS("[BO:%u] %llu bytes mapped to 0x%llx\n",
			args->handle, args->size, args->mapped);

	return 0;
}

int exynos_drm_gem_get_ioctl(struct drm_device *dev, void *data,
				      struct drm_file *file_priv)
{	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_exynos_gem_info *args = data;
	struct drm_gem_object *obj;

	mutex_lock(&dev->struct_mutex);

	obj = drm_gem_object_lookup(dev, file_priv, args->handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

	args->flags = exynos_gem_obj->flags;
	args->size = obj->size;

	drm_gem_object_unreference(obj);
	mutex_unlock(&dev->struct_mutex);

	DRM_DEBUG_KMS("[BO:%u]: size: %llu flags: 0x%x\n",
			args->handle, args->size, args->flags);

	return 0;
}

struct vm_area_struct *exynos_gem_get_vma(struct vm_area_struct *vma)
{
	struct vm_area_struct *vma_copy;

	vma_copy = kmalloc(sizeof(*vma_copy), GFP_KERNEL);
	if (!vma_copy)
		return NULL;

	if (vma->vm_ops && vma->vm_ops->open)
		vma->vm_ops->open(vma);

	if (vma->vm_file)
		get_file(vma->vm_file);

	memcpy(vma_copy, vma, sizeof(*vma));

	vma_copy->vm_mm = NULL;
	vma_copy->vm_next = NULL;
	vma_copy->vm_prev = NULL;

	return vma_copy;
}

void exynos_gem_put_vma(struct vm_area_struct *vma)
{
	if (!vma)
		return;

	if (vma->vm_ops && vma->vm_ops->close)
		vma->vm_ops->close(vma);

	if (vma->vm_file)
		fput(vma->vm_file);

	kfree(vma);
}

int exynos_gem_get_pages_from_userptr(unsigned long start,
						unsigned int npages,
						struct page **pages,
						struct vm_area_struct *vma)
{
	int get_npages;

	/* the memory region mmaped with VM_PFNMAP. */
	if (vma_is_io(vma)) {
		unsigned int i;

		for (i = 0; i < npages; ++i, start += PAGE_SIZE) {
			unsigned long pfn;
			int ret = follow_pfn(vma, start, &pfn);
			if (ret)
				return ret;

			pages[i] = pfn_to_page(pfn);
		}

		if (i != npages) {
			DRM_ERROR("failed to get user_pages.\n");
			return -EINVAL;
		}

		return 0;
	}

	get_npages = get_user_pages(current, current->mm, start,
					npages, 1, 1, pages, NULL);
	get_npages = max(get_npages, 0);
	if (get_npages != npages) {
		DRM_ERROR("failed to get user_pages.\n");
		while (get_npages)
			put_page(pages[--get_npages]);
		return -EFAULT;
	}

	return 0;
}

void exynos_gem_put_pages_to_userptr(struct page **pages,
					unsigned int npages,
					struct vm_area_struct *vma)
{
	if (!vma_is_io(vma)) {
		unsigned int i;

		for (i = 0; i < npages; i++) {
			set_page_dirty_lock(pages[i]);

			/*
			 * undo the reference we took when populating
			 * the table.
			 */
			put_page(pages[i]);
		}
	}
}

int exynos_gem_map_sgt_with_dma(struct drm_device *drm_dev,
				struct sg_table *sgt,
				enum dma_data_direction dir)
{
	int nents;

	mutex_lock(&drm_dev->struct_mutex);

	nents = dma_map_sg(drm_dev->dev, sgt->sgl, sgt->nents, dir);
	if (!nents) {
		DRM_ERROR("failed to map sgl with dma.\n");
		mutex_unlock(&drm_dev->struct_mutex);
		return nents;
	}

	mutex_unlock(&drm_dev->struct_mutex);
	return 0;
}

void exynos_gem_unmap_sgt_from_dma(struct drm_device *drm_dev,
				struct sg_table *sgt,
				enum dma_data_direction dir)
{
	dma_unmap_sg(drm_dev->dev, sgt->sgl, sgt->nents, dir);
}

static void cpu_acquire_kds_cb_fn(void *param1, void *param2)
{
	struct completion* completion = (struct completion *)param1;
	complete(completion);
}

int exynos_drm_gem_cpu_acquire_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file)
{
	struct drm_exynos_gem_cpu_acquire *args = data;
	struct drm_exynos_file_private *file_priv = file->driver_priv;
	struct drm_gem_object *obj;
	struct exynos_drm_gem_obj *exynos_gem_obj;
#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	struct kds_resource *kds;
	struct kds_resource_set *rset;
	unsigned long exclusive;
	struct kds_callback callback;
	DECLARE_COMPLETION_ONSTACK(completion);
#endif
	struct exynos_drm_gem_obj_node *gem_node;
	int ret = 0;

	DRM_DEBUG_KMS("[BO:%u] flags: 0x%x\n", args->handle, args->flags);

	mutex_lock(&dev->struct_mutex);

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		ret = -ENODEV;
		goto unlock;
	}

	obj = drm_gem_object_lookup(dev, file, args->handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		ret = -EINVAL;
		goto unlock;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	if (exynos_gem_obj->base.dma_buf == NULL) {
		/* If there is no dmabuf present, there is no cross-process/
		 * cross-device sharing and sync is unnecessary.
		 */
		ret = 0;
		goto unref_obj;
	}

	exclusive = 0;
	if ((args->flags & DRM_EXYNOS_GEM_CPU_ACQUIRE_EXCLUSIVE) != 0)
		exclusive = 1;
	kds = &exynos_gem_obj->base.dma_buf->kds;
	kds_callback_init(&callback, 1, &cpu_acquire_kds_cb_fn);
	ret = kds_async_waitall(&rset, &callback,
		&completion, NULL, 1, &exclusive, &kds);
	mutex_unlock(&dev->struct_mutex);

	if (!IS_ERR_VALUE(ret))
		ret = wait_for_completion_interruptible(&completion);
	kds_callback_term(&callback);

	mutex_lock(&dev->struct_mutex);
	if (IS_ERR_VALUE(ret))
		goto release_rset;
#endif

	gem_node = kzalloc(sizeof(*gem_node), GFP_KERNEL);
	if (!gem_node) {
		DRM_ERROR("failed to allocate eyxnos_drm_gem_obj_node.\n");
		ret = -ENOMEM;
		goto release_rset;
	}

#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	WARN_ON(exynos_gem_obj->resource_set);
	exynos_gem_obj->resource_set = rset;
#endif

	gem_node->exynos_gem_obj = exynos_gem_obj;
	list_add(&gem_node->list, &file_priv->gem_cpu_acquire_list);
	mutex_unlock(&dev->struct_mutex);
	return 0;


release_rset:
#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	kds_resource_set_release_sync(&rset);
#endif

unref_obj:
	drm_gem_object_unreference(obj);

unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

int exynos_drm_gem_cpu_release_ioctl(struct drm_device *dev, void* data,
				struct drm_file *file)
{
	struct drm_exynos_gem_cpu_release *args = data;
	struct drm_exynos_file_private *file_priv = file->driver_priv;
	struct drm_gem_object *obj;
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct list_head *cur;
	int ret = 0;

	DRM_DEBUG_KMS("[BO:%u]\n", args->handle);

	mutex_lock(&dev->struct_mutex);

	if (!(dev->driver->driver_features & DRIVER_GEM)) {
		DRM_ERROR("does not support GEM.\n");
		ret = -ENODEV;
		goto unlock;
	}

	obj = drm_gem_object_lookup(dev, file, args->handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		ret = -EINVAL;
		goto unlock;
	}

	exynos_gem_obj = to_exynos_gem_obj(obj);

#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	if (exynos_gem_obj->base.dma_buf == NULL) {
		/* If there is no dmabuf present, there is no cross-process/
		 * cross-device sharing and sync is unnecessary.
		 */
		ret = 0;
		goto unref_obj;
	}
#endif

	list_for_each(cur, &file_priv->gem_cpu_acquire_list) {
		struct exynos_drm_gem_obj_node *node = list_entry(
				cur, struct exynos_drm_gem_obj_node, list);
		if (node->exynos_gem_obj == exynos_gem_obj)
			break;
	}
	if (cur == &file_priv->gem_cpu_acquire_list) {
		DRM_ERROR("gem object not acquired for current process.\n");
		ret = -EINVAL;
		goto unref_obj;
	}

#ifdef CONFIG_DMA_SHARED_BUFFER_USES_KDS
	/* kds_resource_set_release NULLs the pointer */
	WARN_ON(exynos_gem_obj->resource_set == NULL);
	kds_resource_set_release(&exynos_gem_obj->resource_set);
#endif

	list_del(cur);
	kfree(list_entry(cur, struct exynos_drm_gem_obj_node, list));
	/* unreference for the reference held since cpu_acquire_ioctl */
	drm_gem_object_unreference(obj);
	ret = 0;

unref_obj:
	/* unreference for the reference from drm_gem_object_lookup() */
	drm_gem_object_unreference(obj);

unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

void exynos_drm_gem_free_object(struct drm_gem_object *obj)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct exynos_drm_gem_buf *buf;

	DRM_DEBUG_KMS("[GEM:%d]\n", gem_get_name(obj));

	exynos_gem_obj = to_exynos_gem_obj(obj);
	buf = exynos_gem_obj->buffer;

	if (obj->import_attach)
		drm_prime_gem_destroy(obj, buf->sgt);

	exynos_drm_gem_destroy(to_exynos_gem_obj(obj));
}

int exynos_drm_gem_dumb_create(struct drm_file *file_priv,
			       struct drm_device *dev,
			       struct drm_mode_create_dumb *args)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	int ret;

	DRM_DEBUG_KMS("%ux%u bpp: %u flags: 0x%x\n",
			args->width, args->height, args->bpp, args->flags);

	/*
	 * alocate memory to be used for framebuffer.
	 * - this callback would be called by user application
	 *	with DRM_IOCTL_MODE_CREATE_DUMB command.
	 */

	args->pitch = args->width * ((args->bpp + 7) / 8);
	args->pitch = round_up(args->pitch, 64);

	args->size = args->pitch * args->height;

	exynos_gem_obj = exynos_drm_gem_create(dev, args->flags, args->size);
	if (IS_ERR(exynos_gem_obj))
		return PTR_ERR(exynos_gem_obj);

	ret = exynos_drm_gem_handle_create(&exynos_gem_obj->base, file_priv,
			&args->handle);
	if (ret) {
		exynos_drm_gem_destroy(exynos_gem_obj);
		return ret;
	}
	DRM_DEBUG_KMS("Created [BO:%u] size: %llu pitch: %u\n",
			args->handle, args->size, args->pitch);

	return 0;
}

int exynos_drm_gem_dumb_map_offset(struct drm_file *file_priv,
				   struct drm_device *dev, uint32_t handle,
				   uint64_t *offset)
{
	struct drm_gem_object *obj;
	int ret = 0;

	DRM_DEBUG_KMS("[BO:%u]\n", handle);

	mutex_lock(&dev->struct_mutex);

	/*
	 * get offset of memory allocated for drm framebuffer.
	 * - this callback would be called by user application
	 *	with DRM_IOCTL_MODE_MAP_DUMB command.
	 */

	obj = drm_gem_object_lookup(dev, file_priv, handle);
	if (!obj) {
		DRM_ERROR("failed to lookup gem object.\n");
		ret = -EINVAL;
		goto unlock;
	}

	if (!obj->map_list.map) {
		ret = drm_gem_create_mmap_offset(obj);
		if (ret)
			goto out;
	}

	*offset = (u64)obj->map_list.hash.key << PAGE_SHIFT;
	DRM_DEBUG_KMS("[BO:%u] mapped to 0x%llx\n", handle, *offset);

out:
	drm_gem_object_unreference(obj);
unlock:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

int exynos_drm_gem_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct drm_gem_object *obj = vma->vm_private_data;
	struct drm_device *dev = obj->dev;
	unsigned long f_vaddr;
	pgoff_t page_offset;
	int ret;

	page_offset = ((unsigned long)vmf->virtual_address -
			vma->vm_start) >> PAGE_SHIFT;
	f_vaddr = (unsigned long)vmf->virtual_address;

	mutex_lock(&dev->struct_mutex);

	ret = exynos_drm_gem_map_buf(obj, vma, f_vaddr, page_offset);
	if (ret < 0)
		DRM_ERROR("failed to map a buffer with user.\n");

	mutex_unlock(&dev->struct_mutex);

	return convert_to_vm_err_msg(ret);
}

int exynos_drm_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct exynos_drm_gem_obj *exynos_gem_obj;
	struct drm_gem_object *obj;
	int ret;

	DRM_DEBUG_KMS("pgoff: 0x%lx\n", vma->vm_pgoff);

	/* set vm_area_struct. */
	ret = drm_gem_mmap(filp, vma);
	if (ret < 0) {
		DRM_ERROR("failed to mmap.\n");
		return ret;
	}

	obj = vma->vm_private_data;
	exynos_gem_obj = to_exynos_gem_obj(obj);

	ret = check_gem_flags(exynos_gem_obj->flags);
	if (ret) {
		drm_gem_vm_close(vma);
		drm_gem_free_mmap_offset(obj);
		return ret;
	}

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_flags |= VM_MIXEDMAP;

	update_vm_cache_attr(exynos_gem_obj, vma);

	return ret;
}
