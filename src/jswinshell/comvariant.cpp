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


// http://www.codeproject.com/KB/COM/dyn_idispatch.aspx?msg=935502
class JSFunctionDispatch : public IDispatch {

public:
	// IUnknown
	STDMETHOD_(ULONG, AddRef)( void ) {
		
		return ++_refs;
	}

	STDMETHOD_(ULONG, Release)( void ) {

		if( --_refs == 0 ) {

			delete this; // ???
			return 0;
		}
		return _refs;
	}

	STDMETHOD(QueryInterface)( REFIID riid, void **ppvObject ) {

		if( !IsEqualIID(riid, IID_IUnknown) && !IsEqualIID(riid, IID_IDispatch) ) {
	
			*ppvObject = NULL;      
			return E_NOINTERFACE;
		}

		*ppvObject = this;
		AddRef();
		return NOERROR;
	}

	// IDispatch
	STDMETHOD(GetTypeInfoCount)( UINT *pctinfo ) {
/*		
		if ( pctinfo == NULL )
			return E_INVALIDARG;
		*pctinfo = 1;
		return NOERROR;
*/
		return E_NOTIMPL;
	}

	STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo ) {
/*
		if (ppTInfo == NULL)
			return E_INVALIDARG;
		*ppTInfo = NULL;
		if(iTInfo != 0)
			return DISP_E_BADINDEX;
		return NOERROR;
*/
		return DISP_E_BADINDEX;
	}

	STDMETHOD(GetIDsOfNames)( REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId ) {

//		DispGetIDsOfNames
		return DISP_E_UNKNOWNNAME;
	}

	STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr ) {

		if ( !(wFlags & DISPATCH_METHOD) || dispIdMember != DISPID_VALUE )
			return DISP_E_MEMBERNOTFOUND;

		if ( riid != IID_NULL )
			return DISP_E_UNKNOWNINTERFACE;
		
		uintN argc = pDispParams->cArgs;
		jsval *argv = (jsval*)alloca((argc+1) * sizeof(jsval));
		memset(argv, 0, (argc+1) * sizeof(jsval));
		
		JSTempValueRooter tvr;
		JS_PUSH_TEMP_ROOT(_cx, argc+1, argv, &tvr);

		JSBool status = JS_CallFunctionValue(_cx, JS_GetGlobalObject(_cx), OBJECT_TO_JSVAL(_funcObj), argc, argv+1, argv);
//		if ( !status )

		if ( pVarResult != NULL )
			JsvalToVariant(_cx, argv, pVarResult);
		
		JS_POP_TEMP_ROOT(_cx, &tvr);

		return NOERROR;
	}

	JSFunctionDispatch(JSContext *cx, JSObject *funcObj) : _refs(0), _cx(cx), _funcObj(funcObj) { }
//	~JSFunctionDispatch() { }

private:
	JSContext *_cx;
	JSObject *_funcObj;
	ULONG _refs;
};



