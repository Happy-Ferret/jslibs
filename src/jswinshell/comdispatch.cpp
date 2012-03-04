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
#include "com.h"


BEGIN_CLASS( ComDispatch )

DEFINE_FINALIZE() {

	if ( obj == JL_PROTOTYPE(cx, ComDispatch) )
		return;
	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	disp->Release();
}


JSBool FunctionInvoke(JSContext *cx, uintN argc, jsval *vp) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

#ifdef DEBUG
	jsval dbg_funNameVal;
	JS_GetPropertyById(cx, JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)), JLID(cx, name), &dbg_funNameVal);
	const jschar *dbg_name = JS_GetStringCharsZ(cx, JSVAL_TO_STRING( dbg_funNameVal ));
	JL_IGNORE(dbg_name);
#endif

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( disp );

	JSObject *funObj = JSVAL_TO_OBJECT(JS_CALLEE(cx, vp));
	jsval dispidVal;
	JL_CHK( JS_GetPropertyById(cx, funObj, JLID(cx, id), &dispidVal) );
	DISPID dispid = JSVAL_TO_INT(dispidVal);

	HRESULT hr;

	DISPPARAMS params;
	params.rgvarg = (VARIANTARG*)alloca(argc * sizeof(VARIANTARG));

	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;
	params.cArgs = argc;
	for ( uintN i = 0; i < argc; ++i ) {

		VariantInit(&params.rgvarg[i]);
		JL_CHK( JL_JsvalToVariant(cx, &JL_ARGV[i], &params.rgvarg[i]) );
	}

	VARIANT *result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);

	EXCEPINFO ex;
	UINT argErr;
	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, result, &ex, &argErr);
	for ( uintN i = 0; i < argc; ++i ) {

		HRESULT hr1 = VariantClear(&params.rgvarg[i]);
		if ( FAILED(hr1) )
			JL_CHK( WinThrowError(cx, hr1) );
	}

	if ( hr == DISP_E_EXCEPTION ) {

		JL_CHK( WinThrowError(cx, ex.wCode ? ex.wCode : ex.scode) ); // (TBD) fix this
	}

	if ( hr == DISP_E_MEMBERNOTFOUND ) { // see DEFINE_GET_PROPERTY

		// remove the function because it is not a DISPATCH_METHOD
		jsval funNameVal;
		jsid funNameId;
		JL_CHK( JS_GetPropertyById(cx, funObj, JLID(cx, name), &funNameVal) );
		//JL_CHK( JL_JsvalToJsid(cx, &funNameVal, &funNameId) );
		ASSERT( JSVAL_IS_STRING(funNameVal) );
		funNameId = JL_StringToJsid(cx, JSVAL_TO_STRING( funNameVal ));
		ASSERT( JSID_IS_STRING(funNameId) );
		//JL_CHK( JL_RemovePropertyById(cx, JL_OBJ, funNameId) );
		JL_CHK( JS_DeletePropertyById(cx, JL_OBJ, funNameId) ); // beware: permanant properties cannot be removed.

	#ifdef DEBUG
		JSBool found;
		ASSERT( JS_HasPropertyById(cx, obj, funNameId, &found) && !found );
	#endif
	} // ...

	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JL_CHK( VariantToJsval(cx, result, JL_RVAL) ); // loose variant ownership
	return JS_TRUE;
	JL_BAD;
}


