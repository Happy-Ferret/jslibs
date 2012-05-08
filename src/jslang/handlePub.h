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

#include "stdafx.h"

#define JL_HANDLE_PUBLIC_SLOT_COUNT 4

#define JLHID(id) \
	jl::CastCStrToUint32(#id)

typedef void (*HandleFinalizeCallback_t)(void* data);

#define JL_HANDLE_TYPE uint32_t
#define JL_HANDLE_INVALID (0)


// (TBD) better alignment: __attribute__ ((__vector_size__ (16), __may_alias__)); OR  __declspec(align(64))
//       note that SSE data must be 128bits alligned !

struct HandlePrivate {
	JL_HANDLE_TYPE handleType;
	HandleFinalizeCallback_t finalizeCallback;
};


ALWAYS_INLINE JSClass*
JL_HandleJSClass( JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClass(JL_GetHostPrivate(cx), "Handle");
	return clasp;
}


template <class Struct>
INLINE JSBool FASTCALL
HandleCreate( JSContext *cx, const JL_HANDLE_TYPE handleType, Struct **userStruct, HandleFinalizeCallback_t finalizeCallback, jsval *handleVal ) {

	ASSERT( handleType != JL_HANDLE_INVALID );

	const ClassProtoCache *classProtoCache = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Handle");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME("Handle"), E_NOTFOUND );

	JSObject *handleObj;
	handleObj = JL_NewObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto, NULL);
	JL_CHK( handleObj );
	*handleVal = OBJECT_TO_JSVAL(handleObj);

	HandlePrivate *pv;
	pv = (HandlePrivate*)jl_malloc(sizeof(HandlePrivate) + sizeof(Struct));
	JL_ASSERT_ALLOC( pv );
	JL_updateMallocCounter(cx, sizeof(HandlePrivate) + sizeof(Struct));
	JL_SetPrivate( handleObj, pv);

	pv->handleType = handleType;
	pv->finalizeCallback = finalizeCallback;
	*userStruct = (Struct*)(pv+1);

	return JS_TRUE;
	JL_BAD;
}


INLINE JSBool
HandleClose( JSContext *cx, jsval handleVal ) { // see finalize
	
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(handleObj);
	JL_ASSERT_OBJECT_STATE(pv, "Handle");

	if ( pv->finalizeCallback )
		pv->finalizeCallback(pv+1);
	jl_free(pv);
	JL_SetPrivate( handleObj, NULL);

	return JS_TRUE;
	JL_BAD;
}


INLINE JL_HANDLE_TYPE
GetHandleType( JSContext *cx, jsval handleVal ) {

	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	
	pv = (HandlePrivate*)JL_GetPrivate(handleObj);
	JL_CHK( pv != NULL );
	return pv->handleType;
bad:
	return JL_HANDLE_INVALID;
}


ALWAYS_INLINE bool
IsHandle( JSContext *cx, jsval handleVal ) {

	return JL_ValueIsClass(handleVal, JL_HandleJSClass(cx));
}

ALWAYS_INLINE bool
IsHandleType( JSContext *, JSObject *handleObj, JL_HANDLE_TYPE handleType ) {

	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(handleObj);
	return pv != NULL && pv->handleType == handleType;
}


ALWAYS_INLINE bool
IsHandleType( JSContext *cx, jsval handleVal, JL_HANDLE_TYPE handleType ) {

	if ( !JL_ValueIsClass(handleVal, JL_HandleJSClass(cx)) )
		return false;
	JSObject *handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(handleObj);
	return pv != NULL && pv->handleType == handleType;
}


INLINE void*
GetHandlePrivate( JSContext *cx, const jsval &handleVal ) {

	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(JSVAL_TO_OBJECT(handleVal));
	JL_CHK( pv != NULL );
	return (char*)pv + sizeof(HandlePrivate); // user data is just behind our private structure.
bad:
	return NULL;
}


INLINE JSBool
SetHandleSlot( JSContext *cx, jsval handleVal, uint32_t slotIndex, jsval value ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );
	return JL_SetReservedSlot( JSVAL_TO_OBJECT(handleVal), slotIndex, value);
	JL_BAD;
}


INLINE JSBool
GetHandleSlot( JSContext *cx, jsval handleVal, uint32_t slotIndex, jsval *value ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	JL_ASSERT_INSTANCE( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );
	return JL_GetReservedSlot( JSVAL_TO_OBJECT(handleVal), slotIndex, value);
	JL_BAD;
}


/* Usage:
	void FinalizeTrimesh(void *data) {
	}
	...
	void *data;
	JL_CHK( CreateId(cx, 'TEST', 10, &data, FinalizeTrimesh, JL_RVAL) );
	bool c = IsIdType(cx, *JL_RVAL, 'TEST');
	bool d = (data == GetHandlePrivate(cx, *JL_RVAL));
*/
