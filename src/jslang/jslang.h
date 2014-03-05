/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


#pragma once

#define PROCESSEVENT_MAX_ITEM (sizeof(uint32_t) * 8 -1) // see eventsMask. -1 for the sign

struct ProcessEventThreadInfo {

	/*volatile*/ ProcessEvent2 *peSlot;
	volatile bool isEnd;
	JLSemaphoreHandler startSem;
	JLSemaphoreHandler signalEventSem;
	JLThreadHandler thread;
};

struct ModulePrivate {
	ProcessEventThreadInfo processEventThreadInfo[PROCESSEVENT_MAX_ITEM];
	JLSemaphoreHandler processEventSignalEventSem;
};


bool jslangModuleInit(JSContext *cx, JS::HandleObject obj);
bool jslangModuleRelease(JSContext *cx);
void jslangModuleFree(bool skipCleanup);


//static const uint32_t jslangModuleId = jl::CastCStrToUint32("lang");

#define jslangModuleId (reinterpret_cast<moduleId_t>(jslangModuleInit))

