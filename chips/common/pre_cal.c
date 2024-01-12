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
#if (CFG_SUPPORT_CONNFEM == 1)
#include "connfem_api.h"
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
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
#if CFG_MTK_ANDROID_EMI
u_int8_t *gEmiCalResult;
u_int32_t gEmiCalSize;
u_int32_t gEmiCalOffset;
u_int32_t gEmiGetCalOffset;
bool gEmiCalUseEmiData;
#endif

struct wireless_dev *grWdev;

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */

uint32_t wlanAccessCalibrationEMI(struct ADAPTER *prAdapter,
	struct INIT_EVENT_PHY_ACTION_RSP *pCalEvent,
	uint8_t backupEMI)
{
	uint32_t u4Status = WLAN_STATUS_FAILURE;

#if CFG_MTK_ANDROID_EMI

	do {
		if (backupEMI == TRUE) {
			if (gEmiCalResult != NULL) {
				kalMemFree(gEmiCalResult,
					VIR_MEM_TYPE,
					gEmiCalSize);
				gEmiCalResult = NULL;
			}

			if (pCalEvent->u4EmiLength == 0) {
				DBGLOG(INIT, ERROR, "gEmiCalSize 0\n");
				break;
			}

			gEmiCalOffset = emi_mem_offset_convert(
				pCalEvent->u4EmiAddress);
			gEmiCalSize = pCalEvent->u4EmiLength;

			gEmiCalResult = kalMemAlloc(gEmiCalSize, VIR_MEM_TYPE);
			if (gEmiCalResult == NULL) {
				DBGLOG(INIT, ERROR,
					"gEmiCalResult kalMemAlloc NULL\n");
				break;
			}

			emi_mem_read(prAdapter->chip_info,
				gEmiCalOffset,
				gEmiCalResult,
				gEmiCalSize);

			u4Status = WLAN_STATUS_SUCCESS;
			break;
		}

		/* else, put calibration data to EMI */

		if (gEmiCalResult == NULL) {
			DBGLOG(INIT, ERROR, "gEmiCalResult NULL\n");
			break;
		}

		if (gEmiCalUseEmiData == TRUE) {
			DBGLOG(INIT, INFO, "No Write back to EMI\n");
			break;
		}
#if 0  /* Disable host write EMI */
		emi_mem_write(prAdapter->chip_info,
			gEmiCalOffset,
			gEmiCalResult,
			gEmiCalSize);
#endif
		u4Status = WLAN_STATUS_SUCCESS;
	} while (FALSE);
#endif /* CFG_MTK_ANDROID_EMI */
	return u4Status;
}

