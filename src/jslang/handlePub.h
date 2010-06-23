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

typedef void (*HandleFinalizeCallback_t)(void* data);

#define HANDLE_TYPE uint32_t
#define HANDLE_INVALID 0

// (TBD) better alignment: __attribute__ ((__vector_size__ (16), __may_alias__)); OR  __declspec(align(64))
//       note that SSE data must be 128bits alligned !
struct HandlePrivate {
	HANDLE_TYPE handleType;
	HandleFinalizeCallback_t finalizeCallback;
};

#define JLHID(str) JL_CAST_CSTR_TO_UINT32(str)

inline JSClass *GetHandleJSClass( JSContext *cx ) {

//	static JSClass *idJSClass = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
//	if (unlikely( idJSClass == NULL ))
//		idJSClass = JL_GetRegistredNativeClass(cx, "Id");
//	return idJSClass;
	return JL_GetRegistredNativeClass(cx, "Handle");
}


inline JSBool CreateHandle( JSContext *cx, HANDLE_TYPE handleType, size_t userDataSize, void** userData, HandleFinalizeCallback_t finalizeCallback, jsval *handleVal ) {

	JSClass *handleJSClass = GetHandleJSClass(cx);
	JL_S_ASSERT( handleJSClass != NULL, "Handle class not initialized.");
	JL_S_ASSERT( handleType != HANDLE_INVALID, "Invalid Handle type.");

	JSObject *handleObj;
	handleObj = JS_NewObject(cx, handleJSClass, NULL, NULL);
	JL_CHK( handleObj );
	*handleVal = OBJECT_TO_JSVAL(handleObj);
	HandlePrivate *pv;
	pv = (HandlePrivate*)jl_malloc(sizeof(HandlePrivate) + userDataSize);
	JL_CHK( pv );
	JL_SetPrivate(cx, handleObj, pv);

	pv->handleType = handleType;
	pv->finalizeCallback = finalizeCallback;
	if (userData)
		*userData = (char*)pv + sizeof(HandlePrivate);

	return JS_TRUE;
	JL_BAD;
}


inline JSBool HandleClose( JSContext *cx, jsval handleVal ) { // see finalize
	
	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(handleVal), GetHandleJSClass(cx));

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	JL_S_ASSERT_RESOURCE( pv );
	if ( pv->finalizeCallback ) // callback function is present
		pv->finalizeCallback((char*)pv + sizeof(HandlePrivate)); // (TBD) test it !
	jl_free(pv);
	JL_SetPrivate(cx, handleObj, NULL);

	return JS_TRUE;
	JL_BAD;
}


inline HANDLE_TYPE GetHandleType( JSContext *cx, jsval handleVal ) {

	JL_S_ASSERT_OBJECT( handleVal );
	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(handleVal), GetHandleJSClass(cx));

	JSObject *handleObj;
	handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	JL_CHK( pv != NULL );
	return pv->handleType;
bad:
	return HANDLE_INVALID;
}


inline bool IsHandleType( JSContext *cx, jsval handleVal, HANDLE_TYPE handleType ) {

	JSClass *handleJSClass = GetHandleJSClass(cx);
	if ( handleJSClass == NULL || !JsvalIsClass(handleVal, handleJSClass) )
		return false;
	JSObject *handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	return pv != NULL && pv->handleType == handleType;
}


inline void* GetHandlePrivate( JSContext *cx, jsval handleVal ) {

	JL_S_ASSERT_OBJECT(handleVal);
	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(handleVal), GetHandleJSClass(cx));

	HandlePrivate *pv;
	pv = (HandlePrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(handleVal));
	JL_CHK( pv != NULL );
	return (char*)pv + sizeof(HandlePrivate); // user data is just behind our private structure.
bad:
	return NULL;
}


inline JSBool SetHandleSlot( JSContext *cx, jsval handleVal, uint32 slotIndex, jsval value ) {

	JL_S_ASSERT_OBJECT(handleVal);
	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(handleVal), GetHandleJSClass(cx));
//	JL_S_ASSERT( slotIndex < ( (JL_GetClass(JSVAL_TO_OBJECT(handleVal))->flags >> JSCLASS_RESERVED_SLOTS_SHIFT) & JSCLASS_RESERVED_SLOTS_MASK ), "Invalid Handle slot." );
	JL_S_ASSERT( slotIndex < HANDLE_PUBLIC_SLOT_COUNT, "Invalid Handle slot." );
	return JL_SetReservedSlot(cx, JSVAL_TO_OBJECT(handleVal), slotIndex, value);
	JL_BAD;
}


inline JSBool GetHandleSlot( JSContext *cx, jsval handleVal, uint32 slotIndex, jsval *value ) {

	JL_S_ASSERT_OBJECT(handleVal);
	JL_S_ASSERT_CLASS(JSVAL_TO_OBJECT(handleVal), GetHandleJSClass(cx));
//	JL_S_ASSERT( slotIndex < ( (JL_GetClass(JSVAL_TO_OBJECT(handleVal))->flags >> JSCLASS_RESERVED_SLOTS_SHIFT) & JSCLASS_RESERVED_SLOTS_MASK ), "Invalid Handle slot." );
	JL_S_ASSERT( slotIndex < HANDLE_PUBLIC_SLOT_COUNT, "Invalid Handle slot." );
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

bool d = data == GetHandlePrivate(cx, *JL_RVAL);


*/

#endif // _HANDLEAPI_H_
