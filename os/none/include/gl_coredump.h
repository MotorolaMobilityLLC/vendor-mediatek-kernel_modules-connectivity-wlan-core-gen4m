/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _GL_COREDUMP_H
#define _GL_COREDUMP_H

#define COREDUMP_TIMEOUT			(10 * 1000)

#define COREDUMP_OFFSET_CTRL_BLOCK		0x0

#define COREDUMP_MAX_BLOCK_SZ_CTRL_BLOCK	0x80
#define COREDUMP_MAX_BLOCK_SZ_DEBUG_BUFF	0x80
#define COREDUMP_MAX_BLOCK_SZ_PRINT_BUFF	0x7F00
#define COREDUMP_MAX_BLOCK_SZ_DUMP_BUFF		0x8000
#define COREDUMP_MAX_BLOCK_SZ_CR_REGION		0x8000
#define COREDUMP_MAX_BLOCK_SZ_MEM_REGION	0x64

#define COREDUMP_TOTAL_SZ			(96 * 1024)

#define COREDUMP_WIFI_INF_NAME			"coredump_wifi"
#define COREDUMP_WIFI_DEV_NUM			1

typedef int (*bushang_chk_func_cb)(void *, uint8_t);

enum COREDUMP_CTRL_BLK_OFFSET {
	CTRL_BLK_OFFSET_STATE             = 0x4,
	CTRL_BLK_OFFSET_OUTBAND_ASSERT_W1 = 0x8,
	CTRL_BLK_OFFSET_PRINT_BUFF        = 0xC,
	CTRL_BLK_OFFSET_DUMP_BUFF         = 0x10,
	CTRL_BLK_OFFSET_CR_REGION         = 0x14,
	CTRL_BLK_OFFSET_MEM_REGION        = 0x18
};

enum COREDUMP_STATE {
	COREDUMP_STATE_NOT_START = 0,
	COREDUMP_STATE_PUTTING = 1,
	COREDUMP_STATE_PUT_DONE = 2,
	COREDUMP_STATE_NUM
};

struct mem_region {
	uint8_t name[5];
	uint32_t base;
	uint32_t size;
	uint8_t *buf;
};

struct cr_region {
	uint32_t base;
	uint32_t size;
	uint8_t *buf;
};

struct mem_region_layout {
	uint8_t name[4];
	uint32_t base;
	uint32_t size;
};

struct cr_region_layout {
	uint32_t base;
	uint32_t size;
};

__KAL_ATTRIB_PACKED_FRONT__
struct ctrl_blk_layout {
	uint32_t reserved;
	enum COREDUMP_STATE state;
	uint32_t outband_assert_w1;
	uint32_t print_buff_len;
	uint32_t dump_buff_len;
	uint32_t cr_region_len;
	uint32_t mem_region_num;
} __KAL_ATTRIB_PACKED__;

struct coredump_mem {
	enum COREDUMP_STATE state;
	uint8_t *print_buff;
	uint32_t print_buff_len;
	uint32_t print_buff_offset;
	uint8_t *dump_buff;
	uint32_t dump_buff_len;
	uint32_t dump_buff_offset;
	struct cr_region *cr_regions;
	uint32_t cr_region_num;
	uint32_t cr_region_offset;
	struct mem_region *mem_regions;
	uint32_t mem_region_num;
	uint32_t mem_region_offset;
};

struct coredump_ctx {
	void *priv;
	void *handler;
	u_int8_t initialized;
	bushang_chk_func_cb fn_check_bus_hang;
	struct coredump_mem mem;
};

#if CFG_WIFI_COREDUMP_SUPPORT
int wifi_coredump_init(void *priv);
void wifi_coredump_deinit(void);
void wifi_coredump_start(uint8_t drv, char *reason);
void coredump_register_bushang_chk_cb(bushang_chk_func_cb cb);
#else
static inline int wifi_coredump_init(void *priv)
{ return 0; }
static inline void wifi_coredump_deinit(void) {}
static inline void wifi_coredump_start(uint8_t drv, char *reason) {}
static inline void coredump_register_bushang_chk_cb(bushang_chk_func_cb cb) {}
#endif

#endif /* _GL_COREDUMP_H */