void wlanGetEpaElnaFromNvram(
	uint8_t **pu1DataPointer,
	uint32_t *pu4DataLen)
{
#define MAX_NVRAM_READY_COUNT 10
#define MAX_NVRAM_FEM_MAX 512

	/* ePA /eLNA */
	uint8_t u1TypeID;
	uint8_t u1LenLSB;
	uint8_t u1LenMSB;
	uint8_t u1Total_Size_LSB, u1Total_Size_MSB;
	uint32_t u4Tag7_9_data_len = 0, u46GCOMM_len = 0;
	uint16_t u2NVRAM_Toal_Size = 0;
	uint32_t u4NvramStartOffset = 0, u4NvramOffset = 0;
	uint8_t *pu1Addr;
	struct WIFI_NVRAM_TAG_FORMAT *prTagDataCurr;
	int retryCount = 0;
	enum WIFI_NVRAM_TAG {
		NVRAM_TAG_NVRAM_CTRL = 0,
		NVRAM_TAG_2G4_TX_POWER = 1,
		NVRAM_TAG_5G_TX_POWER = 2,
		NVRAM_TAG_2G4_WF0_PATH = 3,
		NVRAM_TAG_2G4_WF1_PATH = 4,
		NVRAM_TAG_5G_WF0_PATH = 5,
		NVRAM_TAG_5G_WF1_PATH = 6,
		NVRAM_TAG_2G4_COMMON = 7,
		NVRAM_TAG_5G_COMMON = 8,
		NVRAM_TAG_SYSTEM = 9,
		NVRAM_TAG_CO_ANT = 10,
		NVRAM_TAG_11N_DELAY = 11,
		NVRAM_TAG_2G4_WF0_AUX_PATH = 12,
		NVRAM_TAG_5G_WF0_AUX_PATH = 13,
		NVRAM_TAG_2G4_WF1_AUX_PATH = 14,
		NVRAM_TAG_5G_WF1_AUX_PATH = 15,
		NVRAM_TAG_6G_TX_POWER = 16,
		NVRAM_TAG_6G_WF0_PATH = 17,
		NVRAM_TAG_6G_WF1_PATH = 18,
		NVRAM_TAG_6G_WF0_AUX_PATH = 19,
		NVRAM_TAG_6G_WF1_AUX_PATH = 20,
		NVRAM_TAG_6G_COMMON = 21,
		NVRAM_TAG_NUMBER
	};

	while (g_NvramFsm != NVRAM_STATE_READY) {
		kalMsleep(100);
		retryCount++;

		if (retryCount > MAX_NVRAM_READY_COUNT) {
			DBGLOG(INIT, WARN, "g_NvramFsm != NVRAM_STATE_READY\n");
			return;
		}
	}

	/* Get NVRAM Start Addr */
	pu1Addr = (uint8_t *)(struct WIFI_CFG_PARAM_STRUCT *)&g_aucNvram[0];

	/* Shift to NVRAM Tag */
	u4NvramOffset = OFFSET_OF(struct WIFI_CFG_PARAM_STRUCT, ucTypeID0);
	prTagDataCurr =
		(struct WIFI_NVRAM_TAG_FORMAT *)(pu1Addr
		+ u4NvramOffset);

    /* Shift to Tag ID 0 to get NVRAM total length */
	u1TypeID = prTagDataCurr->u1NvramTypeID;
	if (u1TypeID == NVRAM_TAG_NVRAM_CTRL) {
		u1Total_Size_LSB = g_aucNvram[u4NvramOffset + 4];
		u1Total_Size_MSB = g_aucNvram[u4NvramOffset + 5];
		u2NVRAM_Toal_Size = ((uint16_t)u1Total_Size_MSB << 8)
			 | ((uint16_t)u1Total_Size_LSB);
		u2NVRAM_Toal_Size += 256;  /* NVRAM BASIC data len */
	}

	/* Shift to NVRAM Tag 7 - 9, r2G4Cmm, r5GCmm , rSys*/
	while (u4NvramOffset < u2NVRAM_Toal_Size) {
		u1TypeID = prTagDataCurr->u1NvramTypeID;
		u1LenLSB = prTagDataCurr->u1NvramTypeLenLsb;
		u1LenMSB = prTagDataCurr->u1NvramTypeLenMsb;
		DBGLOG(INIT, TRACE, "CurOfs[%d]:Next(%d)Len:%d\n",
			u4NvramOffset, u1TypeID,
			(u1LenMSB << 8) | (u1LenLSB));

		/*sanity check*/
		if ((u1TypeID == 0) &&
			(u1LenLSB == 0) && (u1LenMSB == 0)) {
			DBGLOG(INIT, TRACE, "TLV is Null\n");
			break;
		}

		/*check Type ID is exist on NVRAM*/
		if (u1TypeID == NVRAM_TAG_2G4_COMMON) {
			u4NvramStartOffset = u4NvramOffset;
			DBGLOG(INIT, TRACE,
			"NVRAM tag(%d) exist! ofst %x\n",
			u1TypeID, u4NvramStartOffset);
		}

		if (u1TypeID == NVRAM_TAG_CO_ANT) {
			*pu1DataPointer = pu1Addr + u4NvramStartOffset;
			*pu4DataLen = u4NvramOffset - u4NvramStartOffset;
			DBGLOG(INIT, TRACE,
			"NVRAM datapointer %x tag7 ofst %x tag7-9 Len %x\n",
			*pu1DataPointer, u4NvramStartOffset, *pu4DataLen);
			kalMemCopy(&g_aucNvram_OnlyPreCal[0],
				&g_aucNvram[u4NvramStartOffset], *pu4DataLen);
			u4Tag7_9_data_len = *pu4DataLen;
		}

		if (u1TypeID == NVRAM_TAG_6G_COMMON) {
			/* Only to get 6G COMMON TLV */
			*pu1DataPointer = pu1Addr + u4NvramOffset;
			u46GCOMM_len = sizeof(struct WIFI_NVRAM_TAG_FORMAT);
			u46GCOMM_len += (u1LenMSB << 8) | (u1LenLSB);
			kalMemCopy(&g_aucNvram_OnlyPreCal[u4Tag7_9_data_len],
			 &g_aucNvram[u4NvramOffset], u46GCOMM_len);
			*pu4DataLen += u46GCOMM_len;
			DBGLOG(INIT, TRACE,
				"NVRAM tag(%d) u46GCOMM_len %d, u4NvramOffset %d, pu4DataLen %d\n",
				u1TypeID, u46GCOMM_len, pu4DataLen);
		}
		u4NvramOffset += sizeof(struct WIFI_NVRAM_TAG_FORMAT);
		u4NvramOffset += (u1LenMSB << 8) | (u1LenLSB);

		/*get the nex TLV format*/
		prTagDataCurr = (struct WIFI_NVRAM_TAG_FORMAT *)
			(pu1Addr + u4NvramOffset);
	}

	/* Get NVRAM Start Addr */
	pu1Addr = (uint8_t *)
		(struct WIFI_CFG_PARAM_STRUCT *)&g_aucNvram_OnlyPreCal[0];
	*pu1DataPointer = pu1Addr;

	if (((*pu4DataLen) & 0x3) != 0) {
		DBGLOG(INIT, WARN,
			"NVRAM datapointer Len adjust to 4 byte alignment(%x->%x)\n",
			*pu4DataLen,
			*pu4DataLen + 4 - ((*pu4DataLen) & 0x3));
		*pu4DataLen += 4 - ((*pu4DataLen) & 0x3);
	}

	if (*pu4DataLen > MAX_NVRAM_FEM_MAX) {
		*pu4DataLen = MAX_NVRAM_FEM_MAX;
		DBGLOG(INIT, WARN,
			"NVRAM datapointer Len(test) adjust (%x) for command max\n",
			*pu4DataLen);
	}

	DBGLOG_MEM8(INIT, TRACE, *pu1DataPointer, *pu4DataLen);
}

