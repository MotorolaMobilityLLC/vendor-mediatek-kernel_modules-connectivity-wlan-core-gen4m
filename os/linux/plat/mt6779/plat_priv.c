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
#include <topo_ctrl.h>
#include "precomp.h"
#include "wmt_exp.h"

#ifdef CONFIG_MTK_EMI
#include <mt_emi_api.h>
#define WIFI_EMI_MEM_OFFSET    0x1D0000
#define WIFI_EMI_MEM_SIZE      0x140000
#endif

#define MAX_CPU_FREQ (3 * 1024 * 1024) /* in kHZ */
#define MAX_CLUSTER_NUM  3

uint32_t kalGetCpuBoostThreshold(void)
{
	DBGLOG(SW4, TRACE, "enter kalGetCpuBoostThreshold\n");
	/*  5, stands for 250Mbps */
	return 5;
}

int32_t kalBoostCpu(IN struct ADAPTER *prAdapter,
		    IN uint32_t u4TarPerfLevel,
		    IN uint32_t u4BoostCpuTh)
{
	struct cpu_ctrl_data freq_to_set[MAX_CLUSTER_NUM];
	int32_t i = 0, i4Freq = -1;

	uint32_t u4ClusterNum = topo_ctrl_get_nr_clusters();

	ASSERT(u4ClusterNum <= MAX_CLUSTER_NUM);
	/* ACAO, we dont have to set core number */
	i4Freq = (u4TarPerfLevel >= u4BoostCpuTh) ? MAX_CPU_FREQ : -1;
	for (i = 0; i < u4ClusterNum; i++) {
		freq_to_set[i].min = i4Freq;
		freq_to_set[i].max = i4Freq;
	}

	update_userlimit_cpu_freq(CPU_KIR_WIFI, u4ClusterNum, freq_to_set);

	return 0;
}

#ifdef CONFIG_MTK_EMI
void kalSetEmiMpuProtection(phys_addr_t emiPhyBase, bool enable)
{
	struct emi_region_info_t region_info;

	DBGLOG(INIT, INFO, "emiPhyBase: 0x%x, enable: %d\n",
			emiPhyBase, enable);

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

	DBGLOG(INIT, INFO, "emiPhyBase: 0x%x, offset: %u, size: %u\n",
			emiPhyBase, offset, size);

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

int32_t kalGetFwFlavor(uint8_t *flavor)
{
	int32_t ret = 1;
	const uint32_t adie_chip_id = mtk_wcn_wmt_ic_info_get(WMTCHIN_ADIE);

	DBGLOG(INIT, INFO, "adie_chip_id: 0x%x\n", adie_chip_id);

	switch (adie_chip_id) {
	case 0x6631:
		*flavor = 'a';
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}
