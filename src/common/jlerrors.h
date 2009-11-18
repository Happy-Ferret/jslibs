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


#ifndef _JSERRORS_H_
#define _JSERRORS_H_

#include "jlconfiguration.h"

typedef enum J_ErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "jlerrors.msg"
#undef MSG_DEF
    J_ErrLimit
#undef MSGDEF
} J_ErrNum;


// usage: JS_ReportErrorNumber(cx, my_GetErrorMessage, NULL, JSSMSG_OUT_OF_RANGE);
inline JSBool JL_ReportError( JSContext *cx, J_ErrNum name ) {

	jsval tmp;
	JL_CHK( GetConfigurationValue(cx, JLID_NAME(_getErrorMessage), &tmp) );
	JSErrorCallback errorCallback;
	errorCallback = NULL;
	if ( !JSVAL_IS_VOID( tmp ) )
		errorCallback = *(JSErrorCallback*)JSVAL_TO_PRIVATE(tmp);
	else
		JS_ReportError(cx, "Failed to get the ErrorCallback.");
	JS_ReportErrorNumber(cx, errorCallback, NULL, name);
	JL_BAD;
}

#endif // _JSERRORS_H_
