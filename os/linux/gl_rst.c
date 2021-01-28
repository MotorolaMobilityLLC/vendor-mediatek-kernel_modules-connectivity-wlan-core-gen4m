/*******************************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 *
 * BSD LICENSE
 *
 * Copyright(C) 2016 MediaTek Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
/*
 ** Id: @(#) gl_rst.c@@
 */

/*! \file   gl_rst.c
 *    \brief  Main routines for supporintg MT6620 whole-chip reset mechanism
 *
 *    This file contains the support routines of Linux driver for MediaTek Inc.
 *    802.11 Wireless LAN Adapters.
 */


/*******************************************************************************
 *                         C O M P I L E R   F L A G S
 *******************************************************************************
 */

/*******************************************************************************
 *                    E X T E R N A L   R E F E R E N C E S
 *******************************************************************************
 */
#include <linux/kernel.h>
#include <linux/workqueue.h>

#include "precomp.h"
#include "gl_rst.h"

#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
#include "fw_log_wifi.h"
#endif

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */



/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */
u_int8_t fgSimplifyResetFlow = FALSE;
uint64_t u8ResetTime;

#if CFG_CHIP_RESET_HANG
u_int8_t fgIsResetHangState = SER_L0_HANG_RST_NONE;
#endif

#if (CFG_SUPPORT_CONNINFRA == 1)
uint32_t g_u4WlanRstThreadPid;
wait_queue_head_t g_waitq_rst;
unsigned long g_ulFlag;/* GLUE_FLAG_XXX */
struct completion g_RstComp;
struct completion g_triggerComp;
KAL_WAKE_LOCK_T g_IntrWakeLock;
struct task_struct *wlan_reset_thread;
static int g_rst_data;
u_int8_t g_IsWholeChipRst = FALSE;
u_int8_t g_SubsysRstCnt;
u_int8_t g_IsSubsysRstOverThreshold = FALSE;
u_int8_t g_IsWfsysBusHang = FALSE;
char *g_reason;
#endif
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static enum _ENUM_CHIP_RESET_REASON_TYPE_T eResetReason;

#if CFG_CHIP_RESET_SUPPORT
static struct RESET_STRUCT wifi_rst;
u_int8_t fgIsResetting = FALSE;
#endif

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
#if CFG_CHIP_RESET_SUPPORT
static void mtk_wifi_reset(struct work_struct *work);

#if CFG_WMT_RESET_API_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 0)
static void mtk_wifi_trigger_reset(struct work_struct *work);
static void *glResetCallback(enum ENUM_WMTDRV_TYPE eSrcType,
			     enum ENUM_WMTDRV_TYPE eDstType,
			     enum ENUM_WMTMSG_TYPE eMsgType, void *prMsgBody,
			     unsigned int u4MsgLength);
#endif /*end of CFG_SUPPORT_CONNINFRA == 0*/
#else
static u_int8_t is_bt_exist(void);
static u_int8_t rst_L0_notify_step1(void);
static void wait_core_dump_end(void);
#endif
#endif

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
void glSetRstReason(enum _ENUM_CHIP_RESET_REASON_TYPE_T
		    eReason)
{
	if (kalIsResetting())
		return;

	u8ResetTime = sched_clock();
	eResetReason = eReason;
}

