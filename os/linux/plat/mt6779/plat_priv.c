/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See http://www.gnu.org/licenses/gpl-2.0.html for more details.
*/

#include <cpu_ctrl.h>
#include <sched_ctl.h>
#include <eas_ctrl.h>
#include <topo_ctrl.h>
#ifdef WLAN_FORCE_DDR_OPP
#include <linux/pm_qos.h>
#endif

#include "precomp.h"

#ifdef CONFIG_MTK_EMI
#include <mt_emi_api.h>
#define WIFI_EMI_MEM_OFFSET    0x236000
#define WIFI_EMI_MEM_SIZE      0xD2000
#endif


#define MAX_CPU_FREQ (3 * 1024 * 1024) /* in kHZ */
#define MAX_CLUSTER_NUM  3

uint32_t kalGetCpuBoostThreshold(void)
{
	DBGLOG(SW4, INFO, "enter kalGetCpuBoostThreshold\n");
	/*  8, stands for 450Mbps */
	return 8;
}

int32_t kalBoostCpu(IN struct ADAPTER *prAdapter,
		    IN uint32_t u4TarPerfLevel,
		    IN uint32_t u4BoostCpuTh)
{
	struct ppm_limit_data freq_to_set[MAX_CLUSTER_NUM];
	int32_t i = 0, i4Freq = -1, ret = 0;
	struct GLUE_INFO *prGlueInfo;

#ifdef WLAN_FORCE_DDR_OPP
	static struct pm_qos_request wifi_qos_request;
	static u_int8_t fgRequested;
#endif
	uint32_t u4ClusterNum = topo_ctrl_get_nr_clusters();

	ASSERT(prAdapter);
	ASSERT(u4ClusterNum <= MAX_CLUSTER_NUM);
	/* ACAO, we dont have to set core number */

	/*  5, stands for 250Mbps */
	if (u4TarPerfLevel >= 5) {
		prGlueInfo = prAdapter->prGlueInfo;
		ASSERT(prGlueInfo);

		/* Set driver threads uclamp */
		if (prGlueInfo->u4RxThreadPid != 0xffffffff) {
			ret = set_task_uclamp(prGlueInfo->u4RxThreadPid, 100);
			if (ret != 0)
				DBGLOG(SW4, WARN, "set uclamp Rx failed\n");
		}
		if (prGlueInfo->u4HifThreadPid != 0xffffffff) {
			ret = set_task_uclamp(prGlueInfo->u4HifThreadPid, 100);
			if (ret != 0)
				DBGLOG(SW4, WARN, "set uclamp Hif failed\n");
		}
		if (prGlueInfo->u4TxThreadPid != 0xffffffff) {
			ret = set_task_uclamp(prGlueInfo->u4TxThreadPid, 100);
			if (ret != 0)
				DBGLOG(SW4, WARN, "set uclamp Tx failed\n");
		}
		/* Set Top APP threads uclamp */
		update_eas_uclamp_min(EAS_UCLAMP_KIR_WIFI, CGROUP_TA, 100);

	} else {
		set_task_uclamp(prAdapter->prGlueInfo->u4RxThreadPid, 0);
		set_task_uclamp(prAdapter->prGlueInfo->u4HifThreadPid, 0);
		set_task_uclamp(prAdapter->prGlueInfo->u4TxThreadPid, 0);
		update_eas_uclamp_min(EAS_UCLAMP_KIR_WIFI, CGROUP_TA, 0);
	}

	/*  Default u4BoostCpuTh set as 8, stands for 450Mbps */
	if (u4TarPerfLevel >= u4BoostCpuTh) {
		/* Boost CPU freq */
		i4Freq = MAX_CPU_FREQ;

		/* Prefer big core */
		set_sched_boost(SCHED_ALL_BOOST);

	} else {
		i4Freq = -1;
		set_sched_boost(SCHED_NO_BOOST);
	}
	/* update cpu freq */
	for (i = 0; i < u4ClusterNum; i++) {
		freq_to_set[i].min = i4Freq;
		freq_to_set[i].max = i4Freq;
	}
	update_userlimit_cpu_freq(CPU_KIR_WIFI, u4ClusterNum, freq_to_set);

#ifdef WLAN_FORCE_DDR_OPP
	if (u4TarPerfLevel >= u4BoostCpuTh) {
		if (!fgRequested) {
			fgRequested = 1;
			pm_qos_add_request(&wifi_qos_request,
					   PM_QOS_DDR_OPP,
					   DDR_OPP_0);
		}
		pm_qos_update_request(&wifi_qos_request, DDR_OPP_0);
	} else if (fgRequested) {
		pm_qos_update_request(&wifi_qos_request, DDR_OPP_UNREQ);
		pm_qos_remove_request(&wifi_qos_request);
		fgRequested = 0;
	}
#endif
	return 0;
}

#ifdef CONFIG_MTK_EMI
void kalSetEmiMpuProtection(phys_addr_t emiPhyBase, bool enable)
{
	struct emi_region_info_t region_info;

	/*set MPU for EMI share Memory */
	region_info.start = emiPhyBase + WIFI_EMI_MEM_OFFSET;
	region_info.end = emiPhyBase + WIFI_EMI_MEM_OFFSET
		+ WIFI_EMI_MEM_SIZE - 1;
	region_info.region = 26;

	SET_ACCESS_PERMISSION(region_info.apc, enable ? LOCK : UNLOCK,
			      FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, enable ? FORBIDDEN : NO_PROTECTION);
	emi_mpu_set_protection(&region_info);
}

void kalSetDrvEmiMpuProtection(phys_addr_t emiPhyBase, uint32_t offset,
			       uint32_t size)
{
	struct emi_region_info_t region_info;

	/*set MPU for EMI share Memory */
	region_info.start = emiPhyBase + offset;
	region_info.end = emiPhyBase + offset + size - 1;
	region_info.region = 18;
	SET_ACCESS_PERMISSION(region_info.apc, LOCK, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, FORBIDDEN,
			      FORBIDDEN, FORBIDDEN, FORBIDDEN, NO_PROTECTION,
			      FORBIDDEN, NO_PROTECTION);
	emi_mpu_set_protection(&region_info);
}
#endif
