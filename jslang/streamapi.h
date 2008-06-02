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

#include <jsobj.h>


typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buf, unsigned int *amount );


static jsid streamReadPropId = 0; // (TBD) try to make this higher than module-static

inline jsid GetStreamReadPropId(JSContext *cx) {

	if ( streamReadPropId == 0 )
		if ( JS_ValueToId(cx, STRING_TO_JSVAL(JS_InternString(cx, "_Read")), &streamReadPropId) != JS_TRUE )
			return 0;
	return streamReadPropId;
}

inline JSBool HasStreamReadInterface( JSContext *cx, JSObject *obj, bool *found ) {

	jsval tmp;
	J_CHK( OBJ_GET_PROPERTY(cx, obj, GetStreamReadPropId(cx), &tmp) );
	*found = ( tmp != JSVAL_VOID );
	return JS_TRUE;
}

inline JSBool SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead *ppFct ) {
	
	jsval tmp;
	if ( ppFct != NULL ) {

		// (TBD) assert ppFct is well aligned
		J_S_ASSERT( sizeof(ppFct) <= sizeof(jsval), "Unable to store the function pointer." );
		tmp = PRIVATE_TO_JSVAL(ppFct);
		return OBJ_SET_PROPERTY(cx, obj, GetStreamReadPropId(cx), &tmp ); // JSPROP_READONLY | JSPROP_PERMANENT);
	} else {
		
		return OBJ_DELETE_PROPERTY(cx, obj, GetStreamReadPropId(cx), &tmp);
	}
}

// read data from a stream object that implements NIStreamRead or a javascript function Read( amount ) if the NIStreamRead is not available.
inline JSBool StreamReadInterface(JSContext *cx, JSObject *obj, char *buffer, size_t *amount ) {

	jsval niVal;
	J_CHKM( OBJ_GET_PROPERTY(cx, obj, GetStreamReadPropId(cx), &niVal), "Unable to get the native interface." );

	if ( niVal != JSVAL_VOID ) {
		
		void *ppFct = JSVAL_TO_PRIVATE(niVal);
		J_S_ASSERT( ppFct != NULL, "Invalid native interface." );
		NIStreamRead fct = *(NIStreamRead*)ppFct;
		J_S_ASSERT( fct != NULL, "Invalid native interface." );
		J_CHK( fct(cx, obj, buffer, amount) );
	} else {
		
		jsval tmpVal, rval;
		IntToJsval(cx, *amount, &tmpVal);
		J_CHKM( JS_CallFunctionName(cx, obj, "Read", 1, &tmpVal, &rval), "Read() function not found." );
		const char *tmpBuf;
		size_t size;
		J_CHK( JsvalToStringAndLength(cx, rval, &tmpBuf, &size) );
		*amount = J_MIN(size, *amount);
		memcpy(buffer, tmpBuf, *amount);
	}

	return JS_TRUE;
}



#endif // _STREAMAPI_H_