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
#include "gl_fw_log.h"
#include "fw_log_emi.h"

/*******************************************************************************
 *                              C O N S T A N T S
 *******************************************************************************
 */
#define FW_LOG_STATS_UPDATE_PERIOD		1000

/*******************************************************************************
 *                            P U B L I C   D A T A
 *******************************************************************************
 */

/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
struct FW_LOG_EMI_CTRL g_fw_log_emi_ctx;

/*******************************************************************************
 *                   F U N C T I O N   D E C L A R A T I O N S
 *******************************************************************************
 */
static void fw_log_emi_refresh_sub_header(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl);
static void fw_log_emi_stats_dump(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl);

/*******************************************************************************
 *                              F U N C T I O N S
 *******************************************************************************
 */
static void fw_log_emi_update_rp(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl,
	uint32_t rp)
{
	ACQUIRE_POWER_CONTROL_FROM_PM(ad);
	if (ad->fgIsFwOwn == TRUE) {
		DBGLOG(INIT, WARN,
			"Skip due to driver own failed.\n");
		return;
	}

	DBGLOG(INIT, TRACE,
		"[%d %s] rp: 0x%x\n",
		sub_ctrl->type,
		fw_log_type_to_str(sub_ctrl->type),
		rp);

	ccif_set_fw_log_read_pointer(ad,
				     sub_ctrl->type,
				     rp);

	wlanReleasePowerControl(ad);
}

static u_int8_t fw_log_emi_is_empty(struct FW_LOG_EMI_SUB_CTRL *sub_ctrl)
{
	if (sub_ctrl->irp == sub_ctrl->wp)
		return TRUE;
	else
		return FALSE;
}

static void __fw_log_emi_sub_handler(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl,
	u_int8_t force)
{
	struct FW_LOG_EMI_STATS *stats = &ctrl->stats;
	struct FW_LOG_EMI_SUB_STATS *sub_stats =
		&stats->sub_stats[sub_ctrl->type];
	uint32_t offset = 0, rp = 0, recv = 0, handled = 0;

	fw_log_emi_refresh_sub_header(ad, ctrl, sub_ctrl);

	if (force)
		sub_ctrl->wp = sub_ctrl->iwp;

	if (fw_log_emi_is_empty(sub_ctrl))
		return;

	if (sub_ctrl->wp > sub_ctrl->irp)
		recv = sub_ctrl->wp - sub_ctrl->irp;
	else
		recv = sub_ctrl->length - sub_ctrl->irp + sub_ctrl->wp;
	if (recv > sub_ctrl->length) {
		DBGLOG(INIT, ERROR,
			"Invalid rcv length (%u %u)\n",
			recv, sub_ctrl->length);
		return;
	}
	handled = recv;

	rp = sub_ctrl->irp;
	while (recv) {
		uint32_t size = 0;

		if (rp + recv > sub_ctrl->length)
			size = sub_ctrl->length - rp;
		else
			size = recv;

		DBGLOG(INIT, LOUD,
			"[%d %s] Read data from 0x%x, size: 0x%x\n",
			sub_ctrl->type,
			fw_log_type_to_str(sub_ctrl->type),
			rp,
			size);

		emi_mem_read(ad->chip_info,
			     sub_ctrl->buf_base_addr + rp,
			     sub_ctrl->buffer + offset,
			     size);

		offset += size;
		rp += size;
		rp %= sub_ctrl->length;
		recv -= size;
	}

	handled = fw_log_notify_rcv(sub_ctrl->type,
				    sub_ctrl->buffer,
				    handled);
	sub_stats->handle_size += handled;

	sub_ctrl->irp += handled;
	sub_ctrl->irp %= sub_ctrl->length;
	fw_log_emi_update_rp(ad, ctrl, sub_ctrl, sub_ctrl->irp);
}

static int32_t __fw_log_emi_handler(u_int8_t force)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;
	struct FW_LOG_EMI_STATS *stats = &ctrl->stats;
	struct ADAPTER *ad = NULL;
	uint8_t i = 0;

	ad = (struct ADAPTER *)ctrl->priv;
	if (!ad) {
		stats->skipped++;
		return 0;
	}

	KAL_ACQUIRE_MUTEX(ad, MUTEX_FW_LOG);
	for (i = 0; i < ENUM_FW_LOG_CTRL_TYPE_NUM; i++) {
		struct FW_LOG_EMI_SUB_CTRL *sub_ctrl = &ctrl->sub_ctrls[i];

		__fw_log_emi_sub_handler(ad,
					 ctrl,
					 sub_ctrl,
					 force);
	}
	KAL_RELEASE_MUTEX(ad, MUTEX_FW_LOG);

	stats->handled++;

	fw_log_emi_stats_dump(ad, ctrl);

	return 0;
}

