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


BEGIN_CLASS( ComDispatch )


JSBool NewComDispatch( JSContext *cx, IDispatch *pdisp, jsval *rval ) {

	JSObject *varObj = JS_NewObject(cx, _class, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, pdisp);
	pdisp->AddRef();




	return JS_TRUE;
}


DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	disp->Release();
}


static JSBool Invoke(JSContext *cx, uintN argc, jsval *vp) {

#ifdef DEBUG
	jsval dbg_funNameVal;
	JS_GetPropertyById(cx, JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)), JLID(cx, name), &dbg_funNameVal);
	jschar *dbg_name = JS_GetStringChars(JSVAL_TO_STRING( dbg_funNameVal ));
#endif

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( disp );

	JSObject *funObj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp));
	jsval dispidVal;
	JS_GetPropertyById(cx, funObj, JLID(cx, id), &dispidVal);
	DISPID dispid = JSVAL_TO_INT(dispidVal);

	HRESULT hr;

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
	UINT argErr;
	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, result, &ex, &argErr);
	for ( uintN i = 0; i < argc; ++i ) {

		hr = VariantClear(&params.rgvarg[i]);
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
	}

	if ( hr == DISP_E_EXCEPTION ) {

		JSString *exStr = JS_NewUCStringCopyZ(cx, (const jschar*)ex.bstrDescription);
		JS_SetPendingException(cx, STRING_TO_JSVAL(exStr));
		goto bad;
	}

	if ( hr == DISP_E_MEMBERNOTFOUND ) { // see DEFINE_GET_PROPERTY
		
		// remove the function because it is not a DISPATCH_METHOD
		jsval funNameVal;
		JL_CHK( JS_GetPropertyById(cx, funObj, JLID(cx, name), &funNameVal) );
		jschar *funName = JS_GetStringChars(JSVAL_TO_STRING( funNameVal ));
		jsid id;
		JL_CHK( JS_ValueToId(cx, funNameVal, &id) );
		JL_CHK( JS_DeletePropertyById(cx, JL_FOBJ, id) );
		JL_CHK( WinThrowError(cx, hr) );
	}

	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JL_CHK( VariantToJsval(cx, result, JL_FRVAL) ); // loose variant ownership
	return JS_TRUE;
	JL_BAD;
}


DEFINE_GET_PROPERTY() {

	HRESULT hr;

	EXCEPINFO ex;
	UINT err;
	DISPPARAMS params;
	VARIANT *result = NULL; // bad:

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);

	if ( JSVAL_IS_INT(id) ) {

		VARIANTARG arg;
		VariantInit(&arg);
		V_VT(&arg) = VT_I4;
		V_I4(&arg) = JSVAL_TO_INT(id);

		DISPID dispid = DISPID_VALUE;
		params.rgvarg = &arg;
		params.rgdispidNamedArgs = &dispid;
		params.cArgs = 1;
		params.cNamedArgs = 1;

		hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, &ex, &err);
		goto end;
	}

	params.rgvarg = NULL;
	params.rgdispidNamedArgs = NULL;
	params.cArgs = 0;
	params.cNamedArgs = 0;

	jschar *name;
	name = JS_GetStringChars(JSVAL_TO_STRING(id));
	DISPID dispid;
	hr = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(hr) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, hr) );

	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, &ex, &err);

	if ( hr == DISP_E_EXCEPTION ) {

		JSString *exStr = JS_NewUCStringCopyZ(cx, (const jschar*)ex.bstrDescription);
		JS_SetPendingException(cx, STRING_TO_JSVAL(exStr));
		return JS_FALSE;
	}

	if ( hr == DISP_E_MEMBERNOTFOUND || hr == DISP_E_BADPARAMCOUNT ) { // not a getter, then guess it is a method

		hr = VariantClear(result);
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
		JS_free(cx, result);

		JSFunction *fun = JS_NewFunction(cx, (JSNative)Invoke, 8, JSFUN_FAST_NATIVE, NULL, NULL);
		JSObject *funObj = JS_GetFunctionObject(fun);
		*vp = OBJECT_TO_JSVAL(funObj);
		JL_CHK( JS_DefinePropertyById(cx, funObj, JLID(cx, id), INT_TO_JSVAL(dispid), NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY) );
		JL_CHK( JS_DefinePropertyById(cx, funObj, JLID(cx, name), id, NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY) );
		JL_CHK( JS_DefinePropertyById(cx, obj, id, *vp, NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY) );
		return JS_TRUE;
	}

end:
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JL_CHK( VariantToJsval(cx, result, vp) ); // loose variant ownership
	return JS_TRUE;

