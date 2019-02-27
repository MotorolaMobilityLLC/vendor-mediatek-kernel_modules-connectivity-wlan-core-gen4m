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

#include <legacy_controller.h>
#include <linux/pm_qos.h>

#include "precomp.h"
#include <helio-dvfsrc-opp.h>

#define MAX_CPU_FREQ (3 * 1024 * 1024) /* in kHZ */
#define CLUSTER_NUM  2       /* 2 clusters */

int32_t kalBoostCpu(IN struct ADAPTER *prAdapter,
		    IN uint32_t u4TarPerfLevel,
		    IN uint32_t u4BoostCpuTh)
{
	struct ppm_limit_data freq_to_set[CLUSTER_NUM];
	int32_t i = 0, i4Freq = -1;
	static struct pm_qos_request wifi_qos_request;
	static u_int8_t fgRequested;

	/* ACAO, we dont have to set core number */
	i4Freq = (u4TarPerfLevel >= u4BoostCpuTh) ? MAX_CPU_FREQ : -1;
	for (i = 0; i < CLUSTER_NUM; i++)
		freq_to_set[i].min = i4Freq;

	update_userlimit_cpu_freq(PPM_KIR_WIFI, CLUSTER_NUM, freq_to_set);

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
	return 0;
}
