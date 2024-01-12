/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.h"
 *  \brief
 */

#ifndef _MLO_H
#define _MLO_H

#define BE_SET_ML_CTRL_TYPE(_u2ctrl, _ctrl_type) \
{\
	(_u2ctrl) &= ~(ML_CTRL_TYPE_MASK); \
	(_u2ctrl) |= (((_ctrl_type) << (ML_CTRL_TYPE_SHIFT)) \
	& (ML_CTRL_TYPE_MASK)); \
}

#define BE_GET_ML_CTRL_TYPE(_u2ctrl) \
	((_u2ctrl & ML_CTRL_TYPE_MASK) >> ML_CTRL_TYPE_SHIFT)

#define BE_SET_ML_CTRL_PRESENCE(_u2ctrl, _ctrl_type) \
{\
	(_u2ctrl) &= ~(ML_CTRL_PRE_BMP_MASK); \
	(_u2ctrl) |= (((_ctrl_type) << (ML_CTRL_PRE_BMP_SHIFT)) \
	& (ML_CTRL_PRE_BMP_MASK)); \
}

#define BE_GET_ML_CTRL_PRESENCE(_u2ctrl) \
	((_u2ctrl & ML_CTRL_PRE_BMP_MASK) >> ML_CTRL_PRE_BMP_SHIFT)

#define BE_IS_ML_CTRL_PRESENCE_LINK_ID(_u2ctrl) \
	(_u2ctrl & (ML_CTRL_LINK_ID_INFO_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_IS_ML_CTRL_PRESENCE_BSS_PARA_CHANGE_COUNT(_u2ctrl) \
	(_u2ctrl & (ML_CTRL_BSS_PARA_CHANGE_COUNT_PRESENT << \
		ML_CTRL_PRE_BMP_SHIFT))

#define BE_IS_ML_CTRL_PRESENCE_MLD_CAP(_u2ctrl) \
	(_u2ctrl & (ML_CTRL_MLD_CAPA_PRESENT << ML_CTRL_PRE_BMP_SHIFT))


#define MLCIE(fp)              ((struct IE_MULTI_LINK_CONTROL *) fp)

struct IE_MULTI_LINK_CONTROL {
	u_int8_t  ucId;
	u_int8_t  ucLength;
	u_int8_t  ucExtId;
	u_int16_t u2Ctrl;	/* control field - BITS(0, 2): type, BIT(3): reserved, BITS(4, 15): presence bitmap*/
	u_int8_t aucCommonInfo[0];	/* common field - varied by presence bitmap of control field*/
} __KAL_ATTRIB_PACKED__;

#define MLSTAIE(fp)              ((struct IE_MULTI_LINK_INFO_STA_CONTROL *) fp)

#define BE_SET_ML_STA_CTRL_LINK_ID(_u2ctrl, _val) \
{\
	(_u2ctrl) &= ~(ML_STA_CTRL_LINK_ID_MASK); \
	(_u2ctrl) |= (((_val) << (ML_STA_CTRL_LINK_ID_SHIFT)) \
	& (ML_STA_CTRL_LINK_ID_MASK)); \
}

#define BE_SET_ML_STA_CTRL_FIELD(_u2ctrl, _ctrl_type) \
	(_u2ctrl = _ctrl_type)

#define BE_SET_ML_STA_CTRL_COMPLETE_PROFILE(_u2ctrl) \
	(_u2ctrl |= ML_STA_CTRL_COMPLETE_PROFILE)

#define BE_UNSET_ML_STA_CTRL_COMPLETE_PROFILE(_u2ctrl) \
	(_u2ctrl &= ~ML_STA_CTRL_COMPLETE_PROFILE)

#define BE_SET_ML_STA_CTRL_NSTR_BITMAP_SIZE(_u2ctrl, _val) \
{\
	(_u2ctrl) &= ~(ML_STA_CTRL_NSTR_BMP_SIZE); \
	(_u2ctrl) |= (((_val) << (ML_STA_CTRL_NSTR_BMP_SIZE_SHIFT)) \
	& (ML_STA_CTRL_NSTR_BMP_SIZE)); \
}

#define BE_SET_ML_STA_CTRL_PRESENCE_MAC(_u2ctrl) \
	(_u2ctrl |= ML_STA_CTRL_MAC_ADDR_PRESENT)

#define BE_UNSET_ML_STA_CTRL_PRESENCE_MAC(_u2ctrl) \
	(_u2ctrl &= ~ML_STA_CTRL_MAC_ADDR_PRESENT)

#define BE_IS_ML_STA_CTRL_COMPLETE(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_COMPLETE_PROFILE)

#define BE_IS_ML_STA_CTRL_PRESENCE_MAC(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_MAC_ADDR_PRESENT)

#define BE_IS_ML_STA_CTRL_PRESENCE_BCN_INTV(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_BCN_INTV_PRESENT)

#define BE_IS_ML_STA_CTRL_PRESENCE_DTIM(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_DTIM_INFO_PRESENT)

#define BE_IS_ML_STA_CTRL_PRESENCE_NSTR(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_NSTR_LINK_PAIR_PRESENT)

struct IE_ML_PER_STA_PROFILE_INFO {
	uint16_t u2StaCtrl;
	/* Include STA info and STA profile */
	uint8_t aucMultiLinkVarIe[0];
} __KAL_ATTRIB_PACKED__;

struct IE_MULTI_LINK_INFO {
	uint8_t ucId;
	uint8_t ucLength;
	uint8_t ucExtId;
	uint16_t u2MlCtrl;
	/* Include Common info and Link info */
	uint8_t aucMultiLinkVarIe[0];
} __KAL_ATTRIB_PACKED__;

struct IE_MULTI_LINK_INFO_STA_CONTROL {
	u_int8_t ucSubID;	/* 0: Per-STA Profile */
	u_int8_t ucLength;
	u_int16_t u2StaCtrl;	/* control field - BITS(0, 3): link ID, BIT(4): complete profile, BITS(5, 8): presence bitmap, BITS(9): NSTR bitmap size*/
	u_int8_t aucStaInfo[0];
} __KAL_ATTRIB_PACKED__;

struct IE_NON_INHERITANCE {
	uint8_t ucId;
	uint8_t ucLength;
	uint8_t ucExtId;
	uint8_t aucList[0];
} __KAL_ATTRIB_PACKED__;

struct STA_PROFILE {
	uint16_t u2StaCtrl;
	uint8_t ucLinkId;
	uint8_t aucLinkAddr[MAC_ADDR_LEN];
	uint16_t u2BcnIntv;
	uint16_t u2DtimInfo;
	uint16_t u2NstrBmp;
	struct RF_CHANNEL_INFO rChnlInfo;
	uint8_t ucChangeSeq;
	uint8_t aucIEbuf[256];
};

struct MULTI_LINK_INFO {
	uint8_t ucValid;
	uint8_t	ucMlCtrlPreBmp;
	uint8_t ucCommonInfoLength;
	uint8_t aucMldAddr[MAC_ADDR_LEN];
	uint8_t ucLinkId;
	uint8_t ucBssParaChangeCount;
	uint16_t u2MediumSynDelayInfo;
	uint16_t u2EmlCap;
	uint16_t u2MldCap;
	uint8_t ucLinkNum;
	struct STA_PROFILE rStaProfiles[MLD_LINK_MAX];
};

typedef struct MSDU_INFO* (*PFN_COMPOSE_ASSOC_IE_FUNC) (struct ADAPTER *,
	struct STA_RECORD *);

typedef struct MSDU_INFO* (*PFN_COMPOSE_AUTH_IE_FUNC) (struct ADAPTER *,
	  struct STA_RECORD *, uint8_t, struct SW_RFB *, uint16_t, uint16_t);

typedef struct MSDU_INFO* (*PFN_COMPOSE_BEACON_IE_FUNC) (struct ADAPTER *,
	uint8_t);

void beGenerateAssocMldIE(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_ASSOC_IE_FUNC pfnComposeIE);

void beGenerateAuthMldIE(
	struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucBssIndex,
	struct SW_RFB *prRecvAuthSwRfb,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_AUTH_IE_FUNC pfnComposeIE);

void beGenerateBeaconMldIE(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	uint8_t ucComplete,
	struct MSDU_INFO *prMsduInfo,
	PFN_COMPOSE_BEACON_IE_FUNC pfnComposeIE);

struct IE_MULTI_LINK_CONTROL *beGenerateMldCommonInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo);

void beGenerateMldSTAInfo(
	struct ADAPTER *prAdapter,
	struct IE_MULTI_LINK_CONTROL *prMultiLinkControlIE,
	struct MSDU_INFO *prMsduInfo,
	uint32_t u4BeginOffset,
	uint32_t u4PrimaryLength,
	struct MSDU_INFO *prMsduInfoSta,
	uint8_t ucBssIndex);

uint32_t beCalculateRnrIELen(
	struct ADAPTER *prAdapter,
	uint8_t ucBssIndex,
	struct STA_RECORD *prStaRec);

void beGenerateRnrIE(struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo);

void beParsingMldElement(IN struct MULTI_LINK_INFO *prMlInfo,
	IN uint8_t *pucIE);

void mldBssDump(struct ADAPTER *prAdapter);

int8_t mldBssRegister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBss);

