/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include "precomp.h"
#include "gl_coredump.h"
#if CFG_SUPPORT_CONNINFRA
#include "connsys_debug_utility.h"
#elif IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
#include "connv3.h"
#include "connv3_debug_utility.h"
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_SUPPORT_CONNINFRA
static int coredump_check_reg_readable(void);
#endif
static int file_ops_coredump_open(struct inode *inode, struct file *file);
static int file_ops_coredump_release(struct inode *inode, struct file *file);
static int file_ops_coredump_mmap(struct file *file,
	struct vm_area_struct *vma);

#define PRINT_BYTES_PER_LOOP	512
#define PRINT_LONG_STR_MSG(__msg, __size) \
{ \
	uint8_t buf[PRINT_BYTES_PER_LOOP + 1]; \
	uint8_t *pos = __msg, *end = __msg + __size; \
	while (pos < end) { \
		uint32_t print_sz = (end - pos) > PRINT_BYTES_PER_LOOP ? \
			PRINT_BYTES_PER_LOOP : (end - pos); \
		kalMemCopy(buf, pos, print_sz); \
		buf[print_sz] = '\0'; \
		LOG_FUNC("%s\n", buf); \
		pos += print_sz; \
	} \
}

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
struct coredump_ctx g_coredump_ctx;

#if CFG_SUPPORT_CONNINFRA
struct coredump_event_cb g_wifi_coredump_cb = {
	.reg_readable = coredump_check_reg_readable,
	.poll_cpupcr = NULL,
};
#endif

const struct file_operations g_coredump_fops = {
	.open = file_ops_coredump_open,
	.release = file_ops_coredump_release,
	.mmap = file_ops_coredump_mmap,
};

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
#if CFG_SUPPORT_CONNINFRA
static int coredump_check_reg_readable(void)
{
	struct coredump_ctx *ctx = &g_coredump_ctx;
	int ret = 1;

	if (conninfra_reg_readable() == 0) {
		DBGLOG(INIT, ERROR,
			"conninfra_reg_readable check failed.\n");
		ret = 0;
	}

	if (ctx->fn_check_bus_hang) {
		if (ctx->fn_check_bus_hang(NULL, 0) != 0) {
			DBGLOG(INIT, ERROR, "bus check failed.\n");
			ret = 0;
		}
	}

	return ret;
}
#endif

static int file_ops_coredump_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int file_ops_coredump_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int file_ops_coredump_mmap(struct file *file,
	struct vm_area_struct *vma)
{
	struct mt66xx_chip_info *chip_info;
	unsigned long id = vma->vm_pgoff;
	unsigned long pfn;
	int ret = 0;

	glGetChipInfo((void **)&chip_info);
	if (!chip_info) {
		DBGLOG(INIT, ERROR, "chip info is NULL\n");
		ret = -EINVAL;
		goto exit;
	}

	switch (id) {
	case 0:
		DBGLOG(INIT, INFO, "Coredump mmap size: %lu\n",
			vma->vm_end - vma->vm_start);
		if (vma->vm_end - vma->vm_start >
		    emi_mem_get_size(chip_info)) {
			ret = -EINVAL;
			break;
		}
		pfn = emi_mem_get_phy_base(chip_info) >> PAGE_SHIFT;
		if (remap_pfn_range(vma,
				    vma->vm_start,
				    pfn,
				    vma->vm_end - vma->vm_start,
				    vma->vm_page_prot)) {
			ret = -EINVAL;
			break;
		}
		break;
	default:
		DBGLOG(INIT, WARN, "Invalid id: %lu\n", id);
		ret = -EINVAL;
		break;
	}

exit:
	return ret;
}

void coredump_register_bushang_chk_cb(bushang_chk_func_cb cb)
{
	struct coredump_ctx *ctx = &g_coredump_ctx;

	ctx->fn_check_bus_hang = cb;
}