uint32_t wlanRcvPhyActionRsp(struct ADAPTER *prAdapter,
	struct HAL_PHY_ACTION_TLV_HEADER *prPhyTlvHeader)
{
	struct HAL_PHY_ACTION_TLV *prPhyTlv;
	struct INIT_EVENT_PHY_ACTION_RSP *prPhyEvent;
	uint32_t u4Status = WLAN_STATUS_FAILURE;

	if (prPhyTlvHeader->u4MagicNum != HAL_PHY_ACTION_MAGIC_NUM) {
		DBGLOG(INIT, ERROR, "HAL_PHY_ACTION_MAGIC_NUM failed\n");
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	prPhyTlv =
		(struct HAL_PHY_ACTION_TLV *)prPhyTlvHeader->aucBuffer;

	prPhyEvent = (struct INIT_EVENT_PHY_ACTION_RSP *)
		prPhyTlv->aucBuffer;

	if (prPhyTlv->u2Tag == HAL_PHY_ACTION_TAG_CAL) {
		DBGLOG(INIT, INFO,
			"HAL_PHY_ACTION_TAG_CAL ucEvent[0x%x]status[0x%x]emiAddr[0x%x]emiLen[0x%x]\n",
			prPhyEvent->ucEvent,
			prPhyEvent->ucStatus,
			prPhyEvent->u4EmiAddress,
			prPhyEvent->u4EmiLength);

		if ((prPhyEvent->ucEvent ==
			HAL_PHY_ACTION_CAL_FORCE_CAL_RSP &&
			prPhyEvent->ucStatus ==
			HAL_PHY_ACTION_STATUS_SUCCESS) ||
			(prPhyEvent->ucEvent ==
			HAL_PHY_ACTION_CAL_USE_BACKUP_RSP &&
			prPhyEvent->ucStatus ==
			HAL_PHY_ACTION_STATUS_RECAL)) {

#if CFG_MTK_ANDROID_EMI
			/* Get Emi address and send to Conninfra */
			gEmiGetCalOffset = prPhyEvent->u4EmiAddress &
				WIFI_EMI_ADDR_MASK;
#endif

			/* read from EMI, backup in driver */
			wlanAccessCalibrationEMI(prAdapter,
				prPhyEvent,
				TRUE);
		}

		u4Status = WLAN_STATUS_SUCCESS;
	} else if (prPhyTlv->u2Tag == HAL_PHY_ACTION_TAG_NVRAM) {
		DBGLOG(INIT, INFO,
			"HAL_PHY_ACTION_TAG_NVRAM status[0x%x]\n",
			prPhyEvent->ucStatus);

		u4Status = WLAN_STATUS_SUCCESS;
	} else if (prPhyTlv->u2Tag == HAL_PHY_ACTION_TAG_COM_FEM) {
		DBGLOG(INIT, INFO,
			"HAL_PHY_ACTION_TAG_COM_FEM status[0x%x]\n",
			prPhyEvent->ucStatus);

		u4Status = WLAN_STATUS_SUCCESS;
	}

exit:
	return u4Status;
}

#if (CFG_SUPPORT_CONNFEM == 1)
struct COM_FEM_TAG_FORMAT {
	uint32_t tag_fem_info_id;
	struct connfem_epaelna_pin_info tag_pin_info;
#if (CFG_SUPPORT_CONNAC3X == 1)
	struct connfem_epaelna_flags_common tag_flags_common;
#endif
};

struct LAA_TAG_FORMAT {
	struct connfem_epaelna_laa_pin_info tag_laa_pin_info;
};
#endif

uint32_t wlanSendPhyAction(struct ADAPTER *prAdapter,
	uint16_t u2Tag,
	uint8_t ucCalCmd)
{
	struct HAL_PHY_ACTION_TLV_HEADER *prCmd = NULL;
	struct HAL_PHY_ACTION_TLV_HEADER *prEvent = NULL;
	struct HAL_PHY_ACTION_TLV *prPhyTlv;
	struct INIT_CMD_PHY_ACTION_CAL *prPhyCal;
#if (CFG_SUPPORT_CONNFEM == 1)
	struct COM_FEM_TAG_FORMAT *prTagDataComFEM;
	struct LAA_TAG_FORMAT *prTagDataLAA;
	struct connfem_epaelna_fem_info fem_info;
	struct connfem_epaelna_pin_info pin_info;
	struct connfem_epaelna_laa_pin_info laa_pin_info;
#if (CFG_SUPPORT_CONNAC3X == 1)
	struct connfem_epaelna_flags_common flags_common;
#endif
#endif
	uint8_t *u1EpaELnaDataPointer = NULL;
	uint32_t u4EpaELnaDataSize = 0, u4CmdSize = 0, u4EvtSize = 0;
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, INFO, "SendPhyAction begin\n");

	ASSERT(prAdapter);

	wlanGetEpaElnaFromNvram(&u1EpaELnaDataPointer,
		&u4EpaELnaDataSize);

