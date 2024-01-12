/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _FW_LOG_MMIO_H
#define _FW_LOG_MMIO_H

#if (CFG_SUPPORT_CONNINFRA == 1)
#include "connsys_debug_utility.h"
#endif

enum ENUM_FW_LOG_CTRL_TYPE {
	ENUM_FW_LOG_CTRL_TYPE_MCU,
	ENUM_FW_LOG_CTRL_TYPE_WIFI,
	ENUM_FW_LOG_CTRL_TYPE_NUM,
};

__KAL_ATTRIB_PACKED_FRONT__
struct FW_LOG_COMMON_HEADER {
	uint32_t wifi_log_base_addr;
	uint32_t wifi_log_length;
	uint32_t mcu_log_base_addr;
	uint32_t mcu_log_length;
	uint32_t bt_log_base_addr;
	uint32_t bt_log_length;
	uint32_t gps_log_base_addr;
	uint32_t gps_log_length;
	uint8_t reserved[24];
	uint8_t key[8];
	uint8_t reserved2[32];
} __KAL_ATTRIB_PACKED__;

__KAL_ATTRIB_PACKED_FRONT__
struct FW_LOG_SUB_HEADER {
	uint32_t rp;
	uint32_t wp;
	uint32_t internal_wp;
	uint8_t reserved[20];
} __KAL_ATTRIB_PACKED__;

struct FW_LOG_MMIO_SUB_CTRL {
	enum ENUM_FW_LOG_CTRL_TYPE type;
	uint32_t base_addr;
	uint32_t buf_base_addr;
	uint32_t length;
	uint32_t rp; /* read pointer */
	uint32_t irp; /* internal read pointer, used by driver */
	uint32_t wp; /* write pointer */
	uint32_t iwp; /* internal write pointer, used by fw */
	uint8_t *buffer;
};

enum ENUM_LOG_READ_POINTER_PATH {
	ENUM_LOG_READ_POINTER_PATH_CMD,
	ENUM_LOG_READ_POINTER_PATH_CCIF,
	ENUM_LOG_READ_POINTER_PATH_NUM,
};

struct FW_LOG_MMIO_CTRL {
	enum ENUM_LOG_READ_POINTER_PATH ePath;
	u_int8_t initialized;
	u_int8_t started;
	u_int8_t defered;
	uint32_t base_addr;
	struct FW_LOG_MMIO_SUB_CTRL sub_ctrls[ENUM_FW_LOG_CTRL_TYPE_NUM];
	void *priv;
};

#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_MMIO)
int32_t fwLogMmioHandler(void);
void fwLogMmioStart(void);
void fwLogMmioStop(void);
uint32_t fwLogMmioInitMcu(struct ADAPTER *prAdapter);
void fwLogMmioDeInitMcu(void);
#else
static inline void fwLogMmioStart(void) {}
static inline void fwLogMmioStop(void) {}
static inline uint32_t fwLogMmioInitMcu(struct ADAPTER *prAdapter)
{
	return WLAN_STATUS_SUCCESS;
}
static inline void fwLogMmioDeInitMcu(void) {}
#endif

static inline int32_t fw_log_wifi_irq_handler(void)
{
#if IS_ENABLED(CFG_MTK_WIFI_FW_LOG_MMIO)
	return fwLogMmioHandler();
#elif (CFG_SUPPORT_CONNINFRA == 1)
	return connsys_log_irq_handler(CONN_DEBUG_TYPE_WIFI);
#else
	return 0;
#endif
}

#endif /* _FW_LOG_MMIO_H */