int glGetRstReason(void)
{
	return eResetReason;
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for checking if connectivity chip is resetting
 *
 * @param   None
 *
 * @retval  TRUE
 *          FALSE
 */
/*----------------------------------------------------------------------------*/
u_int8_t kalIsResetting(void)
{
#if CFG_CHIP_RESET_SUPPORT
	return fgIsResetting;
#else
	return FALSE;
#endif
}

#if CFG_CHIP_RESET_SUPPORT

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. register wifi reset callback
 *        2. initialize wifi reset work
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void glResetInit(struct GLUE_INFO *prGlueInfo)
{
#if CFG_WMT_RESET_API_SUPPORT
	/* 1. Register reset callback */
#if (CFG_SUPPORT_CONNINFRA == 0)
	mtk_wcn_wmt_msgcb_reg(WMTDRV_TYPE_WIFI,
			      (PF_WMT_CB) glResetCallback);
	/* 2. Initialize reset work */
	INIT_WORK(&(wifi_rst.rst_trigger_work),
		  mtk_wifi_trigger_reset);
#endif
#endif
	fgIsResetting = FALSE;
	wifi_rst.prGlueInfo = prGlueInfo;
	INIT_WORK(&(wifi_rst.rst_work), mtk_wifi_reset);
#if (CFG_SUPPORT_CONNINFRA == 1)

#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
	fw_log_connsys_coredump_init();
#endif

	KAL_WAKE_LOCK_INIT(NULL, &g_IntrWakeLock, "WLAN Reset");
	init_waitqueue_head(&g_waitq_rst);
	init_completion(&g_RstComp);
	init_completion(&g_triggerComp);
	wlan_reset_thread = kthread_run(wlan_reset_thread_main,
					&g_rst_data, "wlan_rst_thread");
	g_SubsysRstCnt = 0;

#endif /* CFG_SUPPORT_CONNINFRA */
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is responsible for
 *        1. deregister wifi reset callback
 *
 * @param none
 *
 * @retval none
 */
/*----------------------------------------------------------------------------*/
void glResetUninit(void)
{
#if CFG_WMT_RESET_API_SUPPORT
	/* 1. Deregister reset callback */
#if (CFG_SUPPORT_CONNINFRA == 0)
	mtk_wcn_wmt_msgcb_unreg(WMTDRV_TYPE_WIFI);
#else

#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
	fw_log_connsys_coredump_deinit();
#endif

	set_bit(GLUE_FLAG_HALT_BIT, &g_ulFlag);
	wake_up_interruptible(&g_waitq_rst);
#endif
#endif
}
/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for generating reset request to WMT
 *
 * @param   None
 *
 * @retval  None
 */
/*----------------------------------------------------------------------------*/
void glSendResetRequest(void)
{
#if CFG_WMT_RESET_API_SUPPORT

	/* WMT thread would trigger whole chip reset itself */
#endif
}

u_int8_t glResetTrigger(struct ADAPTER *prAdapter,
		uint32_t u4RstFlag, const uint8_t *pucFile, uint32_t u4Line)
{
	u_int8_t fgResult = TRUE;
	uint16_t u2FwOwnVersion;
	uint16_t u2FwPeerVersion;
#if (CFG_SUPPORT_CONNINFRA == 1)
	struct mt66xx_chip_info *prChipInfo;
#endif
	dump_stack();
	if (kalIsResetting())
		return fgResult;

	fgIsResetting = TRUE;
#if (CFG_SUPPORT_CONNINFRA == 0)
	if (eResetReason != RST_BT_TRIGGER)
		DBGLOG(INIT, STATE, "[SER][L0] wifi trigger eResetReason=%d\n",
								eResetReason);
	else
		DBGLOG(INIT, STATE, "[SER][L0] BT trigger\n");
#endif

#if CFG_WMT_RESET_API_SUPPORT
	if (u4RstFlag & RST_FLAG_DO_CORE_DUMP)
		if (glIsWmtCodeDump())
			DBGLOG(INIT, WARN, "WMT is code dumping !\n");
#endif
	if (prAdapter == NULL)
		prAdapter = wifi_rst.prGlueInfo->prAdapter;
#if (CFG_SUPPORT_CONNINFRA == 1)
	prChipInfo = prAdapter->chip_info;
#endif
	u2FwOwnVersion = prAdapter->rVerInfo.u2FwOwnVersion;
	u2FwPeerVersion = prAdapter->rVerInfo.u2FwPeerVersion;

	DBGLOG(INIT, ERROR,
		"Trigger chip reset in %s line %u! Chip[%04X E%u] FW Ver DEC[%u.%u] HEX[%x.%x], Driver Ver[%u.%u]\n",
		 pucFile, u4Line, MTK_CHIP_REV,
	wlanGetEcoVersion(prAdapter),
		(uint16_t)(u2FwOwnVersion >> 8),
		(uint16_t)(u2FwOwnVersion & BITS(0, 7)),
		(uint16_t)(u2FwOwnVersion >> 8),
		(uint16_t)(u2FwOwnVersion & BITS(0, 7)),
		(uint16_t)(u2FwPeerVersion >> 8),
		(uint16_t)(u2FwPeerVersion & BITS(0, 7)));

	prAdapter->u4HifDbgFlag |= DEG_HIF_DEFAULT_DUMP;
	halPrintHifDbgInfo(prAdapter);

#if CFG_WMT_RESET_API_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 0)
	wifi_rst.rst_trigger_flag = u4RstFlag;
	schedule_work(&(wifi_rst.rst_trigger_work));
#else
	if (u4RstFlag & RST_FLAG_DO_WHOLE_RESET) {
		if (prChipInfo->trigger_wholechiprst)
			prChipInfo->trigger_wholechiprst(g_reason);
	} else {
		if (prChipInfo->triggerfwassert)
			prChipInfo->triggerfwassert();
	}
#endif /*end of CFG_SUPPORT_CONNINFRA == 0*/

#else
	wifi_rst.prGlueInfo = prAdapter->prGlueInfo;
	schedule_work(&(wifi_rst.rst_work));
#endif

	return fgResult;
}




/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is called for wifi reset
 *
 * @param   skb
 *          info
 *
 * @retval  0
 *          nonzero
 */
/*----------------------------------------------------------------------------*/
static void mtk_wifi_reset(struct work_struct *work)
{
	struct RESET_STRUCT *rst = container_of(work,
						struct RESET_STRUCT, rst_work);
	u_int8_t fgResult = FALSE;

#if CFG_WMT_RESET_API_SUPPORT
	/* wlanOnAtReset(); */
	wifi_reset_end(rst->rst_data);
#else
	fgResult = rst_L0_notify_step1();

	wait_core_dump_end();

	fgResult = rst->prGlueInfo->prAdapter->chip_info->rst_L0_notify_step2();

#if CFG_CHIP_RESET_HANG
	if (fgIsResetHangState == SER_L0_HANG_RST_NONE)
		fgIsResetHangState = SER_L0_HANG_RST_TRGING;
#endif

	if (is_bt_exist() == FALSE)
		kalRemoveProbe(rst->prGlueInfo);

#endif
	if (fgSimplifyResetFlow) {
		DBGLOG(INIT, INFO, "Force down the reset flag.\n");
		fgSimplifyResetFlow = FALSE;
	}
	DBGLOG(INIT, STATE, "[SER][L0] flow end, fgResult=%d\n", fgResult);

}


#if CFG_WMT_RESET_API_SUPPORT
#if (CFG_SUPPORT_CONNINFRA == 0)
static void mtk_wifi_trigger_reset(struct work_struct *work)
{
	u_int8_t fgResult = FALSE;
	struct RESET_STRUCT *rst = container_of(work,
					struct RESET_STRUCT, rst_trigger_work);

	fgIsResetting = TRUE;
	/* Set the power off flag to FALSE in WMT to prevent chip power off
	 * after wlanProbe return failure, because we need to do core dump
	 * afterward.
	 */
	if (rst->rst_trigger_flag & RST_FLAG_PREVENT_POWER_OFF)
		mtk_wcn_set_connsys_power_off_flag(FALSE);

	fgResult = mtk_wcn_wmt_assert_timeout(WMTDRV_TYPE_WIFI, 0x40, 0);
	DBGLOG(INIT, INFO, "reset result %d, trigger flag 0x%x\n",
				fgResult, rst->rst_trigger_flag);
}
#endif
/* Weak reference for those platform doesn't support wmt functions */
u_int8_t __weak mtk_wcn_stp_coredump_start_get(void)
{
	return FALSE;
}


/*0= f/w assert flag is not set, others=f/w assert flag is set */
u_int8_t glIsWmtCodeDump(void)
{
	return mtk_wcn_stp_coredump_start_get();
}

/*----------------------------------------------------------------------------*/
/*!
 * @brief This routine is invoked when there is reset messages indicated
 *
 * @param   eSrcType
 *          eDstType
 *          eMsgType
 *          prMsgBody
 *          u4MsgLength
 *
 * @retval
 */
/*----------------------------------------------------------------------------*/
#if (CFG_SUPPORT_CONNINFRA == 0)
static void *glResetCallback(enum ENUM_WMTDRV_TYPE eSrcType,
			     enum ENUM_WMTDRV_TYPE eDstType,
			     enum ENUM_WMTMSG_TYPE eMsgType, void *prMsgBody,
			     unsigned int u4MsgLength)
{
	switch (eMsgType) {
	case WMTMSG_TYPE_RESET:
		if (u4MsgLength == sizeof(enum ENUM_WMTRSTMSG_TYPE)) {
			enum ENUM_WMTRSTMSG_TYPE *prRstMsg =
					(enum ENUM_WMTRSTMSG_TYPE *) prMsgBody;

			switch (*prRstMsg) {
			case WMTRSTMSG_RESET_START:
				DBGLOG(INIT, WARN, "Whole chip reset start!\n");
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				break;

			case WMTRSTMSG_RESET_END:
				DBGLOG(INIT, WARN, "Whole chip reset end!\n");
				wifi_rst.rst_data = RESET_SUCCESS;
				fgIsResetting = FALSE;
				schedule_work(&(wifi_rst.rst_work));
				break;

			case WMTRSTMSG_RESET_END_FAIL:
				DBGLOG(INIT, WARN, "Whole chip reset fail!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_FAIL;
				schedule_work(&(wifi_rst.rst_work));
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	return NULL;
}
#else

void glSetRstReasonString(char *reason)
{
	g_reason = reason;
}


static u_int8_t glResetMsgHandler(enum ENUM_WMTMSG_TYPE eMsgType,
				  enum ENUM_WMTRSTMSG_TYPE MsgBody)
{
	switch (eMsgType) {
	case WMTMSG_TYPE_RESET:

			switch (MsgBody) {
			case WMTRSTMSG_RESET_START:
				DBGLOG(INIT, WARN, "Whole chip reset start!\n");
				fgIsResetting = TRUE;
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				hifAxiRemove();
				complete(&g_RstComp);
				break;

			case WMTRSTMSG_RESET_END:
				DBGLOG(INIT, WARN, "WF reset end!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_SUCCESS;
				schedule_work(&(wifi_rst.rst_work));
				break;

			case WMTRSTMSG_RESET_END_FAIL:
				DBGLOG(INIT, WARN, "Whole chip reset fail!\n");
				fgIsResetting = FALSE;
				wifi_rst.rst_data = RESET_FAIL;
				schedule_work(&(wifi_rst.rst_work));
				break;
			case WMTRSTMSG_0P5RESET_START:
				DBGLOG(INIT, WARN, "WF chip reset start!\n");
				fgIsResetting = TRUE;
				fgSimplifyResetFlow = TRUE;
				wifi_reset_start();
				hifAxiRemove();
				break;
			default:
				break;
			}
		break;

	default:
		break;
	}

	return TRUE;
}

int glRstwlanPreWholeChipReset(enum consys_drv_type type, char *reason)
{
	uint32_t waitRet = 0;
	bool bRet = TRUE;
	struct GLUE_INFO *prGlueInfo;

	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());
	g_IsWholeChipRst = TRUE;
	DBGLOG(INIT, INFO,
		"Enter glRstwlanPreWholeChipReset (%d).\n",
		g_IsWholeChipRst);

	if (!g_IsSubsysRstOverThreshold) {
		if (!kalIsResetting()) {
			DBGLOG(INIT, INFO,
				"Wi-Fi Driver processes whole chip reset start.\n");
		GL_RESET_TRIGGER(prGlueInfo->prAdapter, RST_FLAG_WF_RESET);
		} else {
			DBGLOG(INIT, INFO, "Wi-Fi is doing Subsys reset.\n");
			kalSetRstEvent();
		}
	} else {
		DBGLOG(INIT, INFO, "Reach subsys reset threshold!!!\n");
		kalSetRstEvent();
	}
	waitRet = wait_for_completion_timeout(&g_RstComp,
					MSEC_TO_JIFFIES(WIFI_RST_TIMEOUT));
	if (waitRet > 0) {
		/* Case 1: No timeout. */
		DBGLOG(INIT, INFO, "Wi-Fi is off successfully.\n");
	} else {
		/* Case 2: timeout TBD*/
		DBGLOG(INIT, ERROR,
			"WiFi rst takes more than 4 seconds, trigger rst self\n");
	}
	return bRet;
}

int glRstwlanPostWholeChipReset(void)
{
	glResetMsgHandler(WMTMSG_TYPE_RESET, WMTRSTMSG_RESET_END);
	g_IsWholeChipRst = FALSE;
	DBGLOG(INIT, INFO,
		"Leave glRstwlanPostWholeChipReset (%d).\n",
		g_IsWholeChipRst);
	return 0;
}
u_int8_t kalIsWholeChipResetting(void)
{
#if CFG_CHIP_RESET_SUPPORT
	return g_IsWholeChipRst;
#else
	return FALSE;
#endif
}

int wlan_reset_thread_main(void *data)
{
	int ret = 0;
	struct GLUE_INFO *prGlueInfo = NULL;

#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
	KAL_WAKE_LOCK_T *prWlanRstThreadWakeLock;

	prWlanRstThreadWakeLock = kalMemAlloc(sizeof(KAL_WAKE_LOCK_T),
		VIR_MEM_TYPE);
	if (!prWlanRstThreadWakeLock) {
		DBGLOG(INIT, ERROR, "%s MemAlloc Fail\n",
			KAL_GET_CURRENT_THREAD_NAME());
		return 0;
	}

	KAL_WAKE_LOCK_INIT(NULL,
			   prWlanRstThreadWakeLock, "WLAN rst_thread");
	KAL_WAKE_LOCK(NULL, prWlanRstThreadWakeLock);
#endif
	prGlueInfo = (struct GLUE_INFO *) wiphy_priv(wlanGetWiphy());

	DBGLOG(INIT, INFO, "%s:%u starts running...\n",
	       KAL_GET_CURRENT_THREAD_NAME(), KAL_GET_CURRENT_THREAD_ID());

	g_u4WlanRstThreadPid = KAL_GET_CURRENT_THREAD_ID();

	while (TRUE) {
		/* Unlock wakelock if hif_thread going to idle */
		KAL_WAKE_UNLOCK(NULL, prWlanRstThreadWakeLock);
		/*
		 * sleep on waitqueue if no events occurred. Event contain
		 * (1) GLUE_FLAG_HALT (2) GLUE_FLAG_RST
		 *
		 */
		do {
			ret = wait_event_interruptible(g_waitq_rst,
				((g_ulFlag & GLUE_FLAG_RST_PROCESS)
				!= 0));
		} while (ret != 0);
#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
		if (!KAL_WAKE_LOCK_ACTIVE(NULL,
					  prWlanRstThreadWakeLock))
			KAL_WAKE_LOCK(NULL,
				      prWlanRstThreadWakeLock);
#endif
		if (test_and_clear_bit(GLUE_FLAG_RST_START_BIT, &g_ulFlag)) {
			if (KAL_WAKE_LOCK_ACTIVE(NULL, &g_IntrWakeLock))
				KAL_WAKE_UNLOCK(NULL, &g_IntrWakeLock);

#if (CFG_ANDORID_CONNINFRA_COREDUMP_SUPPORT == 1)
			fw_log_connsys_coredump_start();
#endif

			if (g_IsWholeChipRst) {
				glResetMsgHandler(WMTMSG_TYPE_RESET,
							WMTRSTMSG_RESET_START);
				g_IsSubsysRstOverThreshold = FALSE;
				g_SubsysRstCnt = 0;
			} else {
				if (g_SubsysRstCnt < 3) {
					g_SubsysRstCnt++;
					DBGLOG(INIT, INFO,
						"WF reset count = %d\n",
						g_SubsysRstCnt);
					glResetMsgHandler(WMTMSG_TYPE_RESET,
						WMTRSTMSG_0P5RESET_START);
					glResetMsgHandler(WMTMSG_TYPE_RESET,
							WMTRSTMSG_RESET_END);
				} else {
					g_SubsysRstCnt = 0;
					g_IsSubsysRstOverThreshold = TRUE;
					glSetRstReasonString(
							"subsys reset more than 3 times");
					GL_RESET_TRIGGER(prGlueInfo->prAdapter,
							RST_FLAG_WHOLE_RESET);
				}
			}
		}
		if (test_and_clear_bit(GLUE_FLAG_HALT_BIT, &g_ulFlag)) {
			DBGLOG(INIT, INFO, "rst_thread should stop now...\n");
			break;
		}
	}

#if defined(CONFIG_ANDROID) && (CFG_ENABLE_WAKE_LOCK)
	if (KAL_WAKE_LOCK_ACTIVE(NULL,
				 prWlanRstThreadWakeLock))
		KAL_WAKE_UNLOCK(NULL, prWlanRstThreadWakeLock);
	KAL_WAKE_LOCK_DESTROY(NULL,
			      prWlanRstThreadWakeLock);
	kalMemFree(prWlanRstThreadWakeLock, VIR_MEM_TYPE,
		sizeof(KAL_WAKE_LOCK_T));
#endif

	DBGLOG(INIT, TRACE, "%s:%u stopped!\n",
	       KAL_GET_CURRENT_THREAD_NAME(), KAL_GET_CURRENT_THREAD_ID());

	return 0;
}
#endif
#else
static u_int8_t is_bt_exist(void)
{
	typedef int (*p_bt_fun_type) (int);
	p_bt_fun_type bt_func;
	char *bt_func_name = "WF_rst_L0_notify_BT_step1";

	bt_func = (p_bt_fun_type) kallsyms_lookup_name(bt_func_name);
	if (bt_func)
		return TRUE;

	DBGLOG(INIT, ERROR, "[SER][L0] %s does not exist\n", bt_func_name);
	return FALSE;

}

static u_int8_t rst_L0_notify_step1(void)
{
	if (eResetReason != RST_BT_TRIGGER) {
		typedef int (*p_bt_fun_type) (int);
		p_bt_fun_type bt_func;
		char *bt_func_name = "WF_rst_L0_notify_BT_step1";

		DBGLOG(INIT, STATE, "[SER][L0] %s\n", bt_func_name);
		bt_func = (p_bt_fun_type) kallsyms_lookup_name(bt_func_name);
		if (bt_func) {
			bt_func(0);
		} else {
			DBGLOG(INIT, ERROR,
				"[SER][L0] %s does not exist\n", bt_func_name);
			return FALSE;
		}
	}

	return TRUE;
}

static void wait_core_dump_end(void)
{
#ifdef CFG_SUPPORT_CONNAC2X
	if (eResetReason == RST_OID_TIMEOUT)
		return;
	DBGLOG(INIT, WARN, "[SER][L0] not support..\n");
#endif
}

int32_t BT_rst_L0_notify_WF_step1(int32_t reserved)
{
	glSetRstReason(RST_BT_TRIGGER);
	GL_RESET_TRIGGER(NULL, RST_FLAG_CHIP_RESET);

	return 0;
}
EXPORT_SYMBOL(BT_rst_L0_notify_WF_step1);

int32_t BT_rst_L0_notify_WF_2(int32_t reserved)
{
	DBGLOG(INIT, WARN, "[SER][L0] not support...\n");

	return 0;
}
EXPORT_SYMBOL(BT_rst_L0_notify_WF_2);

#endif
#endif