	if (u1EpaELnaDataPointer == NULL) {
		DBGLOG(INIT, ERROR, "Get u1EpaELnaDataPointer failed\n");
		return WLAN_STATUS_FAILURE;
	}

#if (CFG_SUPPORT_CONNFEM == 1)
	/* Get data from connfem_api */
	connfem_epaelna_get_fem_info(&fem_info);
	connfem_epaelna_get_pin_info(&pin_info);
	connfem_epaelna_laa_get_pin_info(&laa_pin_info);
#if (CFG_SUPPORT_CONNAC3X == 1)
	connfem_epaelna_get_flags(CONNFEM_SUBSYS_NONE, &flags_common);
#endif
#endif

	/* 1. Allocate CMD Info Packet and its Buffer. */
	if (u2Tag == HAL_PHY_ACTION_TAG_NVRAM) {
		u4CmdSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize;
#if (CFG_SUPPORT_CONNFEM == 1)
	} else if (u2Tag == HAL_PHY_ACTION_TAG_COM_FEM) {
		u4CmdSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct LAA_TAG_FORMAT);
	} else if (u2Tag == HAL_PHY_ACTION_TAG_LAA) {
		u4CmdSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct LAA_TAG_FORMAT);
#endif
	} else {
		u4CmdSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL);
#if (CFG_SUPPORT_CONNFEM == 1)
		u4CmdSize += sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT);
#endif
	}

	if ((ucCalCmd == HAL_PHY_ACTION_CAL_FORCE_CAL_REQ) ||
		(ucCalCmd == HAL_PHY_ACTION_CAL_USE_BACKUP_REQ)) {
		u4CmdSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize;
#if (CFG_SUPPORT_CONNFEM == 1)
		u4CmdSize += sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct LAA_TAG_FORMAT);
#endif
	}

	prCmd = kalMemAlloc(u4CmdSize, VIR_MEM_TYPE);
	if (!prCmd) {
		DBGLOG(INIT, ERROR, "Alloc cmd packet failed.\n");
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	u4EvtSize = sizeof(struct HAL_PHY_ACTION_TLV_HEADER) +
		sizeof(struct HAL_PHY_ACTION_TLV) +
		sizeof(struct INIT_EVENT_PHY_ACTION_RSP);
	prEvent = kalMemAlloc(u4EvtSize, VIR_MEM_TYPE);
	if (!prEvent) {
		DBGLOG(INIT, ERROR, "Alloc event packet failed.\n");
		u4Status = WLAN_STATUS_FAILURE;
		goto exit;
	}

	/* Process TLV Header Part1 */
	prCmd->u4MagicNum = HAL_PHY_ACTION_MAGIC_NUM;
	prCmd->ucVersion = HAL_PHY_ACTION_VERSION;

	if (u2Tag == HAL_PHY_ACTION_TAG_NVRAM) {
		/* Process TLV Header Part2 */
		/* Process HAL_PHY_ACTION_TAG_NVRAM only */
		prCmd->ucTagNums = 1;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize;

		/* Process TLV Content */
		/* TAG HAL_PHY_ACTION_TAG_NVRAM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)prCmd->aucBuffer;
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_NVRAM;
		prPhyTlv->u2BufLength = u4EpaELnaDataSize;
		kalMemCopy(prPhyTlv->aucBuffer,
			u1EpaELnaDataPointer, u4EpaELnaDataSize);
#if (CFG_SUPPORT_CONNFEM == 1)
	} else if (u2Tag == HAL_PHY_ACTION_TAG_COM_FEM ||
		u2Tag == HAL_PHY_ACTION_TAG_LAA) {
		/* Process TLV Header Part2 */
		/* Add HAL_PHY_ACTION_TAG_COM_FEM and HAL_PHY_ACTION_TAG_LAA */
		prCmd->ucTagNums = 3;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct LAA_TAG_FORMAT);

		/* Process TLV Content */
		/* TAG HAL_PHY_ACTION_TAG_NVRAM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)prCmd->aucBuffer;
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_NVRAM;
		prPhyTlv->u2BufLength = u4EpaELnaDataSize;
		kalMemCopy(prPhyTlv->aucBuffer,
			u1EpaELnaDataPointer, u4EpaELnaDataSize);

		/* TAG HAL_PHY_ACTION_TAG_COM_FEM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize);
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_COM_FEM;
		prPhyTlv->u2BufLength = sizeof(struct COM_FEM_TAG_FORMAT);
		prTagDataComFEM =
			(struct COM_FEM_TAG_FORMAT *)prPhyTlv->aucBuffer;
		prTagDataComFEM->tag_fem_info_id = fem_info.id;
		kalMemCopy(&prTagDataComFEM->tag_pin_info,
			&pin_info, sizeof(struct connfem_epaelna_pin_info));
#if (CFG_SUPPORT_CONNAC3X == 1)
		kalMemCopy(&prTagDataComFEM->tag_flags_common,
			&flags_common,
			sizeof(struct connfem_epaelna_flags_common));
#endif

		/* TAG HAL_PHY_ACTION_TAG_LAA */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT));
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_LAA;
		prPhyTlv->u2BufLength = sizeof(struct LAA_TAG_FORMAT);
		prTagDataLAA =
			(struct LAA_TAG_FORMAT *)prPhyTlv->aucBuffer;
		kalMemCopy(&prTagDataLAA->tag_laa_pin_info,
			&laa_pin_info,
			sizeof(struct connfem_epaelna_laa_pin_info));
