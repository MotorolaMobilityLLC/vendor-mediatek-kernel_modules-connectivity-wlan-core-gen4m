/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2021 MediaTek Inc.
 */

#ifndef _NAN_FUNC_H
#define _NAN_FUNC_H

#if !CFG_SUPPORT_NAN_EXT
static inline
void nanExtEnableReq(struct ADAPTER *prAdapter)
{
}

static inline
void nanExtDisableReq(struct ADAPTER *prAdapter)
{
}

static inline
void nanExtClearCustomNdpFaw(uint8_t ucIndex)
{
}

static inline
void nanPeerReportEhtEvent(struct ADAPTER *prAdapter,
					 uint8_t enable)
{
}

static inline
void nanEnableEhtMode(struct ADAPTER *prAdapter, uint8_t mode)
{
}

static inline
void nanEnableEht(struct ADAPTER *prAdapter, uint8_t enable)
{
}

static inline
u_int8_t nanExtHoldNdl(struct _NAN_NDL_INSTANCE_T *prNDL)
{
	return FALSE;
}

static inline
void nanExtBackToNormal(struct ADAPTER *prAdapter)
{
}

static inline
void nanExtResetNdlConfig(struct _NAN_NDL_INSTANCE_T *prNDL)
{
}

static inline
struct _NAN_NDL_INSTANCE_T *nanExtGetReusedNdl(struct ADAPTER *prAdapter)
{
	return NULL;
}

static inline
u32 wlanoidNANExtCmd(struct ADAPTER *prAdapter, void *pvSetBuffer,
		     uint32_t u4SetBufferLen, uint32_t *pu4SetInfoLen)
{
	return -EOPNOTSUPP;
}

static inline
u32 wlanoidNANExtCmdRsp(struct ADAPTER *prAdapter, void *pvSetBuffer,
			uint32_t u4SetBufferLen, uint32_t *pu4SetInfoLen)
{
	return -EOPNOTSUPP;
}

static inline
void nanExtComposeBeaconTrack(struct ADAPTER *prAdapter,
				struct _NAN_EVENT_REPORT_BEACON *prFwEvt)
{
}

static inline
void nanExtComposeClusterEvent(struct ADAPTER *prAdapter,
			       struct NAN_DE_EVENT *prDeEvt)
{
}

static inline
uint32_t nanSchedGetVendorEhtAttr(struct ADAPTER *prAdapter,
				  uint8_t **ppucVendorAttr,
				  uint32_t *pu4VendorAttrLength)
{
	return 0;
}

static inline
uint32_t nanSchedGetVendorAttr(struct ADAPTER *prAdapter,
			       uint8_t **ppucVendorAttr,
			       uint32_t *pu4VendorAttrLength)
{
	return 0;
}


#endif

#endif /* _NAN_FUNC_H */