DEFINE_GET_PROPERTY() {

	VARIANT *result = NULL; // bad:
	
	JL_ASSERT_THIS_INSTANCE();

	HRESULT hr;

	EXCEPINFO ex;
	UINT argErr;
	DISPPARAMS params;

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( disp );

	result = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(result);

	if ( JSID_IS_INT(id) ) {

		VARIANTARG arg;
		VariantInit(&arg);
		V_VT(&arg) = VT_I4;
		V_I4(&arg) = JSID_TO_INT(id);

		DISPID dispid = DISPID_VALUE;
		params.rgvarg = &arg;
		params.rgdispidNamedArgs = &dispid;
		params.cArgs = 1;
		params.cNamedArgs = 1;

		hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, &ex, &argErr);
		goto end;
	}

	params.rgvarg = NULL;
	params.rgdispidNamedArgs = NULL;
	params.cArgs = 0;
	params.cNamedArgs = 0;

	const jschar *name;
	name = JS_GetStringCharsZ(cx, JSID_TO_STRING(id));
	DISPID dispid;
	hr = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(hr) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, hr) );

	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, result, &ex, &argErr);

	if ( hr == DISP_E_MEMBERNOTFOUND || hr == DISP_E_BADPARAMCOUNT ) { // not a getter, then guess it is a method

		hr = VariantClear(result);
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
		JS_free(cx, result);

		JSFunction *fun = JS_NewFunction(cx, FunctionInvoke, 8, JSFUN_LAMBDA, NULL, NULL);
		JSObject *funObj = JS_GetFunctionObject(fun);
		JL_CHK( funObj );
		*vp = OBJECT_TO_JSVAL(funObj);
		JL_CHK( JS_DefinePropertyById(cx, funObj, JLID(cx, id), INT_TO_JSVAL(dispid), NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY) );
		jsval tmp;
		JL_CHK( JS_IdToValue(cx, id, &tmp) );
		JL_CHK( JS_DefinePropertyById(cx, funObj, JLID(cx, name), tmp, NULL, NULL, JSPROP_PERMANENT|JSPROP_READONLY) );
		JL_CHK( JS_DefinePropertyById(cx, obj, id, *vp, NULL, NULL, /*JSPROP_PERMANENT|*/JSPROP_READONLY) ); // not JSPROP_PERMANENT else prop is undeletable, see DISP_E_MEMBERNOTFOUND case in FunctionInvoke() 
		return JS_TRUE;
	}

	if ( hr == DISP_E_EXCEPTION ) {

//		int sev = HRESULT_SEVERITY(ex.scode); // 00:Success, 01:Informational, 10:Warning, 11:Error
//		JSString *exStr = JS_NewUCStringCopyZ(cx, (const jschar*)ex.bstrDescription);
//		JS_SetPendingException(cx, STRING_TO_JSVAL(exStr));
		
		JL_CHK( WinThrowError(cx, ex.wCode ? ex.wCode : ex.scode) ); // (TBD) fix this
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
	
	JL_IGNORE(strict, cx);

	JL_ASSERT_THIS_INSTANCE();

//	JSBool found;
//	JS_AlreadyHasOwnPropertyById(cx, obj, id, &found);
//	jsval xxx;
//	JS_LookupPropertyById(cx, obj, id, &xxx);

	HRESULT hr;

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( disp );

	ASSERT( JSID_IS_STRING( id ) );
	const jschar *name;
	JSString *idStr = JSID_TO_STRING(id);
	name = JS_GetStringCharsZ(cx, idStr);

	DISPID dispid;
	hr = disp->GetIDsOfNames(IID_NULL, (OLECHAR**)&name, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	if ( FAILED(hr) ) // dispid == DISPID_UNKNOWN
		JL_CHK( WinThrowError(cx, hr) );

	VARIANTARG arg;
	VariantInit(&arg);
	JL_CHK( JL_JsvalToVariant(cx, vp, &arg) ); // *vp will be stored in an object slot !

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

//	IConnectionPoint *pcp;
//	hr = disp->QueryInterface(IID_IConnectionPoint, (void**)&pcp);
//	pcp->Release();
//	IConnectionPointContainer *pcpc;
//	hr = disp->QueryInterface(IID_IConnectionPointContainer, (void**)&pcpc);
//	pcpc->Release();

	EXCEPINFO ex;
	memset(&ex, 0, sizeof(EXCEPINFO));
	UINT argErr = 0;
	hr = disp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, flags, &params, NULL, &ex, &argErr);
	VariantClear(&arg);

	if ( FAILED(hr) ) {

		switch ( hr ) {
			case DISP_E_BADPARAMCOUNT: // doc. An error return value that indicates that the number of elements provided to the method is different from the number of arguments accepted by the method.
				JL_ERR( E_NAME(name), E_WRITE ); //JL_REPORT_ERROR_NUM( JLSMSG_LOGIC_ERROR, "read-only property"); // (TBD) be more specific
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


DEFINE_FUNCTION( functionList ) {

	ITypeInfo *pTypeinfo = NULL;

	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_IS_OBJECT(1);

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, JSVAL_TO_OBJECT(JL_ARG(1)));
	JL_ASSERT_OBJECT_STATE( disp, JL_THIS_CLASS_NAME );

//	JSObject *memberList = JL_NewProtolessObj(cx);
	JSObject *memberList = JL_NewObj(cx);
	JL_CHK( memberList );
	*JL_RVAL = OBJECT_TO_JSVAL(memberList);

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
		JL_CHK( JL_SetElement(cx, memberList, i, &tmp) );
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

	IEnumVARIANT *pEnum = NULL;
	HRESULT hr;
	DISPPARAMS params = {0};
	EXCEPINFO ex = {0};
	UINT argErr = 0;
	VARIANT result;

	JL_ASSERT_THIS_INSTANCE();

	JL_CHKM( !keysonly, E_NAME("for...in"), E_NOTSUPPORTED ); // JL_ASSERT( !keysonly, "Only for each..in loop is supported." );

	IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( disp );

	VariantInit(&result);
	hr = disp->Invoke(DISPID_NEWENUM, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD | DISPATCH_PROPERTYGET, &params, &result, &ex, &argErr);
	if ( FAILED(hr) ) {
		
		if ( hr == DISP_E_EXCEPTION )
			JL_CHK( WinThrowError(cx, ex.wCode ? ex.wCode : ex.scode) );
		JL_CHK( WinThrowError(cx, hr) );
	}

	IUnknown *punk = NULL;
	if ( V_VT(&result) == VT_UNKNOWN )
		punk = V_ISBYREF(&result) ? *V_UNKNOWNREF(&result) : V_UNKNOWN(&result);
	else
	if ( V_VT(&result) == VT_DISPATCH )
		punk = V_ISBYREF(&result) ? *V_DISPATCHREF(&result) : V_DISPATCH(&result);
	
	JL_ASSERT( punk != NULL, E_THISOBJ, E_STATE ); // JL_ASSERT( punk != NULL, "Invalid enum." );

	hr = punk->QueryInterface(IID_IEnumVARIANT, (void**)&pEnum);

	VariantClear(&result); // does the punk->Release();

	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	{
		jsval tmp;
		JSBool st = NewComEnum(cx, pEnum, &tmp);
		JL_CHK(st);
		pEnum->Release();
		return JSVAL_TO_OBJECT(tmp);
	}

bad:
	if ( pEnum )
		pEnum->Release();
	return NULL;
}


DEFINE_EQUALITY_OP() {

	JL_IGNORE(cx);
	*bp = JSVAL_IS_OBJECT(*v) && JSVAL_TO_OBJECT(*v) == obj;
	return JS_TRUE;
}

/*
DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}
*/

CONFIGURE_CLASS
	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	
	//HAS_HAS_INSTANCE
	IS_INCONSTRUCTIBLE

	HAS_FINALIZE
	HAS_GET_PROPERTY
	HAS_SET_PROPERTY
	HAS_ITERATOR_OBJECT
	HAS_EQUALITY_OP

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( functionList )
	END_STATIC_FUNCTION_SPEC

END_CLASS


JSBool NewComDispatch( JSContext *cx, IDispatch *pdisp, jsval *rval ) {

	JSObject *varObj = JL_NewObjectWithGivenProto(cx, JL_CLASS(ComDispatch), JL_PROTOTYPE(cx, ComDispatch), NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, pdisp);
	pdisp->AddRef();
	return JS_TRUE;
}