// variant must be initialized (VariantInit())
JSBool JsvalToVariant( JSContext *cx, jsval *value, VARIANT *variant ) {

	if ( JSVAL_IS_STRING(*value) ) {
		
		JSString *jsStr = JSVAL_TO_STRING(*value);
		V_VT(variant) = VT_BSTR;
		V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)JS_GetStringChars(jsStr), JS_GetStringLength(jsStr));
		return JS_TRUE;
	}

	if ( JSVAL_IS_BOOLEAN(*value) ) {

		V_VT(variant) = VT_BOOL;
		V_BOOL(variant) = JSVAL_TO_BOOLEAN(*value) == JS_TRUE ? TRUE : FALSE;
		return JS_TRUE;
	}

	if ( JSVAL_IS_INT(*value) ) {
		
		int i = JSVAL_TO_INT(*value);
		if ( i >= SHRT_MIN && i <= SHRT_MAX ) {
			
			V_VT(variant) = VT_I2;
			V_I2(variant) = i;
			return JS_TRUE;
		}
		V_VT(variant) = VT_I4;
		V_I4(variant) = i;
		return JS_TRUE;
	}

	if ( JSVAL_IS_DOUBLE(*value) ) {
		
		double d = *JSVAL_TO_DOUBLE(*value);
		int32_t i = (int32_t)d;

		if ( d == (double)i ) {

			V_VT(variant) = VT_I4;
			V_I4(variant) = i;
			return JS_TRUE;
		}

		V_VT(variant) = VT_R8;
		V_R8(variant) = d;
		return JS_TRUE;
	}

	if ( JSVAL_IS_VOID(*value) ) {

		V_VT(variant) = VT_EMPTY; // (TBD) or VT_VOID ???
		return JS_TRUE;
	}

	if ( JSVAL_IS_NULL(*value) ) {

		V_VT(variant) = VT_NULL;
		return JS_TRUE;
	}

	JL_S_ASSERT( !JSVAL_IS_PRIMITIVE(*value), "Logic error. Missing primitive conversion.");

	JSObject *obj = JSVAL_TO_OBJECT(*value);

	if ( JL_GetClass(obj) == classComDispatch ) {
		
		IDispatch *disp = (IDispatch*)JL_GetPrivate(cx, obj);
		JL_S_ASSERT_RESOURCE(disp);
		disp->AddRef();
		V_VT(variant) = VT_DISPATCH;
		V_DISPATCH(variant) = disp;
		return JS_TRUE;
	}

	if ( JL_GetClass(obj) == classComVariant ) {
		
		VARIANT *v = (VARIANT*)JL_GetPrivate(cx, obj);
		JL_S_ASSERT_RESOURCE(v);
		HRESULT hr = VariantCopy(variant, v); // Frees the destination variant and makes a copy of the source variant.
		if ( FAILED(hr) )
			JL_CHK( WinThrowError(cx, hr) );
		return JS_TRUE;
	}

	if ( JS_ObjectIsFunction(cx, obj) ) {

		JSFunctionDispatch *disp = new JSFunctionDispatch(cx, obj);
//		disp->AddRef(); (TBD) ???
		V_VT(variant) = VT_DISPATCH;
		V_DISPATCH(variant) = disp;
		return JS_TRUE;
	}

//	if ( JL_ValueIsBlob(cx, *value ) {
//	}

	if ( js_DateIsValid(cx, obj) ) {

		SYSTEMTIME time;
		time.wDayOfWeek = 0; // unused by SystemTimeToVariantTime
		time.wYear = js_DateGetYear(cx, obj);
		time.wMonth = js_DateGetMonth(cx, obj)+1;
		time.wDay = js_DateGetDate(cx, obj);
		time.wHour = js_DateGetHours(cx, obj);
		time.wMinute = js_DateGetMinutes(cx, obj);
		time.wSecond = js_DateGetSeconds(cx, obj);
		time.wMilliseconds = ((unsigned long)js_DateGetMsecSinceEpoch(cx, obj)) % 1000;

		V_VT(variant) = VT_DATE;
		SystemTimeToVariantTime(&time, &V_DATE(variant));
		return JS_TRUE;
	}

	// last resort
	JSString *str = JS_ValueToString(cx, *value); // see JS_ConvertValue
	JL_S_ASSERT( str, "Unable to convert to string." );

	V_VT(variant) = VT_BSTR;
	V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)JS_GetStringChars(str), JS_GetStringLength(str) );

	return JS_TRUE;
	JL_BAD;
}


