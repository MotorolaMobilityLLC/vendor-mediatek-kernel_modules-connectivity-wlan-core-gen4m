/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "precomp.h"

int32_t kalGetFwFlavor(uint8_t *flavor)
{
#ifdef CFG_WIFI_FLAVOR
	snprintf(flavor, 2, "%s",
			STR(CFG_WIFI_FLAVOR));
	return 1;
#else
	return 0;
#endif
}
