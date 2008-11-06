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

#include <jsxdrapi.h>

#include "../host/host.h"
#include <process.h>


struct Private {

	JSContext *cx;
	JSXDRState *xdrCode;
	JSXDRState *xdrRval;
	int hasResult;
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Task )

DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		while ( !pv->hasResult ) // pore man's sync
			Sleep(10);

		// (TBD) wait for the thread here

		if ( pv->xdrCode ) {
			JS_XDRMemSetData(pv->xdrCode, NULL, 0);
			JS_XDRDestroy(pv->xdrCode);
		}
		if ( pv->xdrRval ) {
			JS_XDRMemSetData(pv->xdrRval, NULL, 0);
			JS_XDRDestroy(pv->xdrRval);
		}
		DestroyHost(pv->cx);
		JS_free(cx, pv);
	}
}


JSBool XdrDecodeJsval( JSContext *cx, JSXDRState *xdr, jsval *rval ) {

	JSXDRState *xdrDecoder = JS_XDRNewMem(cx, JSXDR_DECODE);
	uint32 length;
	void *data = JS_XDRMemGetData(xdr, &length);
	JS_XDRMemSetData(xdrDecoder, data, length);
	J_CHK( JS_XDRValue(xdrDecoder, rval) );
	JS_XDRMemSetData(xdrDecoder, NULL, 0);
	JS_XDRDestroy(xdrDecoder);
	return JS_TRUE;
}


void __cdecl Thread(void *arglist) {

	Private *pv = (Private*)arglist;

	jsval code, rval;
	JSBool status = XdrDecodeJsval(pv->cx, pv->xdrCode, &code);

	JSFunction *fun = JS_ValueToFunction(pv->cx, code);
	JSObject *funObj = JS_GetFunctionObject(fun);
	JSObject *globalObj = JS_GetGlobalObject(pv->cx);
	JS_SetParent(pv->cx, funObj, globalObj); // re-scope the function
	status = JS_CallFunction(pv->cx, globalObj, fun, 0, NULL, &rval);
	status = JS_XDRValue(pv->xdrRval, &rval);
	pv->hasResult++;
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	J_S_ASSERT_ALLOC(pv);
	J_CHK( JS_SetPrivate(cx, obj, pv) );

	pv->cx = CreateHost(-1, -1, 0);
	J_S_ASSERT_ALLOC(pv->cx);

	J_CHK( InitHost(pv->cx, false, NULL, NULL) );
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	
	pv->xdrCode = NULL;
	pv->xdrRval = NULL;
	pv->hasResult = 0;

	return JS_TRUE;
}


DEFINE_FUNCTION( Run ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_FUNCTION( J_ARG(1) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);

	if ( pv->xdrCode ) {

		JS_XDRMemSetData(pv->xdrCode, NULL, 0);
		JS_XDRDestroy(pv->xdrCode);
	}
	pv->xdrCode = JS_XDRNewMem(cx, JSXDR_ENCODE);
	J_CHK( JS_XDRValue(pv->xdrCode, &J_ARG(1)) );

	if ( pv->xdrRval ) {

		JS_XDRMemSetData(pv->xdrRval, NULL, 0);
		JS_XDRDestroy(pv->xdrRval);
	}
	pv->xdrRval = JS_XDRNewMem(cx, JSXDR_ENCODE);
	pv->hasResult = 0;

	uintptr_t threadHandle = _beginthread(Thread, 0, pv);
	J_S_ASSERT( threadHandle != ((uintptr_t) -1), "Unable to create the thread." ); // errno == EINVAL ...
	return JS_TRUE;
}


DEFINE_PROPERTY( result ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);

	if ( pv->hasResult == 0 ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}
	J_CHK( XdrDecodeJsval(cx, pv->xdrRval, vp) );
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Run)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(result)
	END_PROPERTY_SPEC

END_CLASS