// acquire the ownership of the variant
JSBool VariantToJsval( JSContext *cx, VARIANT *variant, jsval *rval ) {
	
	BOOL isRef = V_ISBYREF(variant);

	switch ( V_VT(variant) ) {

		case VT_HRESULT: {
			
			HRESULT errorCode =  isRef ? *V_I4REF(variant) : V_I4(variant); // check ->scode and ResultFromScode
			LPVOID lpMsgBuf;
			DWORD result = ::FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
				NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL
			);
			if ( result == 0 )
				return JS_FALSE;
			JSString *jsStr = JS_NewStringCopyZ(cx, (const char*)lpMsgBuf);
			LocalFree(lpMsgBuf);
			jsval errVal = STRING_TO_JSVAL(jsStr);
			HostPrivate *hpv = GetHostPrivate(cx);
			JL_CHK( JS_ConstructObjectWithArguments(cx, hpv->errorObjectClass, NULL, NULL, 1, &errVal) );
			}
			break;
		case VT_ERROR: {

			SCODE scode;
			if ( V_VT(variant) == VT_ERROR )
				scode = isRef ? *V_ERRORREF(variant) : V_ERROR(variant);
			else
				scode = variant->scode;

			JL_CHK( JS_NewNumberValue(cx, scode, rval) );
			HostPrivate *hpv = GetHostPrivate(cx);
			JL_CHK( JS_ConstructObjectWithArguments(cx, hpv->errorObjectClass, NULL, NULL, 1, rval) );
			}
			break;
		case VT_NULL:
			 *rval = JSVAL_NULL;
			 break;
		case VT_VOID:
		case VT_EMPTY:
			*rval = JSVAL_VOID;
			 break;
		case VT_DATE: {
			
			SYSTEMTIME time;
			INT st = VariantTimeToSystemTime(isRef ? *V_DATEREF(variant) : V_DATE(variant), &time);
			if ( st != TRUE )
				JL_CHK( WinThrowError(cx, GetLastError()) );
			JSObject *tmpObj;
			tmpObj = js_NewDateObject(cx, time.wYear, time.wMonth-1, time.wDay, time.wHour, time.wMinute, time.wSecond);
			JL_CHK( tmpObj );
			*rval = OBJECT_TO_JSVAL(tmpObj);
			}
			break;
		case VT_BSTR: {

			BSTR bstr = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);
			JSString *str = JS_NewUCStringCopyN(cx, (const jschar*)bstr, SysStringLen(bstr));
			*rval = STRING_TO_JSVAL(str);
			}
			break;
		//case VT_BSTR_BLOB: // For system use only.