static int coredump_add_cdev(struct coredump_ctx *ctx)
{
	int ret = 0;

	ret = alloc_chrdev_region(&ctx->devno, 0, 1,
		COREDUMP_WIFI_INF_NAME);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"alloc_chrdev_region failed, ret: %d\n",
			ret);
		goto return_fn;
	}

	cdev_init(&ctx->cdev, &g_coredump_fops);
	ctx->cdev.owner = THIS_MODULE;

	ret = cdev_add(&ctx->cdev, ctx->devno, COREDUMP_WIFI_DEV_NUM);
	if (ret) {
		DBGLOG(INIT, ERROR,
			"cdev_add failed, ret: %d\n",
			ret);
		goto unregister_chrdev_region;
	}

	ctx->driver_class = class_create(THIS_MODULE,
		COREDUMP_WIFI_INF_NAME);
	if (IS_ERR(ctx->driver_class)) {
		DBGLOG(INIT, ERROR,
			"class_create failed, ret: %d\n",
			ret);
		ret = PTR_ERR(ctx->driver_class);
		goto cdev_del;
	}

	ctx->class_dev = device_create(ctx->driver_class,
		NULL, ctx->devno, NULL, COREDUMP_WIFI_INF_NAME);
	if (IS_ERR(ctx->class_dev)) {
		ret = PTR_ERR(ctx->class_dev);
		DBGLOG(INIT, ERROR,
			"class_device_create failed, ret: %d\n",
			ret);
		goto class_destroy;
	}

	goto return_fn;

class_destroy:
	class_destroy(ctx->driver_class);
cdev_del:
	cdev_del(&ctx->cdev);
unregister_chrdev_region:
	unregister_chrdev_region(ctx->devno, COREDUMP_WIFI_DEV_NUM);
return_fn:
	if (ret)
		DBGLOG(INIT, ERROR, "ret: %d\n",
			ret);

	return ret;
}

static void coredump_del_cdev(struct coredump_ctx *ctx)
{
	device_destroy(ctx->driver_class, ctx->devno);
	class_destroy(ctx->driver_class);
	cdev_del(&ctx->cdev);
	unregister_chrdev_region(ctx->devno, COREDUMP_WIFI_DEV_NUM);
}

int wifi_coredump_init(void *priv)
{
	struct coredump_ctx *ctx = &g_coredump_ctx;
	struct mt66xx_chip_info *chip_info;
#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	struct connv3_coredump_event_cb cb;
#endif
	int ret = 0;

	kalMemZero(ctx, sizeof(*ctx));

	ctx->priv = priv;
	glGetChipInfo((void **)&chip_info);
	if (!chip_info) {
		DBGLOG(INIT, ERROR, "chip info is NULL\n");
		ret = -EINVAL;
		goto exit;
	}

	ret = coredump_add_cdev(ctx);
	if (ret)
		goto exit;

#if CFG_SUPPORT_CONNINFRA
	ctx->handler = connsys_coredump_init(CONN_DEBUG_TYPE_WIFI,
		&g_wifi_coredump_cb);
	if (!ctx->handler) {
		DBGLOG(INIT, ERROR, "connsys_coredump_init init failed.\n");
		ret = -EINVAL;
		goto free_cdev;
	}
#elif IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	kalMemZero(&cb, sizeof(cb));
	kalSnprintf(cb.dev_node, CONNV3_EMI_MAP_DEV_NODE_SIZE,
		"%s%s", "/dev/", COREDUMP_WIFI_INF_NAME);
	cb.emi_size = MCU_WF_EMI_SIZE;
	cb.mcif_emi_size = 0;
	ctx->handler = connv3_coredump_init(CONNV3_DEBUG_TYPE_WIFI,
		&cb);
	if (!ctx->handler) {
		DBGLOG(INIT, ERROR, "connv3_coredump_init init failed.\n");
		ret = -EINVAL;
		goto free_cdev;
	}
#endif

	ctx->initialized = TRUE;
	goto exit;

free_cdev:
	coredump_del_cdev(ctx);
exit:
	return ret;
}

void wifi_coredump_deinit(void)
{
	struct coredump_ctx *ctx = &g_coredump_ctx;

	ctx->initialized = FALSE;

#if CFG_SUPPORT_CONNINFRA
	connsys_coredump_deinit(ctx->handler);
#elif IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	connv3_coredump_deinit(ctx->handler);
#endif

	ctx->handler = NULL;
	coredump_del_cdev(ctx);
	ctx->fn_check_bus_hang = NULL;
	ctx->priv = NULL;
}