int32_t fw_log_emi_handler(void)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;
	struct FW_LOG_EMI_STATS *stats = &ctrl->stats;

	stats->request++;

	if (!ctrl->initialized) {
		stats->skipped++;
		goto exit;
	} else if (!ctrl->started) {
		ctrl->defered = TRUE;
		stats->skipped++;
		goto exit;
	}

	queue_work(ctrl->wq, &ctrl->work);

exit:
	return 0;
}

static void fw_log_emi_refresh_common_header(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl)
{
	struct mt66xx_chip_info *chip_info = ad->chip_info;
	struct FW_LOG_COMMON_HEADER common = {0};
	uint8_t i = 0;

	ctrl->base_addr = chip_info->fw_log_info.base;

	emi_mem_read(chip_info,
		     ctrl->base_addr,
		     &common,
		     sizeof(common));

	for (i = 0; i < ENUM_FW_LOG_CTRL_TYPE_NUM; i++) {
		struct FW_LOG_EMI_SUB_CTRL *sub_ctrl = &ctrl->sub_ctrls[i];

		switch (i) {
		case ENUM_FW_LOG_CTRL_TYPE_MCU:
			sub_ctrl->base_addr = ctrl->base_addr +
				common.mcu_log_base_addr;
			sub_ctrl->length = common.mcu_log_length;
			break;
		case ENUM_FW_LOG_CTRL_TYPE_WIFI:
			sub_ctrl->base_addr = ctrl->base_addr +
				common.wifi_log_base_addr;
			sub_ctrl->length = common.wifi_log_length;
			break;
		default:
			DBGLOG(INIT, ERROR, "Unknown type: %d\n",
				sub_ctrl->type);
			continue;
		}

		DBGLOG(INIT, TRACE,
			"type: %d %s, base_addr: 0x%x, length: 0x%x\n",
			i,
			fw_log_type_to_str(i),
			sub_ctrl->base_addr,
			sub_ctrl->length);
	}
}

static void fw_log_emi_refresh_sub_header(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl)
{
	struct FW_LOG_SUB_HEADER sub_header = {0};

	emi_mem_read(ad->chip_info,
		     sub_ctrl->base_addr,
		     &sub_header,
		     sizeof(sub_header));

	sub_ctrl->buf_base_addr = sub_ctrl->base_addr + sizeof(sub_header);
	sub_ctrl->rp = sub_header.rp;
	sub_ctrl->wp = sub_header.wp;
	sub_ctrl->iwp = sub_header.internal_wp;

	DBGLOG(INIT, TRACE,
		"[%d %s] rp: 0x%x, irp: 0x%x, wp: 0x%x, iwp: 0x%x\n",
		sub_ctrl->type,
		fw_log_type_to_str(sub_ctrl->type),
		sub_ctrl->rp,
		sub_ctrl->irp,
		sub_ctrl->wp,
		sub_ctrl->iwp);
}

static uint32_t fw_log_emi_sub_ctrl_init(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	enum ENUM_FW_LOG_CTRL_TYPE type)
{
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl = &ctrl->sub_ctrls[type];
	struct FW_LOG_SUB_HEADER sub_header = {0};
	uint32_t status = WLAN_STATUS_SUCCESS;

	sub_ctrl->type = type;

	emi_mem_read(ad->chip_info,
		     sub_ctrl->base_addr,
		     &sub_header,
		     sizeof(sub_header));

	sub_ctrl->buf_base_addr = sub_ctrl->base_addr + sizeof(sub_header);
	if (sub_ctrl->length) {
		sub_ctrl->buffer = kalMemAlloc(sub_ctrl->length,
			VIR_MEM_TYPE);
		if (!sub_ctrl->buffer) {
			DBGLOG(INIT, ERROR, "Alloc buffer failed\n");
			status = WLAN_STATUS_RESOURCES;
		} else {
			kalMemZero(sub_ctrl->buffer, sub_ctrl->length);
		}
	} else {
		status = WLAN_STATUS_INVALID_LENGTH;
	}

	return status;
}

