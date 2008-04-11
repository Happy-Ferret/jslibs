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

#ifndef _BSTRING_H_
#define _BSTRING_H_

#include "../common/jsHelper.h"

#define SLOT_BSTRING_LENGTH 0


inline JSClass* BStringJSClass( JSContext *cx ) {

	static JSClass *jsClass = NULL;
	if ( jsClass == NULL )
		jsClass = GetClassByName(cx, "BString");
	return jsClass;
}


// NewBString takes ownership of jsMallocatedBuffer on success. Allocation must be done with JS_malloc
inline JSObject* NewBString( JSContext *cx, char *jsMallocatedBuffer, size_t bufferLength ) {
		
	JSClass *bstringClass = BStringJSClass(cx);
	if ( bstringClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, BStringJSClass(cx), NULL, NULL);
	if ( obj == NULL )
		return NULL;
	JS_SetReservedSlot(cx, obj, SLOT_BSTRING_LENGTH, INT_TO_JSVAL( bufferLength ));
	JS_SetPrivate(cx, obj, jsMallocatedBuffer);
	return obj;
}


inline JSObject* EmptyBString( JSContext *cx ) {

	JSClass *bstringClass = BStringJSClass(cx);
	if ( bstringClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, bstringClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	return obj;
}


inline size_t BStringLength( JSContext *cx, JSObject *bStringObject ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return 0;
	);
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	return JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
}


inline char* BStringData( JSContext *cx, JSObject *bStringObject ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return NULL;
	);
	return (char*)JS_GetPrivate(cx, bStringObject);
}


inline JSBool BStringGetDataAndLength( JSContext *cx, JSObject *bStringObject, char **data, size_t *dataLength ) {

	RT_SAFE(
		if ( JS_GET_CLASS( cx, bStringObject ) != BStringJSClass( cx ) )
			return JS_FALSE;
	);
	jsval lengthVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, bStringObject, SLOT_BSTRING_LENGTH, &lengthVal) );
	*dataLength = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	*data = (char*)JS_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}


#endif
