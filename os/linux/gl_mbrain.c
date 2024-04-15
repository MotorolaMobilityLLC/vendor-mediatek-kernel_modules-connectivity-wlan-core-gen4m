// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

/*! \file   "gl_mbrain.c"
 *  \brief  This file defines the interface with Mbraink.
 *
 *    Detail description.
 */

/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */

#if CFG_SUPPORT_MBRAIN
#include "precomp.h"
#include "gl_mbrain.h"
/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

#define WIFI2MBR_TAG_RETRY_LIMIT 3
/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */

/* Table specified the mapping between tag and funciton pointers */
struct wifi2mbr_handler g_arMbrHdlr[] = {};

int32_t g_i4CurTag = -1;
uint16_t g_u2LeftLoopNum;
uint16_t g_u2LoopNum;

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/* call the corresponding handler and return data to mbraink
 * retry if the handler return failed.
 * if it's still failed after retry WIFI2MBR_TAG_RETRY_LIMIT times,
 * return fail to Mbraink.
 */
enum wifi2mbr_status mbraink2wifi_get_data(void *priv,
	enum mbr2wifi_reason reason,
	enum wifi2mbr_tag tag, void *buf, uint16_t *pu2Len)
{
	struct ADAPTER *prAdapter;
	int i;
	enum wifi2mbr_status eStatus = WIFI2MBR_FAILURE;
	void *tmpBuf;
	uint16_t u2CurTagRetryCnt = 0;

	prAdapter = (struct ADAPTER *)priv;
	if (unlikely(!prAdapter))
		return WIFI2MBR_FAILURE;

	if (g_i4CurTag != -1 && g_u2LeftLoopNum == 0) {
		g_i4CurTag = -1;
		return WIFI2MBR_END;
	}

	for (i = 0; i < ARRAY_SIZE(g_arMbrHdlr); i++) {
		if (g_arMbrHdlr[i].eTag != tag)
			continue;

		if (g_i4CurTag != tag) {
			g_u2LoopNum = g_arMbrHdlr[i].pfnGetDataNum(
				prAdapter, tag);
			if (g_u2LoopNum == 0)
				return WIFI2MBR_END;

			g_u2LeftLoopNum = g_u2LoopNum;
			g_i4CurTag = tag;

			DBGLOG(REQ, INFO, "reason:%u tag:%u loopNum:%u\n",
				reason, tag, g_u2LoopNum);
		}

get_data_retry:
		tmpBuf = kalMemAlloc(g_arMbrHdlr[i].ucExpdLen, VIR_MEM_TYPE);
		if (!tmpBuf) {
			DBGLOG(REQ, WARN,
				"Can not alloc memory for tag:%u\n", tag);
			return WIFI2MBR_FAILURE;
		}

		kalMemZero(tmpBuf, g_arMbrHdlr[i].ucExpdLen);

		eStatus = g_arMbrHdlr[i].pfnHandler(prAdapter, tag,
			g_u2LoopNum - g_u2LeftLoopNum,
			tmpBuf, pu2Len);

		if (eStatus == WIFI2MBR_SUCCESS &&
			*pu2Len <= g_arMbrHdlr[i].ucExpdLen) {
			kalMemCopy(buf, tmpBuf, g_arMbrHdlr[i].ucExpdLen);
		}

		DBGLOG_LIMITED(REQ, TRACE,
			"reason:%u tag:%u status:%u leftLoopNum:%u curLoopIdx:%u retry:%u\n",
			reason, tag, eStatus, g_u2LeftLoopNum,
			g_u2LoopNum - g_u2LeftLoopNum, u2CurTagRetryCnt);

		kalMemFree(tmpBuf, VIR_MEM_TYPE, g_arMbrHdlr[i].ucExpdLen);

		if (eStatus == WIFI2MBR_FAILURE) {
			if (u2CurTagRetryCnt < WIFI2MBR_TAG_RETRY_LIMIT) {
				u2CurTagRetryCnt++;
				goto get_data_retry;
			}
		}

		g_u2LeftLoopNum--;
		u2CurTagRetryCnt = 0;

		break;
	}

	return eStatus;
}

/* function to register callback to Mbraink when wifi on */
void glRegCbsToMbraink(struct ADAPTER *prAdapter)
{
	void (*pfn)(struct mbraink2wifi_ops *ops);
	struct mbraink2wifi_ops wifi2mbraink_ops = {
		.get_data = mbraink2wifi_get_data,
		.priv = (void *)prAdapter,
	};

	pfn = symbol_get(register_wifi2mbraink_ops);
	if (pfn == NULL) {
		DBGLOG(INIT, WARN, "symbol_get failed\n");
		return;
	}

	pfn(&wifi2mbraink_ops);
}

/* function to unregister callback to Mbraink when wifi off */
void glUnregCbsToMbraink(void)
{
	void (*pfn)(void);

	pfn = symbol_get(unregister_wifi2mbraink_ops);
	if (pfn == NULL) {
		DBGLOG(INIT, WARN, "symbol_get failed\n");
		return;
	}

	pfn();
}

#endif /* CFG_SUPPORT_MBRAIN */
