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

#ifndef _AP_SELECTION_H
#define _AP_SELECTION_H

struct weight_config {
	uint8_t chnl_util_weight;
	uint8_t snr_weight;
	uint8_t rssi_weight;
	uint8_t scan_miss_cnt_weight;
	uint8_t probe_resp_weight;
	uint8_t client_cnt_weight;
	uint8_t ap_num_weight;
	uint8_t band_weight;
	uint8_t band_width_weight;
	uint8_t stbc_weight;
	uint8_t last_deauth_weight;
	uint8_t black_list_weight;
	uint8_t saa_weight;
	uint8_t chnl_idle_weight;
	uint8_t opchnl_weight;
};

/* Support AP Selection */
struct BSS_DESC *scanSearchBssDescByScoreForAis(struct ADAPTER *prAdapter);
void scanGetCurrentEssChnlList(struct ADAPTER *prAdapter);
/* end Support AP Selection */

#endif
