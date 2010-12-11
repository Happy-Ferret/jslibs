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

#ifndef _HANDLEAPI_H_
#define _HANDLEAPI_H_

#include "stdafx.h"

#define HANDLE_PUBLIC_SLOT_COUNT 4

#define JLHID(id) JL_CAST_CSTR_TO_UINT32(#id)

typedef void (*HandleFinalizeCallback_t)(void* data);

#define HANDLE_TYPE uint32_t
#define HANDLE_INVALID 0

// (TBD) better alignment: __attribute__ ((__vector_size__ (16), __may_alias__)); OR  __declspec(align(64))
//       note that SSE data must be 128bits alligned !

struct HandlePrivate {
	HANDLE_TYPE handleType;
	HandleFinalizeCallback_t finalizeCallback;
};


static ALWAYS_INLINE JSClass*
JL_HandleJSClass( JSContext *cx ) {

	static JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if (unlikely( clasp == NULL ))
		clasp = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Handle")->clasp;
	return clasp;
}


static INLINE JSBool
HandleCreate( JSContext *cx, HANDLE_TYPE handleType, size_t userDataSize, void** userData, HandleFinalizeCallback_t finalizeCallback, jsval *handleVal ) {

	JL_ASSERT( handleType != HANDLE_INVALID );

	const ClassProtoCache *classProtoCache = JL_GetCachedClassProto(JL_GetHostPrivate(cx), "Handle");
	JL_S_ASSERT( classProtoCache->clasp != NULL, "Handle class not initialized.");

	JSObject *handleObj;
	handleObj = JS_NewObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto, NULL);
	JL_CHK( handleObj );
	*handleVal = OBJECT_TO_JSVAL(handleObj);
	HandlePrivate *pv;
	pv = (HandlePrivate*)jl_malloc(sizeof(HandlePrivate) + userDataSize);
	JL_S_ASSERT_ALLOC( pv );
	JL_SetPrivate(cx, handleObj, pv);

	pv->handleType = handleType;
	pv->finalizeCallback = finalizeCallback;
	if (userData)
		*userData = (char*)pv + sizeof(HandlePrivate);

	return JS_TRUE;
	JL_BAD;
}


static INLINE JSBool
HandleClose( JSContext *cx, jsval handleVal ) { // see finalize
	
	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	JL_S_ASSERT_RESOURCE( pv );
	if ( pv->finalizeCallback )
		pv->finalizeCallback((char*)pv + sizeof(HandlePrivate));
	jl_free(pv);
	JL_SetPrivate(cx, handleObj, NULL);

	return JS_TRUE;
	JL_BAD;
}


static INLINE HANDLE_TYPE
GetHandleType( JSContext *cx, jsval handleVal ) {

	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	JL_CHK( pv != NULL );
	return pv->handleType;
bad:
	return HANDLE_INVALID;
}


static ALWAYS_INLINE bool
IsHandleType( JSContext *cx, jsval handleVal, HANDLE_TYPE handleType ) {

	if ( !JL_JsvalIsClass(handleVal, JL_HandleJSClass(cx)) )
		return false;
	JSObject *handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	return pv != NULL && pv->handleType == handleType;
}


static INLINE void*
GetHandlePrivate( JSContext *cx, const jsval &handleVal ) {

	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );

	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(handleVal));
	JL_CHK( pv != NULL );
	return (char*)pv + sizeof(HandlePrivate); // user data is just behind our private structure.
bad:
	return NULL;
}


static INLINE JSBool
SetHandleSlot( JSContext *cx, jsval handleVal, uint32 slotIndex, jsval value ) {

	JL_ASSERT( slotIndex < HANDLE_PUBLIC_SLOT_COUNT );
	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );
	return JL_SetReservedSlot(cx, JSVAL_TO_OBJECT(handleVal), slotIndex, value);
	JL_BAD;
}


static INLINE JSBool
GetHandleSlot( JSContext *cx, jsval handleVal, uint32 slotIndex, jsval *value ) {

	JL_ASSERT( slotIndex < HANDLE_PUBLIC_SLOT_COUNT );
	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT(handleVal), JL_HandleJSClass(cx) );
	return JL_GetReservedSlot(cx, JSVAL_TO_OBJECT(handleVal), slotIndex, value);
	JL_BAD;
}


/* Usage

void FinalizeTrimesh(void *data) {
}

...

void *data;
JL_CHK( CreateId(cx, 'TEST', 10, &data, FinalizeTrimesh, JL_RVAL) );

bool c = IsIdType(cx, *JL_RVAL, 'TEST');

bool d = (data == GetHandlePrivate(cx, *JL_RVAL));

*/

#endif // _HANDLEAPI_H_