#endif
	} else if (
		(ucCalCmd == HAL_PHY_ACTION_CAL_FORCE_CAL_REQ) ||
		(ucCalCmd == HAL_PHY_ACTION_CAL_USE_BACKUP_REQ)) {
		/* Process TLV Header Part2 */
		/* Add HAL_PHY_ACTION_TAG_NVRAM */
		prCmd->ucTagNums = 2;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize;
#if (CFG_SUPPORT_CONNFEM == 1)
		/* Add HAL_PHY_ACTION_TAG_NVRAM, HAL_PHY_ACTION_TAG_COM_FEM */
		/* and HAL_PHY_ACTION_TAG_LAA */
		prCmd->ucTagNums = 4;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct LAA_TAG_FORMAT);
#endif

		/* Process TLV Content */
		/* TAG HAL_PHY_ACTION_TAG_CAL */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)prCmd->aucBuffer;
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_CAL;
		prPhyTlv->u2BufLength = sizeof(struct INIT_CMD_PHY_ACTION_CAL);
		prPhyCal =
			(struct INIT_CMD_PHY_ACTION_CAL *)prPhyTlv->aucBuffer;
		prPhyCal->ucCmd = ucCalCmd;
#if CFG_MTK_ANDROID_WMT
		prPhyCal->ucCalSaveResult = 1;
#else
		prPhyCal->ucCalSaveResult = 0;
#endif

		/* TAG HAL_PHY_ACTION_TAG_NVRAM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL));
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_NVRAM;
		prPhyTlv->u2BufLength = u4EpaELnaDataSize;
		kalMemCopy(prPhyTlv->aucBuffer,
		u1EpaELnaDataPointer, u4EpaELnaDataSize);

#if (CFG_SUPPORT_CONNFEM == 1)
		/* TAG HAL_PHY_ACTION_TAG_COM_FEM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize);
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_COM_FEM;
		prPhyTlv->u2BufLength = sizeof(struct COM_FEM_TAG_FORMAT);
		prTagDataComFEM =
			(struct COM_FEM_TAG_FORMAT *)prPhyTlv->aucBuffer;
		prTagDataComFEM->tag_fem_info_id = fem_info.id;
		kalMemCopy(&prTagDataComFEM->tag_pin_info,
			&pin_info, sizeof(struct connfem_epaelna_pin_info));
#if (CFG_SUPPORT_CONNAC3X == 1)
		kalMemCopy(&prTagDataComFEM->tag_flags_common,
			&flags_common,
			sizeof(struct connfem_epaelna_flags_common));
#endif

		/* TAG HAL_PHY_ACTION_TAG_LAA */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			u4EpaELnaDataSize +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT));
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_LAA;
		prPhyTlv->u2BufLength = sizeof(struct LAA_TAG_FORMAT);
		prTagDataLAA =
			(struct LAA_TAG_FORMAT *)prPhyTlv->aucBuffer;
		kalMemCopy(&prTagDataLAA->tag_laa_pin_info,
			&laa_pin_info,
			sizeof(struct connfem_epaelna_laa_pin_info));
#endif
	} else {
		/* Process TLV Header Part2 */
		prCmd->ucTagNums = 1;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL);
#if (CFG_SUPPORT_CONNFEM == 1)
		prCmd->ucTagNums = 2;
		prCmd->u2BufLength =
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL) +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct COM_FEM_TAG_FORMAT);
#endif

		/* Process TLV Content */
		/* TAG HAL_PHY_ACTION_TAG_CAL */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)prCmd->aucBuffer;
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_CAL;
		prPhyTlv->u2BufLength = sizeof(struct INIT_CMD_PHY_ACTION_CAL);
		prPhyCal =
			(struct INIT_CMD_PHY_ACTION_CAL *)prPhyTlv->aucBuffer;
		prPhyCal->ucCmd = ucCalCmd;
#if CFG_MTK_ANDROID_WMT
		prPhyCal->ucCalSaveResult = 1;
#else
		prPhyCal->ucCalSaveResult = 0;
#endif

#if (CFG_SUPPORT_CONNFEM == 1)
		/* TAG HAL_PHY_ACTION_TAG_COM_FEM */
		prPhyTlv =
			(struct HAL_PHY_ACTION_TLV *)
			(prCmd->aucBuffer +
			sizeof(struct HAL_PHY_ACTION_TLV) +
			sizeof(struct INIT_CMD_PHY_ACTION_CAL));
		prPhyTlv->u2Tag = HAL_PHY_ACTION_TAG_COM_FEM;
		prPhyTlv->u2BufLength = sizeof(struct COM_FEM_TAG_FORMAT);
		prTagDataComFEM =
			(struct COM_FEM_TAG_FORMAT *)prPhyTlv->aucBuffer;
		prTagDataComFEM->tag_fem_info_id = fem_info.id;
		kalMemCopy(&prTagDataComFEM->tag_pin_info,
			&pin_info, sizeof(struct connfem_epaelna_pin_info));
#if (CFG_SUPPORT_CONNAC3X == 1)
		kalMemCopy(&prTagDataComFEM->tag_flags_common,
			&flags_common,
			sizeof(struct connfem_epaelna_flags_common));
