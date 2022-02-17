/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/*! \file   "mlo.h"
 *  \brief
 */

#ifndef _MLO_H
#define _MLO_H
#if (CFG_SUPPORT_802_11BE == 1)

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

#define BE_SET_ML_CTRL_PRESENCE_MLD_MAC(_u2ctrl) \
	(_u2ctrl |= (ML_CTRL_MLD_MAC_ADDR_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_UNSET_ML_CTRL_PRESENCE_MLD_MAC(_u2ctrl) \
	(_u2ctrl &= ~(ML_CTRL_MLD_MAC_ADDR_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_IS_ML_CTRL_PRESENCE_MLD_MAC(_u2ctrl) \
	(_u2ctrl & (ML_CTRL_MLD_MAC_ADDR_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_SET_ML_CTRL_PRESENCE_MLD_CAP(_u2ctrl) \
	(_u2ctrl |= (ML_CTRL_MLD_CAPA_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_UNSET_ML_CTRL_PRESENCE_MLD_CAP(_u2ctrl) \
	(_u2ctrl &= ~(ML_CTRL_MLD_CAPA_PRESENT << ML_CTRL_PRE_BMP_SHIFT))

#define BE_IS_ML_CTRL_PRESENCE_MLD_CAP(_u2ctrl) \
	(_u2ctrl & (ML_CTRL_MLD_CAPA_PRESENT << ML_CTRL_PRE_BMP_SHIFT))


#define MLCIE(fp)              ((struct _MULTI_LINK_CONTROL_IE *) fp)

struct _MULTI_LINK_CONTROL_IE {
	u_int8_t  ucId;
	u_int8_t  ucLength;
	u_int8_t  ucExtId;
	u_int16_t ctrl;	/* control field - BITS(0, 2): type, BIT(3): reserved, BITS(4, 15): presence bitmap*/
	u_int8_t commonInfo[1];	/* common field - varied by presence bitmap of control field*/
};

#define MLSTAIE(fp)              ((struct _MULTI_LINK_INFO_STA_CONTROL_IE *) fp)

#define BE_SET_ML_STA_CTRL_LINK_ID(_u2ctrl, _val) \
{\
	(_u2ctrl) &= ~(ML_STA_CTRL_LINK_ID_MASK); \
	(_u2ctrl) |= (((_val) << (ML_STA_CTRL_LINK_ID_SHIFT)) \
	& (ML_STA_CTRL_LINK_ID_MASK)); \
}

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

#define BE_IS_ML_STA_CTRL_PRESENCE_MAC(_u2ctrl) \
	(_u2ctrl & ML_STA_CTRL_MAC_ADDR_PRESENT)


struct _MULTI_LINK_INFO_STA_CONTROL_IE {
	u_int8_t ucSubID;	/* 0: Per-STA Profile */
	u_int8_t ucLength;
	u_int16_t staCtrl;	/* control field - BITS(0, 3): link ID, BIT(4): complete profile, BITS(5, 8): presence bitmap, BITS(9): NSTR bitmap size*/
	u_int8_t staInfo[1];
};

void beReqGenerateMLIE(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo);

void beReqGenerateMultiLinkCommonInfo(
        struct ADAPTER *prAdapter,
        struct MSDU_INFO *prMsduInfo);

void beReqGenerateMultiLinkSTAInfo(
	struct ADAPTER *prAdapter,
	struct MSDU_INFO *prMsduInfo);

//TODO: remove this define
#define MLD_MAC_ADDR "\x00\x11\x22\x33\x44\x55"
#define MLD_STA1_MAC_ADDR "\x00\x11\x22\x33\x11\x11"
#define MLD_STA2_MAC_ADDR "\x00\x11\x22\x33\x22\x22"
#define MLD_CAP 0x33
#define STA1_LINK_ID 9
#define STA2_LINK_ID 10

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

int8_t mldStarecInit(struct ADAPTER *prAdapter);

void mldStarecUninit(struct ADAPTER *prAdapter);

#else

static inline
int8_t mldBssRegister(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO *prMldBssInfo,
	struct BSS_INFO *prBss)
{
	return 0;
}

static inline
int8_t mldBssAlloc(struct ADAPTER *prAdapter,
	struct MLD_BSS_INFO **pprMldBss)
{
	return 0;
}

#define mldBssUnregister(_a, _b, _c)
#define mldBssFree(_a, _b)
#define mldBssInit(_a)
#define mldBssUninit(_a)

static inline
struct MLD_STA_RECORD *mldStarecGetByStarec(struct ADAPTER *prAdapter,
	struct STA_RECORD *prStaRec)
{
	return NULL;
}

#define mldStarecRegister(_a, _b)
#define mldStarecUnregister(_a, _b)
#define mldStarecSetSetupIdx(_a, _b)
#define mldStarecInit(_a)
#define mldStarecUninit(_a)

#endif /* CFG_SUPPORT_802_11BE == 1 */

#endif /* !_MLO_H */
