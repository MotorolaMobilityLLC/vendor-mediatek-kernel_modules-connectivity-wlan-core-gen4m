/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

/*! \file   "qosmap.c"
 *    \brief  This file including the qosmap related function.
 *
 *    This file provided the macros and functions library support for the
 *    protocol layer qosmap related function.
 *
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

#if CFG_SUPPORT_PPR2

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */

/*******************************************************************************
 *                             D A T A   T Y P E S
 *******************************************************************************
 */

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */



/*******************************************************************************
 *                                 M A C R O S
 *******************************************************************************
 */

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

/*----------------------------------------------------------------------------*/
/*!
 *
 * \brief This routine is called to process the qos category action frame.
 *
 *
 * \note
 *      Called by: Handle Rx mgmt request
 */
/*----------------------------------------------------------------------------*/
void handleQosMapConf(struct ADAPTER *prAdapter, struct SW_RFB *prSwRfb)
{
	struct WLAN_ACTION_FRAME *prRxFrame;

	ASSERT(prAdapter);
	ASSERT(prSwRfb);

	prRxFrame = (struct WLAN_ACTION_FRAME *) prSwRfb->pvHeader;

	switch (prRxFrame->ucAction) {
	case ACTION_ADDTS_REQ:
	case ACTION_ADDTS_RSP:
	case ACTION_SCHEDULE:
		log_dbg(INIT, INFO, "qos action frame received, action: %d\n",
			prRxFrame->ucAction);
		break;
	case ACTION_QOS_MAP_CONFIGURE:
		qosHandleQosMapConfigure(prAdapter, prSwRfb);
		log_dbg(INIT, INFO, "qos map configure frame received, action: %d\n",
			prRxFrame->ucAction);
		break;
	default:
		log_dbg(INIT, INFO, "qos action frame: %d, try to send to supplicant\n",
			prRxFrame->ucAction);
		break;
	}
}

int qosHandleQosMapConfigure(struct ADAPTER *prAdapter,
	struct SW_RFB *prSwRfb)
{
	struct _ACTION_QOS_MAP_CONFIGURE_FRAME *prRxFrame = NULL;
	struct STA_RECORD *prStaRec;
	uint16_t u2IELength = 0;

	prRxFrame =
		(struct _ACTION_QOS_MAP_CONFIGURE_FRAME *) prSwRfb->pvHeader;
	if (!prRxFrame)
		return -1;

	prStaRec = cnmGetStaRecByIndex(prAdapter, prSwRfb->ucStaRecIdx);
	if ((!prStaRec) || (!prStaRec->fgIsInUse))
		return -1;

	log_dbg(INIT, INFO,
	"IEEE 802.11: Received Qos Map Configure Frame from " MACSTR "\n",
		MAC2STR(prStaRec->aucMacAddr));

	u2IELength = (prSwRfb->u2PacketLen - prSwRfb->u2HeaderLen) -
		(uint16_t)
		(OFFSET_OF(struct _ACTION_QOS_MAP_CONFIGURE_FRAME, qosMapSet[0])
			- WLAN_MAC_HEADER_LEN);

	if (u2IELength < ELEM_HDR_LEN ||
		u2IELength < ELEM_HDR_LEN + IE_LEN(prRxFrame->qosMapSet)) {
		DBGLOG(INIT, WARN, "QosMapSet IE: insufficient length %d\n",
			u2IELength);
		return -1;
	}

	qosParseQosMapSet(prAdapter, prStaRec, prRxFrame->qosMapSet);

	return 0;
}

