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

//static const uint32_t moduleId = JL_CAST_CSTR_TO_UINT32("io"); // replaced by _moduleId

struct JsioPrivate {

	PRFileDesc *peCancel;
};


#define SLOT_JSIO_DIR_NAME 0

#define SLOT_JSIO_DESCRIPTOR_IMPORTED 0
#define SLOT_JSIO_DESCRIPTOR_TIMEOUT 1

#define SLOT_JSIO_FILE_NAME 2


void FinalizeDescriptor(JSContext *cx, JSObject *obj);

JSBool NativeInterfaceStreamRead(JSContext *cx, JSObject *obj, char *buf, size_t *amount);



ALWAYS_INLINE JSBool
GetTimeoutInterval(JSContext *cx, JSObject *obj, PRIntervalTime *timeout) {

	jsval timeoutValue;
	JL_CHK( JS_GetReservedSlot(cx, obj, SLOT_JSIO_DESCRIPTOR_TIMEOUT, &timeoutValue) );
	*timeout = JSVAL_IS_VOID(timeoutValue) ? PR_INTERVAL_NO_TIMEOUT : PR_MillisecondsToInterval(JSVAL_TO_INT(timeoutValue));
	return JS_TRUE;
	JL_BAD;
}