static int __coredump_init_ctrl_blk(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	struct ctrl_blk_layout ctrl_blk;
	int ret = 0;

	ret = emi_mem_read(chip_info,
		COREDUMP_OFFSET_CTRL_BLOCK,
		&ctrl_blk,
		sizeof(ctrl_blk));
	if (ret) {
		DBGLOG(INIT, ERROR,
			"Read CTRL BLK failed.\n");
		goto exit;
	}

	mem->state = ctrl_blk.state;

	mem->print_buff_offset = COREDUMP_OFFSET_CTRL_BLOCK +
		COREDUMP_MAX_BLOCK_SZ_CTRL_BLOCK +
		COREDUMP_MAX_BLOCK_SZ_DEBUG_BUFF;
	if (ctrl_blk.print_buff_len > COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF)
		mem->print_buff_len = COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF;
	else
		mem->print_buff_len = ctrl_blk.print_buff_len;

	mem->dump_buff_offset = mem->print_buff_offset +
		COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF;
	if (ctrl_blk.dump_buff_len > COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF)
		mem->dump_buff_len = COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF;
	else
		mem->dump_buff_len = ctrl_blk.dump_buff_len;

	mem->cr_region_offset = mem->dump_buff_offset +
		COREDUMP_MAX_BLOCK_SZ_DUMP_BUFF;
	if (ctrl_blk.cr_region_len > COREDUMP_MAX_BLOCK_SZ_CR_REGION)
		mem->cr_region_num = COREDUMP_MAX_BLOCK_SZ_CR_REGION >> 3;
	else
		mem->cr_region_num = ctrl_blk.cr_region_len >> 3;
	mem->cr_region_num = mem->cr_region_num;

	mem->mem_region_offset = COREDUMP_OFFSET_CTRL_BLOCK +
		sizeof(struct ctrl_blk_layout);
	if (ctrl_blk.mem_region_num > COREDUMP_MAX_BLOCK_SZ_MEM_REGION)
		mem->mem_region_num = COREDUMP_MAX_BLOCK_SZ_MEM_REGION;
	else
		mem->mem_region_num = ctrl_blk.mem_region_num;

	DBGLOG(INIT, INFO,
		"pdcm[0x%x 0x%x 0x%x 0x%x]\n",
		ctrl_blk.print_buff_len,
		ctrl_blk.dump_buff_len,
		ctrl_blk.cr_region_len,
		ctrl_blk.mem_region_num);
	DBGLOG(INIT, INFO,
		"pdcm_offset[0x%x 0x%x 0x%x 0x%x] pdcm_len[0x%x 0x%x 0x%x 0x%x]\n",
		mem->print_buff_offset,
		mem->dump_buff_offset,
		mem->cr_region_offset,
		mem->mem_region_offset,
		mem->print_buff_len,
		mem->dump_buff_len,
		mem->cr_region_num,
		mem->mem_region_num);

exit:
	return ret;
}

static int __coredump_wait_done(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	unsigned long timeout;
	int ret = 0;

	timeout = jiffies + msecs_to_jiffies(COREDUMP_TIMEOUT);

	while (TRUE) {
		struct ctrl_blk_layout ctrl_blk = {0};

		if (time_after(jiffies, timeout)) {
			DBGLOG(INIT, ERROR, "Coredump timeout.\n");
			DBGLOG_MEM32(INIT, ERROR, &ctrl_blk, sizeof(ctrl_blk));
			ret = -ETIMEDOUT;
			break;
		}

		ret = emi_mem_read(chip_info,
			COREDUMP_OFFSET_CTRL_BLOCK,
			&ctrl_blk,
			sizeof(ctrl_blk));
		if (ret) {
			DBGLOG(INIT, ERROR,
				"Read coredump state failed.\n");
			break;
		}

		if (ctrl_blk.state == COREDUMP_STATE_PUT_DONE) {
			ret = __coredump_init_ctrl_blk(ctx, chip_info);
			break;
		}
	}

	return ret;
}

