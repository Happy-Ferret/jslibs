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

inline JSBool CreateId( JSContext *cx, ID_TYPE idType, size_t size, IdFinalizeCallback_t cb, JSObject **idObj, void** data ) {

	static JSClass *IdClass = NULL;
	if ( !IdClass )
		IdClass = GetGlobalClassByName(cx, "Id");

	J_S_ASSERT( IdClass != NULL, "Id class not found.");

	*idObj = JS_NewObject( cx, IdClass, NULL, NULL );

	void **pv = (void **)JS_malloc(cx, 2 * sizeof(void*) + size);
	J_S_ASSERT_ALLOC(pv);
	J_CHK( JS_SetPrivate(cx, *idObj, pv) );

	pv[0] = (void*)idType;
	pv[1] = (void*)cb;
	*data = pv+2;

	return JS_TRUE;
}

inline ID_TYPE GetIdType( JSContext *cx, JSObject *idObj ) {

	void **pv = (void**)JS_GetPrivate(cx, idObj);
	return (ID_TYPE)pv[0];
}

inline bool IsIdType( JSContext *cx, JSObject *idObj, ID_TYPE idType ) {

	void **pv = (void**)JS_GetPrivate(cx, idObj);
	return pv != NULL && *pv == (void*)idType;
}

inline void* GetIdPrivate( JSContext *cx, JSObject *idObj ) {

	void **pv = (void**)JS_GetPrivate(cx, idObj);
	return pv + 2;
}


/* Usage

void FinalizeTrimesh(void *data) {
}

...

JSObject *idObj;
void *data;
J_CHK( CreateId(cx, 'GlTr', 4, FinalizeTrimesh, &idObj, &data) );


*/
