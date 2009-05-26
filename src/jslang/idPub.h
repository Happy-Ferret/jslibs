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

#include "stdafx.h"

typedef void (*IdFinalizeCallback_t)(void* data);

#define ID_TYPE uint32_t

// (TBD) better alignment: __attribute__ ((__vector_size__ (16), __may_alias__)); OR  __declspec(align(64)) 
struct IdPrivate {
	ID_TYPE idType;
	IdFinalizeCallback_t finalizeCallback;
//	u_int32_t refCount;
};

inline JSClass *GetIdJSClass( JSContext *cx ) {

//	static JSClass *idJSClass = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
//	if (unlikely( idJSClass == NULL ))
//		idJSClass = JL_GetRegistredNativeClass(cx, "Id");
//	return idJSClass;
	return JL_GetRegistredNativeClass(cx, "Id");
}


inline JSBool CreateId( JSContext *cx, ID_TYPE idType, size_t userSize, void** userData, IdFinalizeCallback_t finalizeCallback, jsval *idVal ) {

	JSClass *idJSClass = GetIdJSClass(cx);
	JL_S_ASSERT( idJSClass != NULL, "Id class not initialized.");

	JSObject *idObj;
	idObj = JS_NewObject( cx, idJSClass, NULL, NULL );
	JL_CHK( idObj );
	*idVal = OBJECT_TO_JSVAL(idObj);
	IdPrivate *pv;
	pv = (IdPrivate*)JS_malloc(cx, sizeof(IdPrivate) + userSize);
	JL_CHK(pv);
	JL_CHKB( JL_SetPrivate(cx, idObj, pv), bad_free );

	pv->idType = idType;
	pv->finalizeCallback = finalizeCallback;
	if (userData)
		*userData = (char*)pv + sizeof(IdPrivate);

	return JS_TRUE;
bad_free:
	JS_free(cx, pv);
	JL_BAD;
}


inline ID_TYPE GetIdType( JSContext *cx, jsval idVal ) {

	JSObject *idObj = JSVAL_TO_OBJECT(idVal);
	IdPrivate *pv = (IdPrivate*)JL_GetPrivate(cx, idObj);
	return pv->idType;
}


inline bool IsIdType( JSContext *cx, jsval idVal, ID_TYPE idType ) {

	JSClass *idJSClass = GetIdJSClass(cx);
	if ( idJSClass == NULL || !JsvalIsClass(idVal, idJSClass) )
		return false;
	JSObject *idObj = JSVAL_TO_OBJECT(idVal);
	IdPrivate *pv = (IdPrivate*)JL_GetPrivate(cx, idObj);
	return pv != NULL && pv->idType == idType;
}


inline void* GetIdPrivate( JSContext *cx, jsval idVal ) {

	IdPrivate *pv = (IdPrivate*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(idVal));
	return (char*)pv + sizeof(IdPrivate); // user data is just behind our private structure.
}


/* Usage

void FinalizeTrimesh(void *data) {
}

...

void *data;
JL_CHK( CreateId(cx, 'TEST', 10, &data, FinalizeTrimesh, J_RVAL) );

bool c = IsIdType(cx, *J_RVAL, 'TEST');

bool d = data == GetIdPrivate(cx, *J_RVAL);


*/
