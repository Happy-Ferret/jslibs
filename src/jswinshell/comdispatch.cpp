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
#include <jsdate.h>

#include "com.h"


JSBool NewComDispatch( JSContext *cx, IDispatch *pdisp, jsval *rval ) {

	JSObject *varObj = JS_NewObject(cx, classComDispatch, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, pdisp);
	pdisp->AddRef();
	return JS_TRUE;
}


BEGIN_CLASS( ComDispatch )


DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	disp->Release();
}


static JSBool Invoke(JSContext *cx, uintN argc, jsval *vp) {

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( disp );

	JSObject *funObj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp));
	jsval dispidVal;
	JS_GetPropertyById(cx, funObj, JLID(cx, id), &dispidVal);
	DISPID dispid = JSVAL_TO_INT(dispidVal);

	HRESULT res;

	DISPPARAMS params;
	params.rgvarg = (VARIANTARG*)alloca(argc * sizeof(VARIANTARG));

	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.cArgs = argc;
	for ( uintN i = 0; i < argc; ++i ) {

		VariantInit(&params.rgvarg[i]);
		JL_CHK( JsvalToVariant(cx, &JL_FARGV[i], &params.rgvarg[i]) );
	}

	VARIANT *result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);

	EXCEPINFO ex;
	memset(&ex, 0, sizeof(EXCEPINFO));
	UINT err = 0;
	res = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, result, &ex, &err);
	for ( uintN i = 0; i < argc; ++i ) {

		res = VariantClear(&params.rgvarg[i]);
		if ( FAILED(res) )
			JL_CHK( WinThrowError(cx, res) );
	}

	if ( res == DISP_E_MEMBERNOTFOUND ) { // see DEFINE_GET_PROPERTY

		//	const char *funName = JS_GetFunctionName(JS_ValueToFunction(cx, JS_CALLEE(cx,vp)));
		jsval funNameVal;
		JS_GetPropertyById(cx, funObj, JLID(cx, name), &funNameVal);
		jschar *funName = JS_GetStringChars(JSVAL_TO_STRING( funNameVal ));
		jsid id;
		JS_ValueToId(cx, funNameVal, &id);
		JS_DeletePropertyById(cx, JL_FOBJ, id);
	}

	if ( FAILED(res) )
		JL_CHK( WinThrowError(cx, res) );

	JL_CHK( VariantToJsval(cx, result, JL_FRVAL) ); // loose variant ownership
	return JS_TRUE;
	JL_BAD;
}


DEFINE_GET_PROPERTY() {

	HRESULT res;
	DISPPARAMS params = {NULL, NULL, 0, 0};
	EXCEPINFO ex = {0};
	UINT err = 0;

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	jschar *name;
	name = JS_GetStringChars(JSVAL_TO_STRING(id));

	DISPID dispid;
	res = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(res) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, res) );

/*
	ITypeInfo *pTypeinfo;
	res = disp->GetTypeInfo(0, 0, &pTypeinfo);
	if ( FAILED(res) )
		JL_CHK( WinThrowError(cx, res) );

	FUNCDESC *pFuncDesc;
	for ( UINT index = 0; SUCCEEDED(pTypeinfo->GetFuncDesc(index, &pFuncDesc)); ++index ) {
	
		if ( pFuncDesc->memid == dispid )
			break;
		pTypeinfo->ReleaseFuncDesc(pFuncDesc);
	}
	INVOKEKIND invkind = pFuncDesc->invkind;
	pTypeinfo->ReleaseFuncDesc(pFuncDesc);
*/

	VARIANT *result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);

	res = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, &ex, &err);

	if ( res == DISP_E_MEMBERNOTFOUND ) { // not a getter, then try a method

		res = VariantClear(result);
		if ( FAILED(res) )
			JL_CHK( WinThrowError(cx, res) );
		JS_free(cx, result);

		JSFunction *fun = JS_NewFunction(cx, (JSNative)Invoke, 8, JSFUN_FAST_NATIVE, NULL, NULL);
		JSObject *funObj = JS_GetFunctionObject(fun);
		*vp = OBJECT_TO_JSVAL(funObj);
		JS_DefinePropertyById(cx, funObj, JLID(cx, id), INT_TO_JSVAL(dispid), NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY);
		JS_DefinePropertyById(cx, funObj, JLID(cx, name), id, NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY);
		JS_DefinePropertyById(cx, obj, id, *vp, NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY);
		return JS_TRUE;
	}

	JL_CHK( VariantToJsval(cx, result, vp) ); // loose variant ownership

	return JS_TRUE;
	JL_BAD;
}


DEFINE_SET_PROPERTY() {

//	JSBool found;
//	JS_AlreadyHasOwnPropertyById(cx, obj, id, &found);

	HRESULT res;

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	jschar *name;
	name = JS_GetStringChars(JSVAL_TO_STRING(id));

	DISPID dispid;
	res = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(res) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, res) );

	VARIANTARG arg;
	VariantInit(&arg);
	JL_CHK( JsvalToVariant(cx, vp, &arg) );

	WORD flags;
	if ( V_VT(&arg) == VT_DISPATCH && V_ISBYREF(&arg) )
		flags = DISPATCH_PROPERTYPUTREF;
	else
		flags = DISPATCH_PROPERTYPUT;

	DISPID dispidPut = DISPID_PROPERTYPUT;
	DISPPARAMS params;
	params.rgvarg = &arg;
	params.cArgs = 1;
	params.rgdispidNamedArgs = &dispidPut;
	params.cNamedArgs = 1;

	//VARIANT *result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	//VariantInit(result);

	EXCEPINFO ex;
	memset(&ex, 0, sizeof(EXCEPINFO));
	UINT argErr = 0;
	res = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, flags, &params, NULL, &ex, &argErr);
	VariantClear(&arg);

	if ( FAILED(res) ) {

		switch ( res ) {
			case DISP_E_BADPARAMCOUNT:
				JL_REPORT_WARNING("Not a setter or read-only property.");
				return JS_TRUE;
			//case DISP_E_PARAMNOTFOUND:
			//	JL_REPORT_WARNING("Invalid argument %d.", argErr);
			//	return JS_TRUE;
			default:
				JL_CHK( WinThrowError(cx, res) );
		}
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( methodList ) {
/*
	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	JSObject *methodList = JS_NewArrayObject(cx, 0, NULL);

	HRESULT res;

	ITypeInfo *pTypeinfo;
	disp->GetTypeInfo(0, 0, &pTypeinfo);

	pTypeinfo->AddRef();

	FUNCDESC *pFuncDesc;
	for ( UINT index = 0; SUCCEEDED(pTypeinfo->GetFuncDesc(index, &pFuncDesc)); ++index ) {
	
		pFuncDesc->
		pTypeinfo->ReleaseFuncDesc(pFuncDesc);
	}

	pTypeinfo->Release();
*/
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( propertyList ) {

	return JS_TRUE;
	JL_BAD;
}



DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && JL_GetClass(JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


CONFIGURE_CLASS
	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	HAS_HAS_INSTANCE
	HAS_FINALIZE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY
	HAS_ALL_PROPERTIES_SHARED

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( methodList )
		PROPERTY_READ( propertyList )
	END_STATIC_PROPERTY_SPEC

END_CLASS
