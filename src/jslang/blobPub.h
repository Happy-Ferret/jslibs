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

#include <jlhelper.h>

/* the new Blob
namespace jl {
struct Blob : CppAllocators {

	size_t _size;
	uint8_t *_data;

	~Blob() {

		jl_free(_data);
	}

	Blob() {

		_data = NULL;
	}

	ALWAYS_INLINE void SetSize( size_t size ) {

		_data = static_cast<uint8_t*>(jl_realloc(_data, size));
	}
};
}
*/


ALWAYS_INLINE JSClass* JL_BlobJSClass( const JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Blob")->clasp;
	return clasp;
}


ALWAYS_INLINE bool JL_JsvalIsBlob( const JSContext *cx, const jsval &val ) {

	return JL_IsClass(val, JL_BlobJSClass(cx) );
}


/*

// NewBlob takes ownership of jsMallocatedBuffer on success. Allocation must be done with JS_malloc
inline JSObject* NewBlob( JSContext *cx, void *jsMallocatedBuffer, size_t bufferLength ) {

	JSClass *blobClass = JL_BlobJSClass(cx);
	if ( blobClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, blobClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	if ( JL_SetReservedSlot(cx, obj, SLOT_BLOB_LENGTH, INT_TO_JSVAL( bufferLength )) != JS_TRUE )
		return NULL;
	if ( JL_SetPrivate(cx, obj, jsMallocatedBuffer) != JS_TRUE )
		return NULL;
	return obj;
}

inline JSBool NewBlobCopyN(JSContext *cx, const void *data, size_t amount, JSObject **bstrObj) {

	char *bstrBuf = (char*)JS_malloc(cx, amount);
	JL_CHK( bstrBuf );
	memcpy( bstrBuf, data, amount );
	*bstrObj = NewBlob(cx, bstrBuf, amount);
	JL_ASSERT( *bstrObj != NULL, "Unable to create a Blob." );
	return JS_TRUE;
}

inline JSObject* NewEmptyBlob( JSContext *cx ) {

	JSClass *blobClass = JL_BlobJSClass(cx);
	if ( blobClass == NULL )
		return NULL;
	JSObject *obj = JS_NewObject(cx, blobClass, NULL, NULL);
	if ( obj == NULL )
		return NULL;
	return obj;
}



inline JSBool BlobLength( JSContext *cx, JSObject *bStringObject, size_t *length ) {

	JL_ASSERT_INSTANCE(bStringObject, JL_BlobJSClass( cx ));
	jsval lengthVal;
	JL_CHK( JL_GetReservedSlot(cx, bStringObject, SLOT_BLOB_LENGTH, &lengthVal) );
	*length = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	return JS_TRUE;
}


inline JSBool BlobBuffer( JSContext *cx, JSObject *bStringObject, const void **buffer ) {

	JL_ASSERT_INSTANCE(bStringObject, JL_BlobJSClass( cx ));
	*buffer = JL_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}


inline JSBool BlobGetBufferAndLength( JSContext *cx, JSObject *bStringObject, void **data, size_t *dataLength ) {

	JL_ASSERT_INSTANCE(bStringObject, JL_BlobJSClass( cx ));
	jsval lengthVal;
	JL_CHK( JL_GetReservedSlot(cx, bStringObject, SLOT_BLOB_LENGTH, &lengthVal) );
	*dataLength = JSVAL_IS_INT(lengthVal) ? JSVAL_TO_INT( lengthVal ) : 0;
	*data = JL_GetPrivate(cx, bStringObject);
	return JS_TRUE;
}

*/
