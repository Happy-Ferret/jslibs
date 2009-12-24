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

typedef void (*HandleFinalizeCallback_t)(void* data);

#define HANDLE_TYPE uint32_t

// (TBD) better alignment: __attribute__ ((__vector_size__ (16), __may_alias__)); OR  __declspec(align(64))
//       note that SSE data must be 128bits alligned !
struct HandlePrivate {
	HANDLE_TYPE handleType;
	HandleFinalizeCallback_t finalizeCallback;
};

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

	JSObject *handleObj;
	handleObj = JS_NewObject(cx, handleJSClass, NULL, NULL);
	JL_CHK(handleObj);
	*handleVal = OBJECT_TO_JSVAL(handleObj);
	HandlePrivate *pv;
	pv = (HandlePrivate*)JS_malloc(cx, sizeof(HandlePrivate) + userDataSize);
	JL_CHK(pv);
	JL_SetPrivate(cx, handleObj, pv);

	pv->handleType = handleType;
	pv->finalizeCallback = finalizeCallback;
	if (userData)
		*userData = (char*)pv + sizeof(HandlePrivate);

	return JS_TRUE;
	JL_BAD;
}


inline HANDLE_TYPE GetHandleType( JSContext *cx, jsval handleVal ) {

	JSObject *handleObj = JSVAL_TO_OBJECT(handleVal);
	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, handleObj);
	return pv->handleType;
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

	return (char*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(handleVal)) + sizeof(HandlePrivate); // user data is just behind our private structure.
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
