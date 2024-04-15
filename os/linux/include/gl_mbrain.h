/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _GL_MBRAIN_H
#define _GL_MBRAIN_H

#if CFG_SUPPORT_MBRAIN
#include "bridge/mbraink_bridge.h"

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */


/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/* function pointer to copy data to mbrain */
typedef enum wifi2mbr_status (*PFN_WIFI2MBR_HANDLER) (struct ADAPTER*,
	enum wifi2mbr_tag, uint16_t, void *, uint16_t *);

/* function pointer to specify the amount of valid data */
typedef uint16_t (*PFN_WIFI2MBR_DATA_NUM) (struct ADAPTER*,
	enum wifi2mbr_tag);

/*
 * eTag: data tag
 * ucExpdLen: expected data length
 * pfnHandler: pointer to the function which should copy to the buffer
 * pfnGetDataNum: pointer to the function which should
 *                specify the amount of valid data
 */
struct wifi2mbr_handler {
	enum wifi2mbr_tag eTag;
	uint8_t ucExpdLen;
	PFN_WIFI2MBR_HANDLER pfnHandler;
	PFN_WIFI2MBR_DATA_NUM pfnGetDataNum;
};

/*******************************************************************************
 *                  F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

void glRegCbsToMbraink(struct ADAPTER *prAdapter);
void glUnregCbsToMbraink(void);

enum wifi2mbr_status mbraink2wifi_get_data(void *priv,
						enum mbr2wifi_reason reason,
						enum wifi2mbr_tag tag,
						void *buf,
						uint16_t *pu2Len);

#endif /* CFG_SUPPORT_MBRAIN */
#endif /* _GL_MBRAIN_H */
