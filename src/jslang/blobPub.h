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

#ifndef _BLOBAPI_H_
#define _BLOBAPI_H_

#include "../common/jsHelper.h"


inline JSClass* BlobJSClass( JSContext *cx ) {

	static JSClass *jsClass = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if ( jsClass == NULL )
		jsClass = GetGlobalClassByName(cx, "Blob");
	return jsClass;
}


inline bool JsvalIsBlob( JSContext *cx, jsval val ) {

	return ( JSVAL_IS_OBJECT(val) && !JSVAL_IS_NULL(val) && JS_GET_CLASS(cx, JSVAL_TO_OBJECT(val)) == BlobJSClass(cx) );
}

/*

// NewBlob takes ownership of jsMallocatedBuffer on success. Allocation must be done with JS_malloc
inline JSObject* NewBlob( JSContext *cx, void *jsMallocatedBuffer, size_t bufferLength ) {

	JSClass *blobClass = BlobJSClass(cx);
	if ( blobClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, blobClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	if ( JS_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL( bufferLength )) != JS_TRUE )
		return NULL;
	if ( JS_SetPrivate(cx, obj, jsMallocatedBuffer) != JS_TRUE )
		return NULL;
	return obj;
}

inline JSBool NewBlobCopyN(JSContext *cx, const void *data, size_t amount, JSObject **bstrObj) {

	char *bstrBuf = (char*)JS_malloc(cx, amount);
	J_S_ASSERT_ALLOC( bstrBuf );
	memcpy( bstrBuf, data, amount );
	*bstrObj = NewBlob(cx, bstrBuf, amount);
	J_S_ASSERT( *bstrObj != NULL, "Unable to create a Blob." );
	return JS_TRUE;
}

inline JSObject* NewEmptyBlob( JSContext *cx ) {

	JSClass *blobClass = BlobJSClass(cx);
	if ( blobClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, blobClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	return obj;
}



inline JSBool BlobLength( JSContext *cx, JSObject *bStringObject, size_t *length ) {

	J_S_ASSERT_CLASS(bStringObject, BlobJSClass( cx ));
	jsval lengthVal;
	J_CHK( JS_GetReservedSlot(cx, bStringObject, SLOT_BLOB_LENGTH, &lengthVal) );
	*length = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	return JS_TRUE;
}


inline JSBool BlobBuffer( JSContext *cx, JSObject *bStringObject, const void **buffer ) {

	J_S_ASSERT_CLASS(bStringObject, BlobJSClass( cx ));
	*buffer = JS_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}


inline JSBool BlobGetBufferAndLength( JSContext *cx, JSObject *bStringObject, void **data, size_t *dataLength ) {

	J_S_ASSERT_CLASS(bStringObject, BlobJSClass( cx ));
	jsval lengthVal;
	J_CHK( JS_GetReservedSlot(cx, bStringObject, SLOT_BLOB_LENGTH, &lengthVal) );
	*dataLength = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	*data = JS_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}

*/


#endif