static int __coredump_init_mem_region(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	struct mem_region *region;
	uint32_t idx = 0;
	int ret = 0;

	if (mem->mem_region_num == 0)
		goto exit;

	mem->mem_regions = kalMemAlloc(
		mem->mem_region_num * sizeof(struct mem_region),
		VIR_MEM_TYPE);
	if (!mem->mem_regions) {
		ret = -ENOMEM;
		goto exit;
	}

	for (idx = 0, region = mem->mem_regions;
	     idx < mem->mem_region_num;
	     idx++, region++) {
		struct mem_region_layout layout;
		uint32_t offset = mem->mem_region_offset +
			idx * sizeof(struct mem_region_layout);

		kalMemZero(&layout, sizeof(layout));
		ret = emi_mem_read(chip_info,
			offset,
			&layout,
			sizeof(struct mem_region_layout));
		if (ret) {
			DBGLOG(INIT, ERROR,
				"Read memory region failed, 0x%x 0x%x\n",
				offset,
				sizeof(struct mem_region_layout));
			goto exit;
		}
		kalMemCopy(region->name, layout.name, sizeof(layout.name));
		region->name[sizeof(layout.name)] = '\0';
		region->base = layout.base;
		region->size = layout.size;
		region->buf = kalMemAlloc(region->size, VIR_MEM_TYPE);
		if (!region->buf) {
			DBGLOG(INIT, ERROR,
				"[%d] Alloc buffer failed, size: 0x%x\n",
				idx,
				region->size);
			ret = -ENOMEM;
			goto exit;
		}
		DBGLOG(INIT, INFO,
			"[%d] name: %s, base: 0x%x, size: 0x%x\n",
			idx,
			region->name,
			region->base,
			region->size);
	}

exit:
	return ret;
}

static void __coredump_deinit_mem_region(struct coredump_ctx *ctx)
{
	struct coredump_mem *mem = &ctx->mem;
	struct mem_region *region;
	uint32_t idx = 0;

	if (mem->mem_regions && mem->mem_region_num) {
		for (idx = 0, region = mem->mem_regions;
		     idx < mem->mem_region_num;
		     idx++, region++) {
			if (region->buf)
				kalMemFree(region->buf,
					VIR_MEM_TYPE,
					region->size);
		}
		kalMemFree(mem->mem_regions,
			VIR_MEM_TYPE,
			mem->mem_region_num * sizeof(struct mem_region));
	}
}

static int __coredump_init_cr_region(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	struct cr_region *region;
	uint32_t idx = 0;
	int ret = 0;

	if (mem->cr_region_num == 0)
		goto exit;

	mem->cr_regions = kalMemAlloc(
		mem->cr_region_num * sizeof(struct cr_region),
		VIR_MEM_TYPE);
	if (!mem->cr_regions) {
		ret = -ENOMEM;
		goto exit;
	}

	for (idx = 0, region = mem->cr_regions;
	     idx < mem->cr_region_num;
	     idx++, region++) {
		struct cr_region_layout layout;
		uint32_t offset = mem->cr_region_offset +
			idx * sizeof(struct cr_region_layout);

		kalMemZero(&layout, sizeof(layout));
		ret = emi_mem_read(chip_info,
			offset,
			&layout,
			sizeof(struct cr_region_layout));
		if (ret) {
			DBGLOG(INIT, ERROR,
				"Read cr region failed, 0x%x 0x%x\n",
				offset,
				sizeof(struct cr_region_layout));
			goto exit;
		}
		region->base = layout.base;
		region->size = layout.size;

		if (region->base == 0xFFFFFFFF || region->size == 0)
			break;

		region->buf = kalMemAlloc(layout.size, VIR_MEM_TYPE);
		if (!region->buf) {
			DBGLOG(INIT, ERROR,
				"[%d] Alloc buffer failed, size: 0x%x\n",
				idx,
				layout.size);
			ret = -ENOMEM;
			goto exit;
		}
	}

exit:
	return ret;
}

static void __coredump_deinit_cr_region(struct coredump_ctx *ctx)
{
	struct coredump_mem *mem = &ctx->mem;
	struct cr_region *region;
	uint32_t idx = 0;

	if (mem->cr_regions && mem->cr_region_num) {
		for (idx = 0, region = mem->cr_regions;
		     idx < mem->cr_region_num;
		     idx++, region++) {
			if (region->buf)
				kalMemFree(region->buf,
					VIR_MEM_TYPE,
					region->size);
		}
		kalMemFree(mem->cr_regions,
			VIR_MEM_TYPE,
			mem->cr_region_num * sizeof(struct cr_region));
	}
}

