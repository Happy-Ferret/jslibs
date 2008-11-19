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

#define ID_TYPE u_int32_t

struct IdPrivate {
	ID_TYPE idType;
	IdFinalizeCallback_t finalizeCallback;
//	u_int32_t refCount;
};

inline JSClass *GetIdJSClass( JSContext *cx ) {

	static JSClass *idJSClass = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	if ( idJSClass == NULL )
		idJSClass = GetGlobalClassByName(cx, "Id");
	return idJSClass;
}


inline JSBool CreateId( JSContext *cx, ID_TYPE idType, size_t size, void** data, IdFinalizeCallback_t finalizeCallback, jsval *idVal ) {

	J_S_ASSERT( GetIdJSClass(cx) != NULL, "Id class not initialized.");

	JSObject *idObj = JS_NewObject( cx, GetIdJSClass(cx), NULL, NULL );
	J_S_ASSERT_ALLOC( idObj );
	*idVal = OBJECT_TO_JSVAL(idObj);
	IdPrivate *pv = (IdPrivate*)JS_malloc(cx, sizeof(IdPrivate) + size);
	J_S_ASSERT_ALLOC(pv);
	J_CHK( JS_SetPrivate(cx, idObj, pv) );

	pv->idType = idType;
	pv->finalizeCallback = finalizeCallback;
	if (data)
		*data = pv + sizeof(IdPrivate);
	return JS_TRUE;
bad:
	return JS_FALSE;
}


inline ID_TYPE GetIdType( JSContext *cx, jsval idVal ) {

	JSObject *idObj = JSVAL_TO_OBJECT(idVal);
	IdPrivate *pv = (IdPrivate*)JS_GetPrivate(cx, idObj);
	return pv->idType;
}


inline bool IsIdType( JSContext *cx, jsval idVal, ID_TYPE idType ) {

	JSClass *idJSClass = GetIdJSClass(cx);
	if ( idJSClass == NULL || !JsvalIsClass(cx, idVal, idJSClass) )
		return false;
	JSObject *idObj = JSVAL_TO_OBJECT(idVal);
	IdPrivate *pv = (IdPrivate*)JS_GetPrivate(cx, idObj);
	return pv != NULL && pv->idType == idType;
}


inline void* GetIdPrivate( JSContext *cx, jsval idVal ) {

	IdPrivate *pv = (IdPrivate*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(idVal));
	return pv + sizeof(IdPrivate);
}


/* Usage

void FinalizeTrimesh(void *data) {
}

...

void *data;
J_CHK( CreateId(cx, 'TEST', 10, &data, FinalizeTrimesh, J_RVAL) );

bool c = IsIdType(cx, *J_RVAL, 'TET');

bool d = data == GetIdPrivate(cx, *J_RVAL);


*/
