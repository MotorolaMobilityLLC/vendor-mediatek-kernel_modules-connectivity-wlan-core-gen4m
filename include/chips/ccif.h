/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 MediaTek Inc.
 */

#ifndef _CCIF_H
#define _CCIF_H

enum CCIF_MSG {
	SW_INT_FW_LOG = 0,
	SW_INT_SUBSYS_RESET,
	SW_INT_WHOLE_RESET,
	SW_INT_SW_WFDMA,
	SW_INT_TIME_SYNC,
};

struct CCIF_OPS {
	uint32_t (*get_interrupt_status)(struct ADAPTER *ad);
	void (*notify_utc_time_to_fw)(struct ADAPTER *ad,
		uint32_t sec,
		uint32_t usec);
};

static inline uint32_t ccif_get_interrupt_status(struct ADAPTER *ad)
{
	if (!ad || !ad->chip_info ||
	    !ad->chip_info->ccif_ops ||
	    !ad->chip_info->ccif_ops->get_interrupt_status)
		return 0;

	return ad->chip_info->ccif_ops->get_interrupt_status(ad);
}

static inline void ccif_notify_utc_time_to_fw(struct ADAPTER *ad,
	uint32_t sec,
	uint32_t usec)
{
	if (!ad || !ad->chip_info ||
	    !ad->chip_info->ccif_ops ||
	    !ad->chip_info->ccif_ops->notify_utc_time_to_fw)
		return;

	ad->chip_info->ccif_ops->notify_utc_time_to_fw(ad, sec, usec);
}

#endif