static int __coredump_init(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	int ret = 0;

	if (mem->print_buff_len) {
		mem->print_buff = kalMemAlloc(
			mem->print_buff_len,
			VIR_MEM_TYPE);
		if (!mem->print_buff) {
			ret = -ENOMEM;
			goto exit;
		}
	}

	if (mem->dump_buff_len) {
		mem->dump_buff = kalMemAlloc(
			mem->dump_buff_len,
			VIR_MEM_TYPE);
		if (!mem->dump_buff) {
			ret = -ENOMEM;
			goto exit;
		}
	}

	ret = __coredump_init_cr_region(ctx, chip_info);
	if (ret)
		goto exit;

	ret = __coredump_init_mem_region(ctx, chip_info);
	if (ret)
		goto exit;

	return 0;

exit:
	__coredump_deinit_mem_region(ctx);

	__coredump_deinit_cr_region(ctx);

	if (mem->dump_buff && mem->dump_buff_len)
		kalMemFree(mem->dump_buff,
			VIR_MEM_TYPE,
			mem->dump_buff_len);

	if (mem->print_buff && mem->print_buff_len)
		kalMemFree(mem->print_buff,
			VIR_MEM_TYPE,
			mem->print_buff_len);

	return ret;
}

static void __coredump_deinit(struct coredump_ctx *ctx)
{
	struct coredump_mem *mem = &ctx->mem;

	if (mem->print_buff && mem->print_buff_len)
		kalMemFree(mem->print_buff,
			VIR_MEM_TYPE,
			mem->print_buff_len);

	if (mem->dump_buff && mem->dump_buff_len)
		kalMemFree(mem->dump_buff,
			VIR_MEM_TYPE,
			mem->dump_buff_len);

	__coredump_deinit_cr_region(ctx);

	__coredump_deinit_mem_region(ctx);
}

static int __coredump_handle_print_buff(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	int ret = 0;

	if (mem->print_buff_len == 0)
		goto exit;

	ret = emi_mem_read(chip_info,
		mem->print_buff_offset,
		mem->print_buff,
		mem->print_buff_len);
	if (ret)
		goto exit;

exit:
	return ret;
}

static int __coredump_handle_dump_buff(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
#define PRINT_MSG_END		";;[CONNSYS] coredump start .."

	struct coredump_mem *mem = &ctx->mem;
	uint8_t *pos = NULL, *msg = NULL;
	uint32_t size = 0;
	int ret = 0;

	if (mem->dump_buff_len == 0)
		goto exit;

	ret = emi_mem_read(chip_info,
		mem->dump_buff_offset,
		mem->dump_buff,
		mem->dump_buff_len);
	if (ret)
		goto exit;

	pos = kalStrStr(mem->dump_buff, PRINT_MSG_END);
	if (!pos) {
		DBGLOG(INIT, ERROR,
			"Can NOT find pattern %s\n",
			PRINT_MSG_END);
		goto exit;
	}

	size = pos - mem->dump_buff;
	msg = kalMemAlloc(size, VIR_MEM_TYPE);
	if (!msg) {
		DBGLOG(INIT, ERROR,
			"Alloc debug msg failed.\n");
		goto exit;
	}

	kalMemCopy(msg, mem->dump_buff, size);
	DBGLOG(INIT, INFO, "++ Coredump message ++\n");
	PRINT_LONG_STR_MSG(msg, size);
	DBGLOG(INIT, INFO, "-- Coredump message --\n");

exit:
	if (msg)
		kalMemFree(msg, VIR_MEM_TYPE, size);

	return ret;
}

static int __coredump_handle_cr_region(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	struct GLUE_INFO *glue = ctx->priv;
	struct cr_region *region;
	uint32_t idx = 0;
	int ret = 0;

	if (mem->cr_region_num == 0)
		goto exit;

	for (idx = 0, region = mem->cr_regions;
	     idx < mem->cr_region_num;
	     idx++, region++) {
		if (region->base == 0xFFFFFFFF || region->size == 0)
			break;

		if (kalDevRegReadRange(glue,
				       region->base,
				       region->buf,
				       region->size) == FALSE) {
			DBGLOG(INIT, ERROR,
				"Read mem region failed, 0x%x 0x%x\n",
				region->base,
				region->size);
			ret = -ENOMEM;
			break;
		}
	}

exit:
	return ret;
}