//		case VT_PTR:

		case VT_SAFEARRAY: {

			SAFEARRAY *psa = isRef ? *V_ARRAYREF(variant) : V_ARRAY(variant);
			HRESULT hr = SafeArrayLock(psa);
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );

			if ( SafeArrayGetDim(psa) != 1 ) { // at the moment we only manage 1D arrays

				SafeArrayUnlock(psa);
				JL_CHK( NewComVariant(cx, variant, rval) );
				return JS_TRUE;
			}

			long lBound, uBound, size;
			SafeArrayGetLBound(psa, 0, &lBound);
			SafeArrayGetUBound(psa, 0, &uBound);
			size = uBound - lBound;

			VARIANT *varray;
			SafeArrayAccessData(psa, (void**)&varray);

			JSObject *jsArr = JS_NewArrayObject(cx, size, NULL);
			JL_CHK( jsArr );
			*rval = OBJECT_TO_JSVAL( jsArr );

			for ( long i = 0; i < size; ++i ) {

				VARIANT *pvar = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
				JL_CHK( pvar );
				VariantInit(pvar);
				VariantCopyInd(pvar, &varray[i]);

				jsval val;
				JL_CHK( VariantToJsval(cx, pvar, &val) );
				JL_CHK( JS_SetElement(cx, jsArr, i - lBound, &val) );
			}

			SafeArrayUnaccessData(psa);
			SafeArrayUnlock(psa);
			}
			break;

		//case VT_LPWSTR:
		//case VT_BLOB: {

		//	BLOB = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);

		//	BSTR bstr = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);
		//	JL_CHK( JL_NewBlobCopyN(cx, (const jschar*)((ULONG*)bstr+1), *(ULONG*)bstr, rval) );
		//	}
		//	break;
		case VT_R4:
			*rval = DOUBLE_TO_JSVAL( JS_NewDouble(cx, isRef ? *V_R4REF(variant) : V_R4(variant)) );
			break;
		case VT_R8:
			*rval = DOUBLE_TO_JSVAL( JS_NewDouble(cx, isRef ? *V_R8REF(variant) : V_R8(variant)) );
			break;

		case VT_CY:
		case VT_DECIMAL: {

			HRESULT hr;
			VARIANT tmpVariant;
			VariantInit(&tmpVariant); // (TND) needed ?
			hr = VariantChangeType(&tmpVariant, variant, 0, VT_R8); // see VarR8FromCy()
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );
			*rval = DOUBLE_TO_JSVAL( JS_NewDouble(cx, V_ISBYREF(&tmpVariant) ? *V_R8REF(&tmpVariant) : V_R8(&tmpVariant)) );
			hr = VariantClear(&tmpVariant);
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );
			}
			break;

		case VT_BOOL:
			*rval = (isRef ? *V_BOOLREF(variant) : V_BOOL(variant)) ? JSVAL_TRUE : JSVAL_FALSE;
			break;
		case VT_I1:
			*rval = INT_TO_JSVAL(isRef ? *V_I1REF(variant) : V_I1(variant));
			break;
		case VT_I2:
			*rval = INT_TO_JSVAL(isRef ? *V_I2REF(variant) : V_I2(variant));
			break;
		case VT_I4:
			JL_CHK( IntToJsval(cx, isRef ? *V_I4REF(variant) : V_I4(variant), rval) );
			break;
		case VT_INT:
			JL_CHK( IntToJsval(cx, isRef ? *V_INTREF(variant) : V_INT(variant), rval) );
			break;

		case VT_UI1:
			*rval = INT_TO_JSVAL(isRef ? *V_UI1REF(variant) : V_UI1(variant));
			break;
		case VT_UI2:
			*rval = INT_TO_JSVAL(isRef ? *V_UI2REF(variant) : V_UI2(variant));
			break;
		case VT_UI4:
			JL_CHK( IntToJsval(cx, isRef ? *V_UI4REF(variant) : V_UI4(variant), rval) );
			break;
		case VT_UINT:
			JL_CHK( IntToJsval(cx, isRef ? *V_UINTREF(variant) : V_UINT(variant), rval) );
			break;

		case VT_DISPATCH:
			JL_CHK( NewComDispatch(cx, isRef ? *V_DISPATCHREF(variant) : V_DISPATCH(variant), rval) );
			break;

		case VT_VARIANT:
		case VT_UNKNOWN:
		default:
			JL_CHK( NewComVariant(cx, variant, rval) );
			return JS_TRUE;
	}

	HRESULT hr = VariantClear(variant);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );
	JS_free(cx, variant);

	return JS_TRUE;
	JL_BAD;
}


BEGIN_CLASS( ComVariant )


// acquire the ownership of the variant
JSBool NewComVariant( JSContext *cx, VARIANT *variant, jsval *rval ) {

	JSObject *varObj = JS_NewObject(cx, _class, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, variant);
	return JS_TRUE;
}


DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	VARIANT *variant = (VARIANT*)JL_GetPrivate(cx, obj);
	HRESULT hr = VariantClear(variant);
	JS_free(cx, variant);
}