void mldBssUnregister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBss);

int8_t mldBssAlloc(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO **pprMldBss);

void mldBssFree(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo);

struct MLD_BSS_INFO *mldBssGetByBss(struct ADAPTER *prAdapter,
	struct BSS_INFO *prBssInfo);

struct MLD_BSS_INFO *mldBssGetByIdx(struct ADAPTER *prAdapter,
	uint8_t ucIdx);

int8_t mldBssInit(struct ADAPTER *prAdapter);

void mldBssUninit(struct ADAPTER *prAdapter);

void mldStarecDump(struct ADAPTER *prAdapter);

int8_t mldStarecRegister(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStarec);

void mldStarecUnregister(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStarec);

int8_t mldStarecAlloc(struct ADAPTER *prAdapter,
	struct MLD_STA_RECORD **pprMldStarec,
	uint8_t *aucMacAddr);

void mldStarecFree(struct ADAPTER *prAdapter,
	struct MLD_STA_RECORD *prMldStarec);

struct MLD_STA_RECORD *mldStarecGetByStarec(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec);

int8_t mldStarecSetSetupIdx(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec);

void mldStarecLogRxData(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec,
	uint8_t ucHwBandIdx);

int8_t mldStarecInit(struct ADAPTER *prAdapter);

void mldStarecUninit(struct ADAPTER *prAdapter);

struct BSS_INFO *mldGetBssInfoByLinkID(
	struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	uint8_t ucLinkIndex,
	uint8_t fgPeerSta);

#endif /* !_MLO_H */