static int __coredump_handle_mem_region(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info)
{
	struct coredump_mem *mem = &ctx->mem;
	struct GLUE_INFO *glue = ctx->priv;
	struct mem_region *region;
	uint32_t idx = 0;
	int ret = 0;

	for (idx = 0, region = mem->mem_regions;
	     idx < mem->mem_region_num;
	     idx++, region++) {
		if (kalDevRegReadRange(glue,
				       region->base,
				       region->buf,
				       region->size) == FALSE) {
			DBGLOG(INIT, ERROR,
				"[%d] Read mem region failed, %s 0x%x 0x%x\n",
				idx,
				region->name,
				region->base,
				region->size);
			ret = -ENOMEM;
			break;
		}

		DBGLOG(INIT, INFO,
			"[%d] mem region %s 0x%x 0x%x\n",
			idx,
			region->name,
			region->base,
			region->size);
	}

	return ret;
}

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
static int __coredump_to_userspace_cr_region(struct coredump_ctx *ctx)
{
/* "[%08x,%08x]" */
#define BUFF_SZ_PER_CR		19

	struct coredump_mem *mem = &ctx->mem;
	struct cr_region *cr_region;
	uint32_t i = 0, j = 0, total_sz = 0;
	uint8_t *buf, *pos;
	int ret = 0;

	/* calculate log buffer size */
	for (i = 0, cr_region = mem->cr_regions;
	     i < mem->cr_region_num;
	     i++, cr_region++)
		total_sz += cr_region->size;
	total_sz *= BUFF_SZ_PER_CR;

	buf = kalMemAlloc(total_sz, VIR_MEM_TYPE);
	if (!buf) {
		DBGLOG(INIT, ERROR,
			"Alloc cr region log buffer failed.\n");
		ret = -ENOMEM;
		goto exit;
	}
	for (i = 0, cr_region = mem->cr_regions, pos = buf;
	     i < mem->cr_region_num;
	     i++, cr_region++) {
		if (cr_region->base == 0xFFFFFFFF || cr_region->size == 0)
			break;

		for (j = 0; j < cr_region->size / 4; j++) {
			uint32_t addr = 0, val = 0;

			addr = cr_region->base + j * 4;
			val = *((uint32_t *)(cr_region->buf + j * 4));

			*pos++ = '[';
			kalMemCopy(pos,
				   &addr,
				   sizeof(addr));
			pos += 4;
			*pos++ = ',';
			kalMemCopy(pos,
				   &val,
				   sizeof(val));
			pos += 4;
			*pos++ = ']';
		}
	}

	/* send to userspace */
	connv3_coredump_send(ctx->handler,
			     "[M]",
			     buf,
			     total_sz);

exit:
	if (buf)
		kalMemFree(buf, VIR_MEM_TYPE, total_sz);

	return ret;
}

static void __coredump_to_userspace_aee_str(struct coredump_ctx *ctx,
	struct connv3_issue_info *issue_info,
	uint8_t *aee_str,
	uint32_t aee_str_len)
{
	struct coredump_mem *mem = &ctx->mem;
	uint8_t *start = NULL, *end = NULL;
	int32_t written = 0;

	switch (issue_info->issue_type) {
	case CONNV3_ISSUE_DRIVER_ASSERT:
		written += kalSnprintf(aee_str + written,
			aee_str_len - written,
			"<DRIVER> WFSYS %s: %s",
			issue_info->task_name,
			issue_info->reason);
		break;
	case CONNV3_ISSUE_FW_ASSERT:
	{
		u_int8_t casan = FALSE;

		casan = kalStrStr(mem->dump_buff, "_asan.c") != NULL;
		if (casan) {
			written += kalSnprintf(aee_str + written,
				aee_str_len - written,
				"<CASAN> WFSYS ");
		} else {
			written += kalSnprintf(aee_str + written,
				aee_str_len - written,
				"<ASSERT> WFSYS ");
		}

		start = kalStrStr(mem->dump_buff, "<ASSERT> ");
		if (start)
			end = kalStrStr(start, "\n");

		if (start && end)
			written += kalSnprintf(aee_str + written,
				end - start - kalStrLen("<ASSERT> ") + 1,
				"%s",
				start + kalStrLen("<ASSERT> "));
		else
			written += kalSnprintf(aee_str + written,
				end - start,
				"UNKNOWN");
	}
		break;
	case CONNV3_ISSUE_FW_EXCEPTION:
	{
		written += kalSnprintf(aee_str + written,
			aee_str_len - written,
			"<EXCEPTION> WFSYS ");

		start = kalStrStr(mem->dump_buff, "<EXCEPTION> ");
		if (start)
			end = kalStrStr(start, "\n");

		if (start && end)
			written += kalSnprintf(aee_str + written,
				end - start - kalStrLen("<EXCEPTION> ") + 1,
				"%s",
				start + kalStrLen("<EXCEPTION> "));
		else
			written += kalSnprintf(aee_str + written,
				end - start,
				"UNKNOWN");
	}
		break;
	default:
		written += kalSnprintf(aee_str + written,
			aee_str_len - written,
			"UNKNOWN");
		break;
	}

	DBGLOG(INIT, INFO, "aee_str: %s\n", aee_str);
}