static void fw_log_emi_sub_ctrl_deinit(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl,
	enum ENUM_FW_LOG_CTRL_TYPE type)
{
	struct FW_LOG_EMI_SUB_CTRL *sub_ctrl = &ctrl->sub_ctrls[type];

	if (sub_ctrl->length == 0 || !sub_ctrl->buffer)
		return;

	kalMemFree(sub_ctrl->buffer, VIR_MEM_TYPE, sub_ctrl->length);
}

uint32_t fw_log_emi_start(struct ADAPTER *ad)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;
	uint32_t status = WLAN_STATUS_SUCCESS;
	uint8_t i = 0;

	DBGLOG(INIT, TRACE, "\n");

	if (!ctrl->initialized)
		return 0;

	fw_log_emi_refresh_common_header(ad, ctrl);

	for (i = 0; i < ENUM_FW_LOG_CTRL_TYPE_NUM; i++) {
		status = fw_log_emi_sub_ctrl_init(ad, ctrl, i);
		if (status != WLAN_STATUS_SUCCESS)
			goto exit;
	}

	ctrl->started = TRUE;

	__fw_log_emi_handler(FALSE);

exit:
	return status;
}

void fw_log_emi_stop(struct ADAPTER *ad)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;
	uint8_t i = 0;

	DBGLOG(INIT, TRACE, "\n");

	if (!ctrl->initialized)
		return;

	cancel_work_sync(&ctrl->work);
	__fw_log_emi_handler(TRUE);

	for (i = 0; i < ENUM_FW_LOG_CTRL_TYPE_NUM; i++)
		fw_log_emi_sub_ctrl_deinit(ctrl->priv, ctrl, i);

	ctrl->started = FALSE;
}

static void fw_log_emi_work(struct work_struct *work)
{
	__fw_log_emi_handler(FALSE);
}

uint32_t fw_log_emi_init(struct ADAPTER *ad)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;
	uint32_t status = WLAN_STATUS_SUCCESS;

	DBGLOG(INIT, TRACE, "\n");

	kalMemZero(ctrl, sizeof(*ctrl));

	ctrl->priv = ad;

	ctrl->wq = create_singlethread_workqueue("fw_log_emi");
	if (!ctrl->wq) {
		DBGLOG(INIT, ERROR, "create_singlethread_workqueue failed.\n");
		status = WLAN_STATUS_RESOURCES;
		goto exit;
	}
	INIT_WORK(&ctrl->work, fw_log_emi_work);

	ctrl->initialized = TRUE;

exit:
	if (status != WLAN_STATUS_SUCCESS) {
		DBGLOG(INIT, ERROR, "status: 0x%x\n", status);
		fw_log_emi_deinit(ad);
	}

	return status;
}

void fw_log_emi_deinit(struct ADAPTER *ad)
{
	struct FW_LOG_EMI_CTRL *ctrl = &g_fw_log_emi_ctx;

	DBGLOG(INIT, TRACE, "\n");

	ctrl->initialized = FALSE;

	destroy_workqueue(ctrl->wq);

	ctrl->priv = NULL;
}

static void fw_log_emi_stats_dump(struct ADAPTER *ad,
	struct FW_LOG_EMI_CTRL *ctrl)
{
	struct FW_LOG_EMI_STATS *stats = &ctrl->stats;
	uint8_t buf[512];
	uint32_t written = 0;
	uint8_t i = 0;

	if (time_before(jiffies, stats->update_period))
		return;

	stats->update_period = jiffies +
		FW_LOG_STATS_UPDATE_PERIOD * HZ / 1000;

	kalMemZero(&buf, sizeof(buf));
	written += kalSnprintf(buf + written,
			       sizeof(buf) - written,
			       "fw log stats[0x%x 0x%x 0x%x]",
			       stats->request,
			       stats->skipped,
			       stats->handled);
	for (i = 0; i < ENUM_FW_LOG_CTRL_TYPE_NUM; i++) {
		struct FW_LOG_EMI_SUB_CTRL *sub_ctrl = &ctrl->sub_ctrls[i];

		written += kalSnprintf(buf + written,
				       sizeof(buf) - written,
				       " [%s][0x%x][0x%x 0x%x 0x%x 0x%x]",
				       fw_log_type_to_str(i),
				       stats->sub_stats[i].handle_size,
				       sub_ctrl->rp,
				       sub_ctrl->irp,
				       sub_ctrl->wp,
				       sub_ctrl->iwp);
	}
	DBGLOG(INIT, INFO, "%s\n", buf);
}

