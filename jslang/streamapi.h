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

#ifndef _STREAMAPI_H_
#define _STREAMAPI_H_


#include "../common/jsNativeInterface.h"


//	jsid idRead;
//	JS_ValueToId(cx, STRING_TO_JSVAL(JS_InternString(cx, "_NISR")), &idRead);

// read data from a stream object that implements NIStreamRead or a javascript function Read( amount )
inline JSBool JSStreamRead(JSContext *cx, JSObject *obj, char *buffer, unsigned int *amount ) {

	NIStreamRead fct;
	J_CHECK_CALL( GetStreamReadInterface(cx, obj, &fct) );
	if ( fct != NULL ) {

		J_CHECK_CALL( fct(cx, obj, buffer, amount) );
	} else {
		
		jsval tmpVal, rval;
		IntToJsval(cx, *amount, &tmpVal);
		if ( JS_CallFunctionName(cx, obj, "Read", 1, &tmpVal, &rval) != JS_TRUE )
			REPORT_ERROR( "The object do not have a Read() function." );
		const char *tmpBuf;
		size_t size;
		J_CHECK_CALL( JsvalToStringAndLength(cx, rval, &tmpBuf, &size) );
		memcpy(buffer, tmpBuf, size);
	}

	return JS_TRUE;
}



#endif // _STREAMAPI_H_