static void __coredump_to_userspace_issue_info(struct coredump_ctx *ctx,
	uint8_t *aee_str,
	uint32_t aee_str_len)
{
#define BUF_SIZE	2048

	struct coredump_mem *mem = &ctx->mem;
	struct connv3_issue_info issue_info;
	struct mem_region *mem_region;
	uint32_t idx = 0;
	int32_t written = 0;
	uint8_t *buf, *pos;

	kalMemZero(&issue_info, sizeof(issue_info));
	buf = kalMemAlloc(BUF_SIZE, VIR_MEM_TYPE);
	if (!buf) {
		DBGLOG(INIT, ERROR, "Alloc buffer failed.\n");
		return;
	}
	kalMemZero(buf, BUF_SIZE);

	connv3_coredump_get_issue_info(ctx->handler,
				       &issue_info,
				       buf,
				       BUF_SIZE);

	__coredump_to_userspace_aee_str(ctx, &issue_info, aee_str,
		aee_str_len);

	pos = kalStrStr(buf, "</main>");
	if (!pos) {
		DBGLOG(INIT, ERROR, "No </main> pattern from %s\n",
			buf);
		goto exit;
	}

	written += kalSnprintf(pos + written,
			       BUF_SIZE - written,
			       "\t<map>\n");
	for (idx = 0, mem_region = mem->mem_regions;
	     idx < mem->mem_region_num;
	     idx++, mem_region++) {
		written += kalSnprintf(pos + written,
				       BUF_SIZE - written,
				       "\t\t<%s>\n",
				       mem_region->name);
		written += kalSnprintf(pos + written,
				       BUF_SIZE - written,
				       "\t\t\t<offset>0x%x</offset>\n",
				       mem_region->base);
		written += kalSnprintf(pos + written,
				       BUF_SIZE - written,
				       "\t\t\t<size>0x%x</size>\n",
				       mem_region->size);
		written += kalSnprintf(pos + written,
				       BUF_SIZE - written,
				       "\t\t</%s>\n",
				       mem_region->name);
	}
	written += kalSnprintf(pos + written,
			       BUF_SIZE - written,
			       "\t</map>\n");
	written += kalSnprintf(pos + written,
			       BUF_SIZE - written,
			       "</main>\n");

	connv3_coredump_send(ctx->handler,
			     "INFO",
			     buf,
			     kalStrLen(buf));

exit:
	kalMemFree(buf, VIR_MEM_TYPE, BUF_SIZE);
}

static int __coredump_to_userspace_mem_region(struct coredump_ctx *ctx)
{
	struct coredump_mem *mem = &ctx->mem;
	struct mem_region *mem_region;
	uint32_t idx = 0;

	for (idx = 0, mem_region = mem->mem_regions;
	     idx < mem->mem_region_num;
	     idx++, mem_region++)
		connv3_coredump_send(ctx->handler,
				     mem_region->name,
				     mem_region->buf,
				     mem_region->size);

	return 0;
}