DEFINE_FUNCTION_FAST( toDispatch ) {

	HRESULT hr;

	IUnknown *punk = NULL;

	VARIANT *variant = (VARIANT*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( variant );

	if ( V_VT(variant) != VT_UNKNOWN ) {

		*JL_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	punk = V_ISBYREF(variant) ? *V_UNKNOWNREF(variant) : V_UNKNOWN(variant);
	JL_CHK( punk );
	punk->AddRef();

	IDispatch *pdisp;
	hr = punk->QueryInterface(IID_IDispatch, (void**)&pdisp);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JL_CHK( NewComDispatch(cx, pdisp, JL_FRVAL) );

	punk->Release();
	return JS_TRUE;

bad:
	if ( punk != NULL )
		punk->Release();
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( toString ) {

	HRESULT hr;

	VARIANT *variant = (VARIANT*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( variant );

	VARIANT tmpVariant;
	VariantInit(&tmpVariant);
	hr = VariantChangeType(&tmpVariant, variant, 0, VT_BSTR);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	BSTR bstr = V_ISBYREF(&tmpVariant) ? *V_BSTRREF(&tmpVariant) : V_BSTR(&tmpVariant);
	JSString *str = JS_NewUCStringCopyN(cx, (const jschar*)bstr, SysStringLen(bstr));
	JL_CHK(str);
	*JL_FRVAL = STRING_TO_JSVAL(str);

	hr = VariantClear(&tmpVariant);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	return JS_TRUE;
	JL_BAD;	
}


DEFINE_FUNCTION_FAST( toTypeName ) {

	VARIANT *variant = (VARIANT*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( variant );

	char str[64];
	*str = '\0';
	strcat(str, "[");
	strcat(str, _class->name);
	strcat(str, " ");
	if ( V_ISBYREF(variant) )
		strcat(str, "*");
	switch ( V_VT(variant) ) {
		case VT_EMPTY: strcat(str, "EMPTY"); break;
		case VT_NULL: strcat(str, "NULL"); break;
		case VT_I2: strcat(str, "I2"); break;
		case VT_I4: strcat(str, "I4"); break;
		case VT_R4: strcat(str, "R4"); break;
		case VT_R8: strcat(str, "R8"); break;
		case VT_CY: strcat(str, "CY"); break;
		case VT_DATE: strcat(str, "DATE"); break;
		case VT_BSTR: strcat(str, "BSTR"); break;
		case VT_DISPATCH: strcat(str, "DISPATCH"); break;
		case VT_ERROR: strcat(str, "ERROR"); break;
		case VT_BOOL: strcat(str, "BOOL"); break;
		case VT_VARIANT: strcat(str, "VARIANT"); break;
		case VT_UNKNOWN: strcat(str, "UNKNOWN"); break;
		case VT_DECIMAL: strcat(str, "DECIMAL"); break;
		case VT_I1: strcat(str, "I1"); break;
		case VT_UI1: strcat(str, "UI1"); break;
		case VT_UI2: strcat(str, "UI2"); break;
		case VT_UI4: strcat(str, "UI4"); break;
		case VT_I8: strcat(str, "I8"); break;
		case VT_UI8: strcat(str, "UI8"); break;
		case VT_INT: strcat(str, "INT"); break;
		case VT_UINT: strcat(str, "UINT"); break;
		case VT_VOID: strcat(str, "VOID"); break;
		case VT_HRESULT: strcat(str, "HRESULT"); break;
		case VT_PTR: strcat(str, "PTR"); break;
		case VT_SAFEARRAY: strcat(str, "SAFEARRAY"); break;
		case VT_CARRAY: strcat(str, "CARRAY"); break;
		case VT_USERDEFINED: strcat(str, "USERDEFINED"); break;
		case VT_LPSTR: strcat(str, "LPSTR"); break;
		case VT_LPWSTR: strcat(str, "LPWSTR"); break;
		case VT_RECORD: strcat(str, "RECORD"); break;
		case VT_INT_PTR: strcat(str, "INT_PTR"); break;
		case VT_UINT_PTR: strcat(str, "UINT_PTR"); break;
		case VT_FILETIME: strcat(str, "FILETIME"); break;
		case VT_BLOB: strcat(str, "BLOB"); break;
		case VT_STREAM: strcat(str, "STREAM"); break;
		case VT_STORAGE: strcat(str, "STORAGE"); break;
		case VT_STREAMED_OBJECT: strcat(str, "STREAMED_OBJECT"); break;
		case VT_STORED_OBJECT: strcat(str, "STORED_OBJECT"); break;
		case VT_BLOB_OBJECT: strcat(str, "BLOB_OBJECT"); break;
		case VT_CF: strcat(str, "CF"); break;
		case VT_CLSID: strcat(str, "CLSID"); break;
		case VT_VERSIONED_STREAM: strcat(str, "VERSIONED_STREAM"); break;
		default:
			strcat(str, "???");
	}
	strcat(str, "]");

	return StringToJsval(cx, str, JL_FRVAL);
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

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( toString )
		FUNCTION_FAST( toTypeName )
		FUNCTION_FAST( toDispatch )
	END_FUNCTION_SPEC

END_CLASS
