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
	uintptr_t threadHandle;

	JSXDRState *xdrScript;
	size_t xdrScriptLength;

	jsval rval;
	JSType type;
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Task )

DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		JS_XDRMemSetData(pv->xdrScript, NULL, 0);
		JS_XDRDestroy(pv->xdrScript);

		DestroyHost(pv->cx);
		JS_free(cx, pv);
	}
}


void __cdecl Thread(void *arglist) {

	Private *pv = (Private*)arglist;

	JSBool status;
	JSXDRState *xdr = JS_XDRNewMem(pv->cx, JSXDR_DECODE);

	uint32 length;
	void *data = JS_XDRMemGetData(pv->xdrScript, &length);
	JS_XDRMemSetData(xdr, data, length);

	jsval script;
	status = JS_XDRValue(xdr, &script);

	JSObject *obj = JS_GetGlobalObject(pv->cx);
	jsval rval;
	status = JS_CallFunctionValue(pv->cx, obj, script, 0, NULL, &rval);


	JS_XDRMemSetData(xdr, NULL, 0);
	JS_XDRDestroy(xdr);


/*
	pv->

	JSBool status;
	jschar *str = JS_GetStringChars(pv->scriptString);
	size_t len = JS_GetStringLength(pv->scriptString);

	JSObject *obj = JS_GetGlobalObject(pv->cx);


	void *buf = JS_XDRMemGetData( pv->xdrScript, &pv->xdrScriptLength );


	jsval rval;

	if ( pv->type == JSTYPE_FUNCTION ) {

		JSFunction *fun = JS_CompileUCFunction(pv->cx, obj, NULL, 0, NULL, str, len, "jslibs task", 0);
		status = JS_CallFunction(pv->cx, obj, fun, 0, NULL, &rval);
	} else 
	if ( pv->type == JSTYPE_STRING ) {

		status = JS_EvaluateUCScript(pv->cx, obj, str, len, "jslibs task", 0, &rval);
	} else {

		rval = JSVAL_VOID;
	}

	if ( status != JS_TRUE )
		return;

	if ( JSVAL_IS_PRIMITIVE(rval) ) {

		pv->rval = rval;
	} else {
		
		JSString *jsstr = JS_ValueToString(pv->cx, rval);
		pv->rval = STRING_TO_JSVAL( jsstr );
	}
	*/
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

	pv->xdrScript = JS_XDRNewMem(cx, JSXDR_ENCODE);
	pv->rval = JSVAL_VOID;

	return JS_TRUE;
}


DEFINE_FUNCTION( Exec ) {

/*
	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_FUNCTION( J_ARG(1) );
	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
//	JS_XDRMemResetData(pv->xdrScript);
	J_CHK( JS_XDRValue(pv->xdrScript, &J_ARG(1)) );
*/

	JSBool status;

	JSXDRState *xdrSrc = JS_XDRNewMem(cx, JSXDR_ENCODE);
	status = JS_XDRValue(xdrSrc, &argv[0]);

	uint32 length;
	void *data = JS_XDRMemGetData(xdrSrc, &length);

	JSXDRState *xdrDst = JS_XDRNewMem(cx, JSXDR_DECODE);
	JS_XDRMemSetData(xdrDst, data, length);

	jsval script;
	status = JS_XDRValue(xdrDst, &script);

	jsval r;
	JSObject *o = JS_GetGlobalObject(cx);
	status = JS_CallFunctionValue(cx, o, script, 0, NULL, &r);




//	pv->threadHandle = _beginthread(Thread, 0, pv);
//	J_S_ASSERT( pv->threadHandle != ((uintptr_t) -1), "Unable to create the thread." ); // errno == EINVAL ...
	return JS_TRUE;
}


DEFINE_PROPERTY( result ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	
	if ( JSVAL_IS_STRING( pv->rval ) ) {

		JSString *jsstr = JSVAL_TO_STRING(pv->rval);
		jschar *str = JS_GetStringChars(jsstr);
		size_t len = JS_GetStringLength(jsstr);
		jsstr = JS_NewUCStringCopyN(cx, str, len);
		*vp = STRING_TO_JSVAL(jsstr);
	} else {
		
		*vp = pv->rval;
	}

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(Exec)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(result)
	END_PROPERTY_SPEC

END_CLASS