static int __coredump_to_userspace(struct coredump_ctx *ctx,
	struct mt66xx_chip_info *chip_info,
	unsigned int drv, char *reason)
{
	struct coredump_mem *mem = &ctx->mem;
	struct GLUE_INFO *glue = ctx->priv;
	uint8_t fw_version[256];
	uint8_t aee_str[256];
	uint32_t u4Len = 0;

	if (glue->u4ReadyFlag == 0) {
		if (chip_info->fw_dl_ops->getFwVerInfo)
			chip_info->fw_dl_ops->getFwVerInfo(fw_version,
				&u4Len,
				sizeof(fw_version));
		else
			DBGLOG(INIT, ERROR, "NULL getFwVerInfo\n");
	} else {
		struct ADAPTER *prAdapter = glue->prAdapter;
		struct WIFI_VER_INFO *prVerInfo = &prAdapter->rVerInfo;

		if (kalStrLen(prVerInfo->aucReleaseManifest)) {
			u4Len = kalStrLen(prVerInfo->aucReleaseManifest);
			kalSnprintf(fw_version,
				    u4Len,
				    "%s",
				    prVerInfo->aucReleaseManifest);
		} else {
			u4Len = kalStrLen(
				prVerInfo->rCommonTailer.aucRamBuiltDate);
			kalSnprintf(fw_version,
				    u4Len,
				    "%s",
				    prVerInfo->rCommonTailer.aucRamBuiltDate);
		}
	}

	connv3_coredump_start(ctx->handler,
			      drv,
			      reason,
			      mem->dump_buff,
			      fw_version);

	__coredump_to_userspace_cr_region(ctx);

	__coredump_to_userspace_issue_info(ctx,
		aee_str,
		sizeof(aee_str));

	__coredump_to_userspace_mem_region(ctx);

	connv3_coredump_end(ctx->handler, aee_str);

	return 0;
}
#endif

static int __coredump_start(struct coredump_ctx *ctx,
	unsigned int drv, char *reason)
{
	struct coredump_mem *mem = &ctx->mem;
	struct GLUE_INFO *glue = ctx->priv;
	struct mt66xx_chip_info *chip_info;
	int ret = 0;

	if (glue->u4ReadyFlag == 0) {
		DBGLOG(INIT, WARN, "Skip coredump due to NOT ready.\n");
		goto exit;
	}

	glGetChipInfo((void **)&chip_info);
	if (!chip_info) {
		DBGLOG(INIT, ERROR, "chip info is NULL\n");
		goto exit;
	}

	kalMemZero(mem, sizeof(*mem));

	ret = __coredump_wait_done(ctx, chip_info);
	if (ret)
		goto exit;

	ret = __coredump_init(ctx, chip_info);
	if (ret)
		goto exit;

	ret = __coredump_handle_print_buff(ctx, chip_info);
	if (ret)
		goto deinit;

	ret = __coredump_handle_dump_buff(ctx, chip_info);
	if (ret)
		goto deinit;

	ret = __coredump_handle_cr_region(ctx, chip_info);
	if (ret)
		goto deinit;

	ret = __coredump_handle_mem_region(ctx, chip_info);
	if (ret)
		goto deinit;

#if IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	__coredump_to_userspace(ctx, chip_info, drv, reason);
#endif

deinit:
	__coredump_deinit(ctx);
exit:
	return ret;
}

void wifi_coredump_start(uint8_t drv, char *reason)
{
	struct coredump_ctx *ctx = &g_coredump_ctx;

	if (!ctx->initialized) {
		DBGLOG(INIT, WARN,
			"Skip coredump due to NOT initialized.\n");
		return;
	}

	DBGLOG(INIT, INFO,
		"drv: 0x%x, reason: %s\n",
		drv, reason);

#if CFG_SUPPORT_CONNINFRA
	{
		enum consys_drv_type drv_type;

		switch (drv) {
		case RST_SOURCE_WIFI_DRIVER:
			drv_type = CONNDRV_TYPE_WIFI;
			break;
		case RST_SOURCE_WIFI_FW:
			drv_type = (enum consys_drv_type)-1;
			break;
		default:
			drv_type = (enum consys_drv_type)drv;
			break;
		}
		connsys_coredump_start(g_coredump_ctx.handler, 0,
			drv_type, reason);
		connsys_coredump_clean(g_coredump_ctx.handler);
	}
#elif IS_ENABLED(CFG_MTK_WIFI_CONNV3_SUPPORT)
	__coredump_start(&g_coredump_ctx, drv, reason);
#endif
}