void qosParseQosMapSet(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t *qosMapSet)
{
	uint8_t dscpExcNum = 0;
	int i = 0;
	uint8_t *tempq = qosMapSet + 2;
	uint8_t *qosmapping = prStaRec->qosMapSet;
	uint8_t excTable[64];

	if (IE_ID(qosMapSet) != ELEM_ID_QOS_MAP_SET) {
		DBGLOG(INIT, WARN,
			"Wrong QosMapSet IE ID: %d\n", IE_ID(qosMapSet));
		return;
	}
	if ((IE_LEN(qosMapSet) < 16) || (IE_LEN(qosMapSet) > 58)) {
		DBGLOG(INIT, WARN,
			"Error in QosMapSet IE len: %d\n", IE_LEN(qosMapSet));
		return;
	}

	qosMapSetInit(prStaRec);
	kalMemSet(excTable, 0, 64);

	dscpExcNum = (IE_LEN(qosMapSet) - WMM_UP_INDEX_NUM * 2) / 2;
	for (i = 0; i < dscpExcNum; i++) {
		uint8_t dscp = *tempq++;
		uint8_t up = *tempq++;

		if (dscp < 64 && up < WMM_UP_INDEX_NUM) {
			qosmapping[dscp] = up;
			excTable[dscp] = TRUE;
		}
	}

	for (i = 0; i < WMM_UP_INDEX_NUM; i++) {
		uint8_t lDscp = *tempq++;
		uint8_t hDscp = *tempq++;
		uint8_t dscp;

		if (lDscp == 255 && hDscp == 255) {
			log_dbg(INIT, WARN, "UP %d is not used\n", i);
			continue;
		}

		if (hDscp < lDscp) {
			log_dbg(INIT, WARN, "CHECK: UP %d, h %d, l %d\n",
				i, hDscp, lDscp);
			continue;
		}

		for (dscp = lDscp; dscp < 64 && dscp <= hDscp; dscp++) {
			if (!excTable[dscp])
				qosmapping[dscp] = i;
		}
	}

	DBGLOG(INIT, INFO, "QosMapSet DSCP Exception number: %d\n", dscpExcNum);
}

void qosMapSetInit(struct STA_RECORD *prStaRec)
{
	/* DSCP to UP maaping based on RFC8325 in the range 0 to 63 */
	static uint8_t dscp2up[64] = {
		[0] = WMM_UP_BE_INDEX,
		[1] = 0xFF,
		[2] = 0xFF,
		[3] = 0xFF,
		[4] = 0xFF,
		[5] = 0xFF,
		[6] = 0xFF,
		[7] = 0xFF,
		[8] = WMM_UP_BK_INDEX,
		[9] = 0xFF,
		[10] = WMM_UP_BE_INDEX,
		[11] = 0xFF,
		[12] = WMM_UP_BE_INDEX,
		[13] = 0xFF,
		[14] = WMM_UP_BE_INDEX,
		[15] = 0xFF,
		[16] = WMM_UP_BE_INDEX,
		[17] = 0xFF,
		[18] = WMM_UP_EE_INDEX,
		[19] = 0xFF,
		[20] = WMM_UP_EE_INDEX,
		[21] = 0xFF,
		[22] = WMM_UP_EE_INDEX,
		[23] = 0xFF,
		[24] = WMM_UP_CL_INDEX,
		[25] = 0xFF,
		[26] = WMM_UP_CL_INDEX,
		[27] = 0xFF,
		[28] = WMM_UP_CL_INDEX,
		[29] = 0xFF,
		[30] = WMM_UP_CL_INDEX,
		[31] = 0xFF,
		[32] = WMM_UP_CL_INDEX,
		[33] = 0xFF,
		[34] = WMM_UP_CL_INDEX,
		[35] = 0xFF,
		[36] = WMM_UP_CL_INDEX,
		[37] = 0xFF,
		[38] = WMM_UP_CL_INDEX,
		[39] = 0xFF,
		[40] = WMM_UP_VI_INDEX,
		[41] = 0xFF,
		[42] = 0xFF,
		[43] = 0xFF,
		[44] = WMM_UP_VO_INDEX,
		[45] = 0xFF,
		[46] = WMM_UP_VO_INDEX,
		[47] = 0xFF,
		[48] = WMM_UP_VO_INDEX,
		[49] = 0xFF,
		[50] = 0xFF,
		[51] = 0xFF,
		[52] = 0xFF,
		[53] = 0xFF,
		[54] = 0xFF,
		[55] = 0xFF,
		[56] = WMM_UP_NC_INDEX,
		[57] = 0xFF,
		[58] = 0xFF,
		[59] = 0xFF,
		[60] = 0xFF,
		[61] = 0xFF,
		[62] = 0xFF,
		[63] = 0xFF,
	};

	kalMemCopy(prStaRec->qosMapSet, dscp2up, 64);
}

uint8_t getUpFromDscp(struct GLUE_INFO *prGlueInfo,
		uint8_t ucBssIndex, int dscp)
{
	struct BSS_INFO *prBssInfo;
	struct STA_RECORD *prStaRec;

	prBssInfo = GET_BSS_INFO_BY_INDEX(prGlueInfo->prAdapter, ucBssIndex);
	if (prBssInfo)
		prStaRec = prBssInfo->prStaRecOfAP;
	else
		return 0xFF;

	if (prStaRec && dscp >= 0 && dscp < 64)
		return prStaRec->qosMapSet[dscp];

	return 0xFF;
}
#endif
