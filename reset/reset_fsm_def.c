/**********************************************************************
 *
 * This file is provided under a dual license.  When you use or
 * distribute this software, you may choose to be licensed under
 * version 2 of the GNU General Public License ("GPLv2 License")
 * or BSD License.
 *
 * GPLv2 License
 *
 * Copyright(C) 2017 MediaTek Inc.
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
 * Copyright(C) 2017 MediaTek Inc. All rights reserved.
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
 **********************************************************************/
/*! \file   reset_fsm_def.c
*   \brief  reset fsm def
*
*    This file contains all implementations of reset module
*/


/**********************************************************************
*                         C O M P I L E R   F L A G S
***********************************************************************
*/


/**********************************************************************
*                    E X T E R N A L   R E F E R E N C E S
***********************************************************************
*/
#include "reset_fsm_def.h"
#include "reset.h"

/**********************************************************************
*                                 M A C R O S
***********************************************************************
*/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif

/**********************************************************************
*                              C O N S T A N T S
***********************************************************************
*/


/**********************************************************************
*                             D A T A   T Y P E S
***********************************************************************
*/


/**********************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
***********************************************************************
*/
static void enter_INIT(struct FsmEntity *fsm, struct FsmState *prev);

static void enter_L0_PRE_RESET(struct FsmEntity *fsm, struct FsmState *prev);
static bool gaurd_L0_PRE_RESET_L0_RESET_READY(struct FsmEntity *fsm);
static void leave_L0_PRE_RESET(struct FsmEntity *fsm, struct FsmState *next);

static void enter_L0_RESET_GOING(struct FsmEntity *fsm, struct FsmState *prev);


/**********************************************************************
*                            P U B L I C   D A T A
***********************************************************************
*/


/**********************************************************************
*                           P R I V A T E   D A T A
***********************************************************************
*/
static struct FsmState RFSM_INIT;
static struct FsmState RFSM_WAIT_PROBE;
static struct FsmState RFSM_PROBED;
static struct FsmState RFSM_L0_PRE_RESET;
static struct FsmState RFSM_L0_RESET_GOING;

static struct ResetFsmEventAction RFSM_EVENT_ACTION_LIST_INIT[] = {
	{RFSM_EVENT_PROBE_START, NULL, NULL, &RFSM_WAIT_PROBE},
	{RFSM_EVENT_TRIGGER_RESET, NULL, NULL, &RFSM_L0_PRE_RESET}
};

static struct ResetFsmEventAction RFSM_EVENT_ACTION_LIST_WAIT_PROBE[] = {
	{RFSM_EVENT_PROBE_FAIL, NULL, NULL, &RFSM_INIT},
	{RFSM_EVENT_PROBE_SUCCESS, NULL, NULL, &RFSM_PROBED},
	{RFSM_EVENT_TRIGGER_RESET, NULL, NULL, &RFSM_L0_PRE_RESET},
	{RFSM_EVENT_L0_RESET_DONE, NULL, NULL, &RFSM_INIT}
};

static struct ResetFsmEventAction RFSM_EVENT_ACTION_LIST_PROBED[] = {
	{RFSM_EVENT_REMOVE, NULL, NULL, &RFSM_INIT},
	{RFSM_EVENT_TRIGGER_RESET, NULL, NULL, &RFSM_L0_PRE_RESET},
	{RFSM_EVENT_L0_RESET_DONE, NULL, NULL, &RFSM_INIT}
};

static struct ResetFsmEventAction RFSM_EVENT_ACTION_LIST_L0_PRE_RESET[] = {
	{RFSM_EVENT_TIMEOUT, NULL, NULL, &RFSM_L0_RESET_GOING},
	{RFSM_EVENT_L0_RESET_READY, gaurd_L0_PRE_RESET_L0_RESET_READY, NULL,
		&RFSM_L0_RESET_GOING},
	{RFSM_EVENT_L0_RESET_GOING, NULL, NULL, &RFSM_L0_RESET_GOING},
	{RFSM_EVENT_TIMEOUT, NULL, NULL, &RFSM_L0_RESET_GOING},
	{RFSM_EVENT_L0_RESET_DONE, NULL, NULL, &RFSM_INIT}
};

static struct ResetFsmEventAction RFSM_EVENT_ACTION_LIST_L0_RESET_GOING[] = {
	{RFSM_EVENT_L0_RESET_DONE, NULL, NULL, &RFSM_INIT},
};