bad:
	if ( result != NULL ) {
	
		VariantClear(result);
		JS_free(cx, result);
	}
	return JS_FALSE;
}


DEFINE_SET_PROPERTY() {

//	JSBool found;
//	JS_AlreadyHasOwnPropertyById(cx, obj, id, &found);

	HRESULT hr;

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	jschar *name;
	name = JS_GetStringChars(JSVAL_TO_STRING(id));

	DISPID dispid;
	hr = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(hr) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, hr) );

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
	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, flags, &params, NULL, &ex, &argErr);
	VariantClear(&arg);

	if ( FAILED(hr) ) {

		switch ( hr ) {
			case DISP_E_BADPARAMCOUNT:
				JL_REPORT_WARNING("Not a setter or read-only property.");
				return JS_TRUE;
			//case DISP_E_PARAMNOTFOUND:
			//	JL_REPORT_WARNING("Invalid argument %d.", argErr);
			//	return JS_TRUE;
			default:
				JL_CHK( WinThrowError(cx, hr) );
		}
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( FunctionList ) {

	ITypeInfo *pTypeinfo = NULL;

	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_OBJECT(JL_FARG(1));

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_FARG(1)));
	JL_S_ASSERT_RESOURCE( disp );

//	JSObject *memberList = JS_NewObjectWithGivenProto(cx, NULL, NULL, NULL);
	JSObject *memberList = JS_NewObject(cx, NULL, NULL, NULL);
	JL_CHK( memberList );
	*JL_FRVAL = OBJECT_TO_JSVAL(memberList);

	HRESULT hr;

	UINT count;
	hr = disp->GetTypeInfoCount(&count);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	if ( count == 0 )
		return JS_TRUE;

	hr = disp->GetTypeInfo(0, 0, &pTypeinfo);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	pTypeinfo->AddRef();

	FUNCDESC *pFuncDesc;
	for ( UINT i = 0; SUCCEEDED(pTypeinfo->GetFuncDesc(i, &pFuncDesc)); ++i ) {

		BSTR bstrName = NULL;
		hr = pTypeinfo->GetDocumentation(pFuncDesc->memid, &bstrName, NULL, NULL, NULL);
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
		JSString *jsstr = JS_NewUCStringCopyZ(cx, (const jschar*)bstrName);

		jsval tmp = STRING_TO_JSVAL(jsstr);
		JL_CHK( JS_SetElement(cx, memberList, i, &tmp) );
//		JL_CHK( JS_DefineUCProperty(cx, memberList, (const jschar*)bstrName, SysStringLen(bstrName), INT_TO_JSVAL(pFuncDesc->invkind), NULL, NULL, JSPROP_ENUMERATE | JSPD_PERMANENT | JSPROP_READONLY) );

		SysFreeString(bstrName);
		pTypeinfo->ReleaseFuncDesc(pFuncDesc);
	}

	pTypeinfo->Release();
	return JS_TRUE;

bad:
	if ( pTypeinfo != NULL )
		pTypeinfo->Release();
	return JS_FALSE;
}


DEFINE_ITERATOR_OBJECT() {

	HRESULT hr;

	DISPPARAMS params = {0};
	EXCEPINFO ex = {0};
	UINT argErr = 0;
	VARIANT result;

	JL_S_ASSERT( !keysonly, "Only for each..in loop is supported." );

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( disp );

	VariantInit(&result);
	hr = disp->Invoke(DISPID_NEWENUM, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &params, &result, &ex, &argErr);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	IUnknown *punk = NULL;
	if ( V_VT(&result) == VT_UNKNOWN )
		punk = V_ISBYREF(&result) ? *V_UNKNOWNREF(&result) : V_UNKNOWN(&result);
	else
	if ( V_VT(&result) == VT_DISPATCH )
		punk = V_ISBYREF(&result) ? *V_DISPATCHREF(&result) : V_DISPATCH(&result);

	JL_S_ASSERT( punk != NULL, "Invalid enum." );

	IEnumVARIANT *pEnum = NULL;
	hr = punk->QueryInterface(IID_IEnumVARIANT, (void**)&pEnum);

	VariantClear(&result); // does the punk->Release();

	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr);
	JSBool st = NewComEnum(cx, pEnum, &tvr.u.value);
	JS_POP_TEMP_ROOT(cx, &tvr);
	JL_CHK(st);
	return JSVAL_TO_OBJECT(tvr.u.value);

bad:
	return NULL;
}


DEFINE_EQUALITY() {

	*bp = JS_FALSE;
	return JS_TRUE;
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
	HAS_ITERATOR_OBJECT
	HAS_EQUALITY

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( FunctionList )
	END_STATIC_FUNCTION_SPEC

END_CLASS
