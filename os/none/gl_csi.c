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
#include "gl_csi.h"

#if CFG_SUPPORT_CSI
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

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static struct CSI_INFO_T rCSIInfo;

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
struct CSI_INFO_T *glCsiGetCSIInfo(void)
{
	return &rCSIInfo;
}

void glCsiSupportInit(IN struct GLUE_INFO *prGlueInfo)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

void glCsiSupportDeinit(IN struct GLUE_INFO *prGlueInfo)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

int32_t glCsiAddSta(IN struct GLUE_INFO *prGlueInfo,
	struct CMD_CSI_CONTROL_T *prCSICtrl)
{
	return KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

int32_t glCsiDelSta(IN struct GLUE_INFO *prGlueInfo,
	struct CMD_CSI_CONTROL_T *prCSICtrl)
{
	return KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

void glCsiFreeStaList(IN struct GLUE_INFO *prGlueInfo)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

void nicEventCSIData(IN struct ADAPTER *prAdapter,
			IN struct WIFI_EVENT *prEvent)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

u_int8_t
wlanPushCSIData(IN struct ADAPTER *prAdapter, struct CSI_DATA_T *prCSIData)
{
	return KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

u_int8_t
wlanPopCSIData(IN struct ADAPTER *prAdapter, struct CSI_DATA_T *prCSIData)
{
	return KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

ssize_t wlanCSIDataPrepare(
	uint8_t *buf,
	struct CSI_INFO_T *prCSIInfo,
	struct CSI_DATA_T *prCSIData)
{
	return KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

/*
*CSI TONE MASK
*this function mask(clear) the null tone && pilot tones
*for example, bw20 has  64 tones in total.however there
*are some null tone && pilot tones we do not need for the
*feature of channel state information(CSI).
*so we need to clear these tone.
*/
void wlanApplyCSIToneMask(
	uint8_t ucRxMode,
	uint8_t ucCBW,
	uint8_t ucDBW,
	uint8_t ucPrimaryChIdx,
	int16_t *ai2IData,
	int16_t *ai2QData)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}

void
wlanShiftCSI(
	uint8_t ucRxMode,
	uint8_t ucCBW,
	uint8_t ucDBW,
	uint8_t ucPrimaryChIdx,
	int16_t *ai2IData,
	int16_t *ai2QData,
	int16_t *ai2ShiftIData,
	int16_t *ai2ShiftQData)
{
	KAL_NEED_IMPLEMENT(__FILE__, __func__, __LINE__);
}
#endif /* CFG_SUPPORT_CSI */