#endif
#endif
	}

	wlanSendInitSetQueryCmdImpl(prAdapter,
		INIT_CMD_ID_PHY_ACTION, prCmd, u4CmdSize,
		TRUE, FALSE,
		INIT_EVENT_ID_PHY_ACTION, prEvent, u4EvtSize,
		CFG_PRE_CAL_SLEEP_WAITING_INTERVAL,
		CFG_PRE_CAL_RX_RESPONSE_TIMEOUT);
	if (u4Status != WLAN_STATUS_SUCCESS)
		goto exit;

	u4Status = wlanRcvPhyActionRsp(prAdapter, prEvent);

exit:
	if (prCmd)
		kalMemFree(prCmd, VIR_MEM_TYPE, u4CmdSize);

	if (prEvent)
		kalMemFree(prEvent, VIR_MEM_TYPE, u4EvtSize);

	return u4Status;
}

uint32_t wlanPhyAction(IN struct ADAPTER *prAdapter)
{
	uint32_t u4Status = WLAN_STATUS_SUCCESS;

	/* Setup calibration data from backup file */
#if CFG_MTK_ANDROID_WMT
	if (wlanAccessCalibrationEMI(prAdapter, NULL, FALSE) ==
		WLAN_STATUS_SUCCESS)
		u4Status = wlanSendPhyAction(prAdapter,
			HAL_PHY_ACTION_TAG_CAL,
			HAL_PHY_ACTION_CAL_USE_BACKUP_REQ);
	else
#endif
		u4Status = wlanSendPhyAction(prAdapter,
			HAL_PHY_ACTION_TAG_CAL,
			HAL_PHY_ACTION_CAL_FORCE_CAL_REQ);

	return u4Status;
}

#if (CFG_SUPPORT_CONNINFRA == 1)
int wlanGetCalResultCb(uint32_t *pEmiCalOffset, uint32_t *pEmiCalSize)
{
	uint32_t u4Status = WLAN_STATUS_FAILURE;

	/* Shift 4 for bypass Cal result CRC */
	*pEmiCalOffset = gEmiGetCalOffset + 0x4;

	/* 2k size for RFCR backup */
	*pEmiCalSize = 2048;

	DBGLOG(INIT, INFO,
				"EMI_GET_CAL emiAddr[0x%x]emiLen[%d]\n",
				*pEmiCalOffset,
				*pEmiCalSize);

	u4Status = WLAN_STATUS_SUCCESS;
	return u4Status;
}