static struct FsmState RFSM_INIT = {
	.name = "INIT",
	.enter_func = enter_INIT,
	.leave_func = NULL,
	.eventActionListCount = ARRAY_SIZE(RFSM_EVENT_ACTION_LIST_INIT),
	.eventActionList = RFSM_EVENT_ACTION_LIST_INIT
};
static struct FsmState RFSM_WAIT_PROBE = {
	.name = "WAIT_PROBE",
	.enter_func = NULL,
	.leave_func = NULL,
	.eventActionListCount = ARRAY_SIZE(RFSM_EVENT_ACTION_LIST_WAIT_PROBE),
	.eventActionList = RFSM_EVENT_ACTION_LIST_WAIT_PROBE
};
static struct FsmState RFSM_PROBED = {
	.name = "PROBED",
	.enter_func = NULL,
	.leave_func = NULL,
	.eventActionListCount = ARRAY_SIZE(RFSM_EVENT_ACTION_LIST_PROBED),
	.eventActionList = RFSM_EVENT_ACTION_LIST_PROBED
};
static struct FsmState RFSM_L0_PRE_RESET = {
	.name = "L0_PRE_RESET",
	.enter_func = enter_L0_PRE_RESET,
	.leave_func = leave_L0_PRE_RESET,
	.eventActionListCount =
		ARRAY_SIZE(RFSM_EVENT_ACTION_LIST_L0_PRE_RESET),
	.eventActionList = RFSM_EVENT_ACTION_LIST_L0_PRE_RESET
};
static struct FsmState RFSM_L0_RESET_GOING = {
	.name = "L0_RESET_GOING",
	.enter_func = enter_L0_RESET_GOING,
	.leave_func = NULL,
	.eventActionListCount =
		ARRAY_SIZE(RFSM_EVENT_ACTION_LIST_L0_RESET_GOING),
	.eventActionList = RFSM_EVENT_ACTION_LIST_L0_RESET_GOING
};


/**********************************************************************
*                              F U N C T I O N S
**********************************************************************/
static void enter_INIT(struct FsmEntity *fsm, struct FsmState *prev)
{
	MR_Info("[%s] %s\n", fsm->name, __func__);
	clearAllModuleReadyForReset();

	if (prev == &RFSM_L0_RESET_GOING)
		resetkoNotifyEvent(fsm, MODULE_NOTIFY_RESET_DONE);
}

static void enter_L0_PRE_RESET(struct FsmEntity *fsm, struct FsmState *prev)
{
	MR_Info("[%s] %s\n", fsm->name, __func__);
	resetkoStartTimer(fsm, WAIT_ALL_MODULE_READY_TIMEOUT);
	fsm->fgReadyForReset = false;
	resetkoNotifyEvent(fsm, MODULE_NOTIFY_PRE_RESET);
}

static bool gaurd_L0_PRE_RESET_L0_RESET_READY(struct FsmEntity *fsm)
{
	bool ret = isAllModuleReadyForReset();

	MR_Info("[%s] %s %s\n", fsm->name, __func__, ret ? "pass" : "fail");

	return ret;
}

static void leave_L0_PRE_RESET(struct FsmEntity *fsm, struct FsmState *next)
{
	MR_Info("[%s] %s\n", fsm->name, __func__);
	resetkoCancleTimer(fsm);
}

static void enter_L0_RESET_GOING(struct FsmEntity *fsm, struct FsmState *prev)
{
	MR_Info("[%s] %s\n", fsm->name, __func__);
	send_reset_event(fsm->eModuleType, RFSM_EVENT_L0_RESET_GOING);
	resetkoNotifyEvent(fsm, MODULE_NOTIFY_RESET_GOING);
	callResetFuncByResetApiType(fsm);
}

struct FsmEntity *allocResetFsm(char *name,
				enum ModuleType eModuleType,
				enum TriggerResetApiType resetApiType)
{
	struct FsmEntity *fsm = allocFsmEntity(name, eModuleType, resetApiType);

	if (!fsm)
		return NULL;

	fsm->fsmState = &RFSM_INIT;

	return fsm;
}

void freeResetFsm(struct FsmEntity *fsm)
{
	if (fsm)
		freeFsmEntity(fsm);
}

void resetFsmHandlevent(struct FsmEntity *fsm, enum ResetFsmEvent event)
{
	if ((fsm != NULL) && ((unsigned int)event < RFSM_EVENT_MAX))
		RFSM_handle_event(fsm, (unsigned int)event);
}


