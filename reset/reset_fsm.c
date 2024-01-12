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
/*! \file   reset_fsm.c
*   \brief  reset fsm
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
#include <linux/string.h>
#include <linux/slab.h>
#include "reset.h"

/**********************************************************************
*                                 M A C R O S
***********************************************************************
*/


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


/**********************************************************************
*                            P U B L I C   D A T A
***********************************************************************
*/


/**********************************************************************
*                           P R I V A T E   D A T A
***********************************************************************
*/


/**********************************************************************
*                              F U N C T I O N S
**********************************************************************/
struct FsmEntity *allocFsmEntity(char *name,
				enum ModuleType eModuleType,
				enum TriggerResetApiType resetApiType)
{
	struct FsmEntity *fsm;

	if ((!name) ||
	    ((unsigned int)eModuleType >= RESET_MODULE_TYPE_MAX) ||
	    ((unsigned int)resetApiType >= TRIGGER_RESET_API_TYPE_MAX))
		return NULL;

	fsm = kmalloc(sizeof(struct FsmEntity), GFP_KERNEL);
	if (!fsm)
		return NULL;
	fsm->name = kmalloc(RFSM_NAME_MAX_LEN, GFP_KERNEL);
	if (fsm->name == NULL) {
		kfree(fsm);
		return NULL;
	}
	strncpy(fsm->name, name, RFSM_NAME_MAX_LEN);
	fsm->eModuleType = eModuleType;
	fsm->resetApiType = resetApiType;
	fsm->fgReadyForReset = false;
	fsm->resetFunc = NULL;
	fsm->notifyFunc = NULL;

	return fsm;
}

void freeFsmEntity(struct FsmEntity *fsm)
{
	if (!fsm)
		return;

	if (fsm->name != NULL) {
		memset(fsm->name, 0, RFSM_NAME_MAX_LEN);
		kfree(fsm->name);
	}
	memset(fsm, 0, sizeof(struct FsmEntity));
	kfree(fsm);
}

void RFSM_handle_event(struct FsmEntity *fsm, unsigned int event)
{
	struct FsmState *currentFsmState;
	struct FsmState *nextFsmState;
	struct ResetFsmEventAction *currentEventAction;
	int i;

	if ((fsm == NULL) || (fsm->name == NULL) || (fsm->fsmState == NULL)) {
		MR_Err("fsm args error\n");
		return;
	}

	currentFsmState = fsm->fsmState;
	if ((currentFsmState->name == NULL) ||
	    (currentFsmState->eventActionList == 0) ||
	    (currentFsmState->eventActionList == NULL)) {
		MR_Err("[%s] RFSM ignore event [%d]\n", fsm->name, event);
		return;
	}

	for (i = 0; i < currentFsmState->eventActionListCount; i++) {
		currentEventAction = &currentFsmState->eventActionList[i];
		if (event == currentEventAction->event) {
			if (currentEventAction->guard_func != NULL) {
				if (!currentEventAction->guard_func(fsm))
					continue;
			}
			if (currentEventAction->action_func != NULL)
				currentEventAction->action_func(fsm);

			nextFsmState = currentEventAction->nextState;
			if (nextFsmState != NULL &&
			    currentFsmState != nextFsmState) {
				if (currentFsmState->leave_func != NULL)
					currentFsmState->leave_func(fsm,
								nextFsmState);
				MR_Info("[%s] state: [%s] -> [%s]",
					fsm->name,
					currentFsmState->name,
					nextFsmState->name);
				fsm->fsmState = nextFsmState;
				if (nextFsmState->enter_func != NULL)
					nextFsmState->enter_func(fsm,
							currentFsmState);
			}

			/* one event can only trigger one action */
			break;
		}
	}
}