int wlanPreCalPwrOn(void)
{
#define MAX_PRE_ON_COUNT 5

	int retryCount = 0;
	void *pvData = NULL;
	enum ENUM_POWER_ON_INIT_FAIL_REASON {
		NET_CREATE_FAIL,
		BUS_SET_IRQ_FAIL,
		ALLOC_ADAPTER_MEM_FAIL,
		DRIVER_OWN_FAIL,
		INIT_ADAPTER_FAIL,
		INIT_HIFINFO_FAIL,
		ROM_PATCH_DOWNLOAD_FAIL,
		POWER_ON_INIT_DONE
	} eFailReason;
	uint32_t i = 0, j = 0;
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_hif_driver_data *prDriverData =
		get_platform_driver_data();
	struct mt66xx_chip_info *prChipInfo;

	while (update_wr_mtx_down_up_status(0, 0)) {
		if (get_wifi_process_status())
			return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
		kalMsleep(50);
	}

	if (get_wifi_powered_status())
		return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;

	prChipInfo = prDriverData->chip_info;
	pvData = (void *)prChipInfo->pdev;
	update_pre_cal_status(1);

	while (g_u4WlanInitFlag == 0) {
		DBGLOG(INIT, WARN,
			"g_u4WlanInitFlag(%d) retryCount(%d)",
			g_u4WlanInitFlag,
			retryCount);

		kalMsleep(100);
		retryCount++;

		if (retryCount > MAX_PRE_ON_COUNT) {
			update_pre_cal_status(0);
			update_wr_mtx_down_up_status(1, 0);
			return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
		}
	}

	/* wf driver power on */
	if (asicConnac2xPwrOnWmMcu(prChipInfo) != 0) {
		update_pre_cal_status(0);
		update_wr_mtx_down_up_status(1, 0);
		return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
	}

	/* Download patch and send PHY action */
	do {
		retryCount = 0;
		while (g_prPlatDev == NULL) {
			DBGLOG(INIT, WARN,
				"g_prPlatDev(0x%x) retryCount(%d)",
				g_prPlatDev,
				retryCount);

			kalMsleep(100);
			retryCount++;

			if (retryCount > MAX_PRE_ON_COUNT) {
				update_pre_cal_status(0);
				update_wr_mtx_down_up_status(1, 0);
				return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
			}
		}

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Init();
#endif

		/* Create network device, Adapter, KalInfo,
		*		prDevHandler(netdev)
		*/
		grWdev = wlanNetCreate(pvData, (void *)prDriverData);

		if (grWdev == NULL) {
			DBGLOG(INIT, ERROR, "wlanNetCreate Error\n");
			eFailReason = NET_CREATE_FAIL;
			break;
		}

		/* Set the ioaddr to HIF Info */
		WIPHY_PRIV(grWdev->wiphy, prGlueInfo);

		/* Setup IRQ */
		if (glBusSetIrq(grWdev->netdev, NULL, prGlueInfo)
			!= WLAN_STATUS_SUCCESS) {
			DBGLOG(INIT, ERROR, "glBusSetIrq error\n");
			eFailReason = BUS_SET_IRQ_FAIL;
			break;
		}

		prGlueInfo->i4DevIdx = 0;
		prAdapter = prGlueInfo->prAdapter;

		if (prChipInfo->asicCapInit != NULL)
			prChipInfo->asicCapInit(prAdapter);

		prAdapter->fgIsFwOwn = TRUE;
		if (wlanAcquirePowerControl(prAdapter) != WLAN_STATUS_SUCCESS) {
			eFailReason = DRIVER_OWN_FAIL;
			break;
		}

		prAdapter->u4OwnFailedCount = 0;
		prAdapter->u4OwnFailedLogCount = 0;

		/* Additional with chip reset optimize */
		prAdapter->ucCmdSeqNum = 0;

		QUEUE_INITIALIZE(&(prAdapter->rPendingCmdQueue));
#if CFG_SUPPORT_MULTITHREAD
		QUEUE_INITIALIZE(&prAdapter->rTxCmdQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxCmdDoneQueue);
#if CFG_FIX_2_TX_PORT
		QUEUE_INITIALIZE(&prAdapter->rTxP0Queue);
		QUEUE_INITIALIZE(&prAdapter->rTxP1Queue);
#else
		for (i = 0; i < MAX_BSSID_NUM; i++)
			for (j = 0; j < TC_NUM; j++)
				QUEUE_INITIALIZE(&prAdapter->rTxPQueue[i][j]);
#endif
		QUEUE_INITIALIZE(&prAdapter->rRxQueue);
		QUEUE_INITIALIZE(&prAdapter->rTxDataDoneQueue);
#endif

		/* reset fgIsBusAccessFailed */
		fgIsBusAccessFailed = FALSE;

		/* Allocate mandatory resource for TX/RX */
		if (nicAllocateAdapterMemory(prAdapter) !=
			WLAN_STATUS_SUCCESS) {

			DBGLOG(INIT, ERROR,
				"nicAllocateAdapterMemory Error!\n");
			eFailReason = ALLOC_ADAPTER_MEM_FAIL;
			break;
		}

		/* should we need this?  to be conti... */
		prAdapter->u4OsPacketFilter = PARAM_PACKET_FILTER_SUPPORTED;

		/* Initialize the Adapter:
		*		verify chipset ID, HIF init...
		*		the code snippet just do the copy thing
		*/
		if (nicInitializeAdapter(prAdapter) != WLAN_STATUS_SUCCESS) {

			DBGLOG(INIT, ERROR,
				"nicInitializeAdapter failed!\n");
			eFailReason = INIT_ADAPTER_FAIL;
			break;
		}

		nicInitSystemService(prAdapter, FALSE);

		/* Initialize Tx */
		nicTxInitialize(prAdapter);

		/* Initialize Rx */
		nicRxInitialize(prAdapter);

		/* HIF SW info initialize */
		if (!halHifSwInfoInit(prAdapter)) {

			DBGLOG(INIT, ERROR,
				"halHifSwInfoInit failed!\n");
			eFailReason = INIT_HIFINFO_FAIL;
			break;
		}

		/* Enable HIF  cut-through to N9 mode */
		HAL_ENABLE_FWDL(prAdapter, TRUE);

		/* Disable interrupt, download is done by polling mode only */
		nicDisableInterrupt(prAdapter);

		/* Initialize Tx Resource to fw download state */
		nicTxInitResetResource(prAdapter);

		if (prChipInfo->pwrondownload) {
			uint32_t ret;

			ret = prChipInfo->pwrondownload(prAdapter,
				ENUM_WLAN_POWER_ON_DOWNLOAD_ROM_PATCH);
			if (ret != WLAN_STATUS_SUCCESS &&
					ret != WLAN_STATUS_NOT_SUPPORTED) {
				DBGLOG(INIT, ERROR,
					"pwrondownload failed!\n");
				eFailReason = ROM_PATCH_DOWNLOAD_FAIL;
				break;
			}
		}

		kalSyncTimeToFW(prAdapter, TRUE);

#if (CFG_SUPPORT_CONNFEM == 1)
		wlanSendPhyAction(prAdapter,
			HAL_PHY_ACTION_TAG_COM_FEM,
			0);
#else
		wlanSendPhyAction(prAdapter,
			HAL_PHY_ACTION_TAG_NVRAM,
			0);
#endif

		eFailReason = POWER_ON_INIT_DONE;
	} while (FALSE);

	DBGLOG(INIT, INFO,
		"wlanPreCalPwrOn end(%d)\n",
		eFailReason);

	if (eFailReason != POWER_ON_INIT_DONE) {
		u_int8_t fgResult;

		HAL_LP_OWN_SET(prAdapter, &fgResult);
		update_pre_cal_status(0);
		update_wr_mtx_down_up_status(1, 0);
	}

	switch (eFailReason) {
	case NET_CREATE_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif
		break;

	case BUS_SET_IRQ_FAIL:
#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif
		wlanWakeLockUninit(prGlueInfo);
		wlanNetDestroy(grWdev);
		break;

	case ALLOC_ADAPTER_MEM_FAIL:
	case DRIVER_OWN_FAIL:
	case INIT_ADAPTER_FAIL:
		glBusFreeIrq(grWdev->netdev,
			*((struct GLUE_INFO **) netdev_priv(grWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);

		if (eFailReason == INIT_ADAPTER_FAIL)
			nicReleaseAdapterMemory(prAdapter);

		wlanNetDestroy(grWdev);
		break;

	case INIT_HIFINFO_FAIL:
		nicRxUninitialize(prAdapter);
		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		glBusFreeIrq(grWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(grWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		nicReleaseAdapterMemory(prAdapter);
		wlanNetDestroy(grWdev);
		break;

	case ROM_PATCH_DOWNLOAD_FAIL:
		HAL_ENABLE_FWDL(prAdapter, FALSE);
		halHifSwInfoUnInit(prGlueInfo);
		nicRxUninitialize(prAdapter);
		nicTxRelease(prAdapter, FALSE);

		/* System Service Uninitialization */
		nicUninitSystemService(prAdapter);

		glBusFreeIrq(grWdev->netdev,
			*((struct GLUE_INFO **)netdev_priv(grWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
		wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

		wlanWakeLockUninit(prGlueInfo);
		nicReleaseAdapterMemory(prAdapter);
		wlanNetDestroy(grWdev);
		break;

	case POWER_ON_INIT_DONE:
		/* pre-cal release resouce */
		break;
	}

	if (eFailReason != POWER_ON_INIT_DONE) {
		if (prChipInfo->dumpBusHangCr)
			prChipInfo->dumpBusHangCr(NULL);
		return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
	}

	return CONNINFRA_CB_RET_CAL_PASS_POWER_OFF;
}

int wlanPreCal(void)
{
	struct GLUE_INFO *prGlueInfo = NULL;
	struct ADAPTER *prAdapter = NULL;
	struct mt66xx_chip_info *prChipInfo;
	u_int8_t fgResult;

	if (get_pre_cal_status() == 0)
		return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;

	if (g_u4WlanInitFlag == 0) {
		DBGLOG(INIT, WARN,
			"g_u4WlanInitFlag(%d)",
			g_u4WlanInitFlag);

		update_pre_cal_status(0);
		update_wr_mtx_down_up_status(1, 0);
		return CONNINFRA_CB_RET_CAL_FAIL_POWER_OFF;
	}

	DBGLOG(INIT, INFO, "PreCal begin\n");

	/* Set the ioaddr to HIF Info */
	WIPHY_PRIV(grWdev->wiphy, prGlueInfo);
	prAdapter = prGlueInfo->prAdapter;
	glGetChipInfo((void **)&prChipInfo);

	/* Disable interrupt, download is done by polling mode only */
	nicDisableInterrupt(prAdapter);

	wlanSendPhyAction(prAdapter,
		HAL_PHY_ACTION_TAG_CAL,
		HAL_PHY_ACTION_CAL_FORCE_CAL_REQ);

	HAL_LP_OWN_SET(prAdapter, &fgResult);

	HAL_ENABLE_FWDL(prAdapter, FALSE);
	halHifSwInfoUnInit(prGlueInfo);
	nicRxUninitialize(prAdapter);
	nicTxRelease(prAdapter, FALSE);

	/* System Service Uninitialization */
	nicUninitSystemService(prAdapter);

	glBusFreeIrq(grWdev->netdev,
		*((struct GLUE_INFO **)netdev_priv(grWdev->netdev)));

#if (CFG_SUPPORT_TRACE_TC4 == 1)
	wlanDebugTC4Uninit();  /* Uninit for TC4 debug */
#endif

	wlanWakeLockUninit(prGlueInfo);
	nicReleaseAdapterMemory(prAdapter);
	wlanNetDestroy(grWdev);

	if (prChipInfo->wmmcupwroff)
		prChipInfo->wmmcupwroff();

	DBGLOG(INIT, INFO, "PreCal end\n");

	update_pre_cal_status(0);
	update_wr_mtx_down_up_status(1, 0);

	return CONNINFRA_CB_RET_CAL_PASS_POWER_OFF;
}
#endif

uint8_t *wlanGetCalResult(uint32_t *prCalSize)
{
#if CFG_MTK_ANDROID_EMI
	*prCalSize = gEmiCalSize;

	return gEmiCalResult;
#else
	*prCalSize = 0;

	return NULL;
#endif
}

void wlanCalDebugCmd(uint32_t cmd, uint32_t para)
{
#if CFG_MTK_ANDROID_EMI
	switch (cmd) {
	case 0:
		if (gEmiCalResult != NULL) {
			kalMemFree(gEmiCalResult,
				VIR_MEM_TYPE,
				gEmiCalSize);
			gEmiCalResult = NULL;
		}
		break;

	case 1:
		if (para == 1)
			gEmiCalUseEmiData = TRUE;
		else
			gEmiCalUseEmiData = FALSE;
		break;
	}

	DBGLOG(RFTEST, INFO, "gEmiCalResult(0x%x), gEmiCalUseEmiData(%d)\n",
			gEmiCalResult, gEmiCalUseEmiData);
#endif
}
