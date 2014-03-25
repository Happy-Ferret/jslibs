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


// see http://www.codeproject.com/KB/COM/TEventHandler.aspx
//IConnectionPoint

// http://www.codeproject.com/KB/COM/dyn_idispatch.aspx?msg=935502
class JSFunctionDispatch : public IDispatch {

public:
// IUnknown
	STDMETHOD_(ULONG, AddRef)( void ) {
		
		return ++_refs;
	}

	STDMETHOD_(ULONG, Release)( void ) {

		if( --_refs == 0 ) {

			delete this;
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
		AddRef(); // mandatory
		return NOERROR;
	}

// IDispatch
	STDMETHOD(GetTypeInfoCount)( UINT *pctinfo ) {

		JL_IGNORE(pctinfo);
/*		
		if ( pctinfo == NULL )
			return E_INVALIDARG;
		*pctinfo = 1;
		return NOERROR;
*/
		return E_NOTIMPLEMENTED;
	}

	STDMETHOD(GetTypeInfo)( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo ) {

		JL_IGNORE(ppTInfo, lcid, iTInfo);
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

		JL_IGNORE(rgDispId, lcid, cNames, rgszNames, riid);
//		DispGetIDsOfNames
		return DISP_E_UNKNOWNNAME;
	}

	// doc: http://msdn.microsoft.com/en-us/library/ms221479.aspx
	STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr ) {

		JL_IGNORE(puArgErr, pExcepInfo, lcid);

		if ( !(wFlags & DISPATCH_METHOD) || dispIdMember != DISPID_VALUE )
			return DISP_E_MEMBERNOTFOUND;

		if ( riid != IID_NULL )
			return DISP_E_UNKNOWNINTERFACE;
		
		unsigned argc = pDispParams->cArgs;

		JSContext *cx = NULL;
		JS_ContextIterator(_rt, &cx);
		JS_ASSERT( cx != NULL );

		JS::RootedValue rval(_rt);
		JS::AutoValueVector avv(cx);
		avv.reserve(argc);

		ASSERT(false); // TBD

		JS::RootedObject globalObj(_rt, JL_GetGlobal(cx));

		bool status = jl::call(cx, globalObj, _funcVal, avv, &rval);

		JL_IGNORE(status); // (TBD) error check

		//JL_CHK( JS::Call(cx, JL_GetGlobal(cx), _funcVal, JL_ARG S, &rval) );


//		if ( !status )
		
		// pVarResult: location where the result is to be stored, or NULL if the caller expects no result.
		// This argument is ignored if DISPATCH_PROPERTYPUT or DISPATCH_PROPERTYPUTREF is specified.
		if ( pVarResult != NULL && (wFlags & (DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF)) != 0 )
			JsvalToVariant(cx, rval, pVarResult);
		
		return NOERROR;
	}

	JSFunctionDispatch(JSRuntime *rt, JS::HandleValue funcVal) : _refs(0), _rt(rt), _funcVal(rt, funcVal) {

		AddRef();
	}

	~JSFunctionDispatch() {
	}

private:
	JSRuntime *_rt;
	JS::PersistentRootedValue _funcVal;
	ULONG _refs;
};


bool BlobToVariant( JSContext *cx, JS::HandleValue val, VARIANT *variant ) {

	JLData buf;
	JL_CHK( JL_JsvalToNative(cx, val, &buf) );
	variant->vt = VT_ARRAY | VT_UI1;
	SAFEARRAYBOUND rgsabound[1];
	rgsabound[0].cElements = buf.Length();
	rgsabound[0].lLbound = 0;
	variant->parray = SafeArrayCreate(VT_UI1, 1, rgsabound);
	JL_ASSERT_ALLOC( variant->parray );

	void *pArrayData = NULL;
	HRESULT hr = SafeArrayAccessData(variant->parray, &pArrayData);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );
	jl::memcpy(pArrayData, buf.GetConstStr(), buf.Length());
	SafeArrayUnaccessData(variant->parray);

	return true;
	JL_BAD;
}


bool VariantToBlob( JSContext *cx, IN VARIANT *variant, OUT JS::MutableHandleValue rval ) {

	JL_ASSERT( variant->vt == (VT_ARRAY | VT_UI1), E_VALUE, E_INVALID ); // "Invalid variant type."

	void * pArrayData;
	HRESULT hr = SafeArrayAccessData(variant->parray, &pArrayData);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );
	JL_CHK( JL_NewBufferCopyN(cx, pArrayData, variant->parray->rgsabound[0].cElements, rval) );
	SafeArrayUnaccessData(variant->parray);

	return true;
	JL_BAD;
}


// variant must be initialized ( see VariantInit() )
bool JsvalToVariant( JSContext *cx, IN JS::HandleValue value, OUT VARIANT *variant ) {

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());

		//if ( JL_GetClass(obj) == JL_BlobJSClass(cx) ) {
		if ( jl::isData(cx, value) ) {

			// see also: Write and read binary data in VARIANT - http://www.ucosoft.com/write-and-read-binary-data-in-variant.html
			
			JLData buf;
			JL_CHK( JL_JsvalToNative(cx, value, &buf) );
			V_VT(variant) = VT_BSTR;
			V_BSTR(variant) = SysAllocStringByteLen(buf.GetConstStr(), buf.Length());
			return true;
		}

		if ( JL_GetClass(obj) == JL_CLASS(ComDispatch) ) {
			
			IDispatch *disp = (IDispatch*)JL_GetPrivate(obj);
			JL_ASSERT_OBJECT_STATE(disp, JL_CLASS_NAME(ComDispatch));
			disp->AddRef();
			V_VT(variant) = VT_DISPATCH;
			V_DISPATCH(variant) = disp;
			return true;
		}

		if ( JL_GetClass(obj) == JL_CLASS(ComVariant) ) {
			
			VARIANT *v = (VARIANT*)JL_GetPrivate(obj);
			JL_ASSERT_OBJECT_STATE(v, JL_CLASS_NAME(ComVariant));
			HRESULT hr = VariantCopy(variant, v); // Frees the destination variant and makes a copy of the source variant.
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );
			return true;
		}

		if ( JS_ObjectIsCallable(cx, obj) ) {

			JSFunctionDispatch *disp = new JSFunctionDispatch(JL_GetRuntime(cx), value); // does the AddRef
			// beware: *value must be GC protected while disp is in use.
			V_VT(variant) = VT_DISPATCH;
			V_DISPATCH(variant) = disp;
			// (TBD) fixme
			// NewComDispatch(cx, disp, value); // GC protect the function stored in disp ?
			return true;
		}

		if ( JS_ObjectIsDate(cx, obj) ) { // see bug 625870

			ASSERT( js_DateIsValid(obj) );
			SYSTEMTIME time;
			time.wDayOfWeek = 0; // unused by SystemTimeToVariantTime
			time.wYear = (WORD)js_DateGetYear(cx, obj);
			time.wMonth = (WORD)js_DateGetMonth(cx, obj) + 1;
			time.wDay = (WORD)js_DateGetDate(cx, obj);
			time.wHour = (WORD)js_DateGetHours(cx, obj);
			time.wMinute = (WORD)js_DateGetMinutes(cx, obj);
			time.wSecond = (WORD)js_DateGetSeconds(obj);
			time.wMilliseconds = ((unsigned long)js_DateGetMsecSinceEpoch(obj)) % 1000;

			V_VT(variant) = VT_DATE;
			SystemTimeToVariantTime(&time, &V_DATE(variant));
			return true;
		}
	}

	if ( value.isString() ) {
		
		JSString *jsstr = value.toString();
		V_VT(variant) = VT_BSTR;
		size_t srclen;
		const jschar *src;
		src = JS_GetStringCharsAndLength(cx, jsstr, &srclen);
		V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)src, srclen);
		return true;
	}

	if ( value.isBoolean() ) {

		V_VT(variant) = VT_BOOL;
		V_BOOL(variant) = value.toBoolean() == true ? TRUE : FALSE;
		return true;
	}

	if ( value.isInt32() ) {
		
		int i = value.toInt32();
		if ( i >= SHRT_MIN && i <= SHRT_MAX ) {
			
			V_VT(variant) = VT_I2;
			V_I2(variant) = (SHORT)i;
			return true;
		}
		V_VT(variant) = VT_I4;
		V_I4(variant) = i;
		return true;
	}

	if ( value.isDouble() ) {
		
		double d = value.toDouble();
		int32_t i = (int32_t)d;

		if ( d == (double)i ) {

			V_VT(variant) = VT_I4;
			V_I4(variant) = i;
			return true;
		}

		V_VT(variant) = VT_R8;
		V_R8(variant) = d;
		return true;
	}

	if ( value.isUndefined() ) {

		V_VT(variant) = VT_EMPTY; // (TBD) or VT_VOID ???
		return true;
	}

	if ( value.isNull() ) {

		V_VT(variant) = VT_NULL;
		return true;
	}


	// last resort
	JSString *jsstr = JS::ToString(cx, value); // see JS_ConvertValue
	JL_ASSERT( jsstr, E_VALUE, E_CONVERT, E_TY_STRING );

	V_VT(variant) = VT_BSTR;

	size_t srclen;
	const jschar *src;
	src = JS_GetStringCharsAndLength(cx, jsstr, &srclen);
	V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)src, srclen );

	return true;
	JL_BAD;
}


// acquire the ownership of the variant
bool VariantToJsval( JSContext *cx, VARIANT *variant, JS::MutableHandleValue rval ) {
	
	BOOL isRef = V_ISBYREF(variant);
	BOOL isArray = V_ISARRAY(variant);

	if ( isArray && V_VT(variant) == VT_UI1 )
		return VariantToBlob(cx, variant, rval);

	JL_ASSERT( !isArray, E_OPERATION, E_NOTSUPPORTED ); // "Unable to convert the variant."

	switch ( V_VT(variant) ) {

		case VT_HRESULT: {
			HRESULT errorCode = isRef ? *V_I4REF(variant) : V_I4(variant); // check ->scode and ResultFromScode
			JL_CHK( WinNewError(cx, errorCode, rval) );
			}
			break;
		case VT_ERROR: {
			SCODE scode;
			if ( V_VT(variant) == VT_ERROR )
				scode = isRef ? *V_ERRORREF(variant) : V_ERROR(variant);
			else
				scode = variant->scode;
			JL_CHK( WinNewError(cx, scode, rval) );
			}
			break;
		case VT_NULL:
			rval.setNull();
			 break;
		case VT_VOID:
		case VT_EMPTY:
			rval.setUndefined();
			 break;
		case VT_DATE: {
			SYSTEMTIME time;
			INT st = VariantTimeToSystemTime(isRef ? *V_DATEREF(variant) : V_DATE(variant), &time);
			if ( st != TRUE )
				JL_CHK( WinThrowError(cx, GetLastError()) );
			JSObject *tmpObj;
			tmpObj = JS_NewDateObject(cx, time.wYear, time.wMonth-1, time.wDay, time.wHour, time.wMinute, time.wSecond); // see bug 625870
			JL_CHK( tmpObj );
			rval.setObject(*tmpObj);
			}
			break;
		case VT_BSTR: {
			BSTR bstr = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);
			JSString *str = JS_NewUCStringCopyN(cx, (const jschar*)bstr, SysStringLen(bstr));
			rval.setString(str);
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
				JL_CHK( NewComVariantCopy(cx, variant, rval) );
				break;
			}

			long lBound, uBound, size;
			SafeArrayGetLBound(psa, 0, &lBound);
			SafeArrayGetUBound(psa, 0, &uBound);
			size = uBound - lBound;

			VARIANT *varray;
			SafeArrayAccessData(psa, (void**)&varray);

			JS::RootedObject jsArr(cx, JS_NewArrayObject(cx, size));
			JL_CHK( jsArr );
			rval.setObject( *jsArr );

			for ( long i = 0; i < size; ++i ) {

				VARIANT *pvar = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
				JL_CHK( pvar );
				VariantInit(pvar);
				VariantCopyInd(pvar, &varray[i]);

				JS::RootedValue val(cx);
				JL_CHK( VariantToJsval(cx, pvar, &val) );
				JL_CHK( JL_SetElement(cx, jsArr, i - lBound, val) );
			}

			SafeArrayUnaccessData(psa);
			SafeArrayUnlock(psa);
			}
			break;

		//case VT_LPWSTR:
		//case VT_BLOB: {

		//	BLOB = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);

		//	BSTR bstr = isRef ? *V_BSTRREF(variant) : V_BSTR(variant);
		//	JL_CHK( JL_NewBufferCopyN(cx, (const jschar*)((ULONG*)bstr+1), *(ULONG*)bstr, rval) );
		//	}
		//	break;
		case VT_R4:
			rval.setDouble( isRef ? *V_R4REF(variant) : V_R4(variant) );
			break;
		case VT_R8:
			rval.setDouble( isRef ? *V_R8REF(variant) : V_R8(variant) );
			break;

		case VT_CY:
		case VT_DECIMAL: {
			HRESULT hr;
			VARIANT tmpVariant;
			VariantInit(&tmpVariant); // (TND) needed ?
			hr = VariantChangeType(&tmpVariant, variant, 0, VT_R8); // see VarR8FromCy()
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );
			rval.setDouble( V_ISBYREF(&tmpVariant) ? *V_R8REF(&tmpVariant) : V_R8(&tmpVariant) );
			hr = VariantClear(&tmpVariant);
			if ( FAILED(hr) )
				JL_CHK( WinThrowError(cx, hr) );
			}
			break;

		case VT_BOOL:
			rval.setBoolean( (isRef ? *V_BOOLREF(variant) : V_BOOL(variant)) == VARIANT_TRUE );
			break;
		case VT_I1:
			rval.setInt32(isRef ? *V_I1REF(variant) : V_I1(variant));
			break;
		case VT_I2:
			rval.setInt32(isRef ? *V_I2REF(variant) : V_I2(variant));
			break;
		case VT_I4:
			JL_CHK( JL_NativeToJsval(cx, isRef ? *V_I4REF(variant) : V_I4(variant), rval) );
			break;
		case VT_INT:
			JL_CHK( JL_NativeToJsval(cx, isRef ? *V_INTREF(variant) : V_INT(variant), rval) );
			break;

		case VT_UI1:
			rval.setInt32(isRef ? *V_UI1REF(variant) : V_UI1(variant));
			break;
		case VT_UI2:
			rval.setInt32(isRef ? *V_UI2REF(variant) : V_UI2(variant));
			break;
		case VT_UI4:
			JL_CHK( JL_NativeToJsval(cx, isRef ? *V_UI4REF(variant) : V_UI4(variant), rval) );
			break;
		case VT_UINT:
			JL_CHK( JL_NativeToJsval(cx, isRef ? *V_UINTREF(variant) : V_UINT(variant), rval) );
			break;

		case VT_DISPATCH:
			JL_CHK( NewComDispatch(cx, isRef ? *V_DISPATCHREF(variant) : V_DISPATCH(variant), rval) );
			break;

		case VT_VARIANT:
		case VT_UNKNOWN:
		default:
			JL_CHK( NewComVariantCopy(cx, variant, rval) );
			break;
	}

	// at this point, the value has successfully been transformed into a jsval.

	HRESULT hr = VariantClear(variant);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );
	JS_free(cx, variant);

	return true;
	JL_BAD;
}


BEGIN_CLASS( ComVariant )

DEFINE_FINALIZE() {

	if ( obj == jl::Host::getHost(fop->runtime()).getCachedProto(JL_THIS_CLASS_NAME) )
		return;
	VARIANT *variant = (VARIANT*)js::GetObjectPrivate(obj);
	HRESULT hr = VariantClear(variant);

	JL_IGNORE(hr); // (TBD) error check
	// (TBD) send to log !

	JS_freeop(fop, variant);
}


DEFINE_FUNCTION( toDispatch ) {

	HRESULT hr;

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	VARIANT *variant = (VARIANT*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( variant );

	if ( V_VT(variant) != VT_DISPATCH ) {

		JL_RVAL.set(JL_OBJVAL);
		return true;
	}

	if ( V_VT(variant) != VT_UNKNOWN ) {

		JL_RVAL.setUndefined();
		return true;
	}

	IUnknown *punk = NULL;
	punk = V_ISBYREF(variant) ? *V_UNKNOWNREF(variant) : V_UNKNOWN(variant);
	JL_CHK( punk );

	IDispatch *pdisp = NULL;
	hr = punk->QueryInterface(IID_IDispatch, (void**)&pdisp);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	JL_CHK( NewComDispatch(cx, pdisp, args.rval()) );
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	HRESULT hr;
	VARIANT *variant = (VARIANT*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( variant );

	VARIANT tmpVariant;
	VariantInit(&tmpVariant);
	hr = VariantChangeType(&tmpVariant, variant, 0, VT_BSTR);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	BSTR bstr = V_ISBYREF(&tmpVariant) ? *V_BSTRREF(&tmpVariant) : V_BSTR(&tmpVariant);
	JSString *str = JS_NewUCStringCopyN(cx, (const jschar*)bstr, SysStringLen(bstr));
	JL_CHK(str);
	JL_RVAL.setString(str);

	hr = VariantClear(&tmpVariant);
	if ( FAILED(hr) )
		JL_CHK( WinThrowError(cx, hr) );

	return true;
	JL_BAD;	
}


DEFINE_FUNCTION( toTypeName ) {

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	VARIANT *variant = (VARIANT*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( variant );

	char str[64];
	*str = '\0';
	strcat(str, "[");
	strcat(str, JL_THIS_CLASS->name);
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

	return JL_NativeToJsval(cx, str, JL_RVAL);
	JL_BAD;
}

/*
DEFINE_HAS_INSTANCE() {

	//*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	*bp = JL_ValueIsClass(cx, vp, JL_THIS_CLASS);
	return true;
}
*/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	IS_UNCONSTRUCTIBLE
	
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( toString )
		FUNCTION( toTypeName )
		FUNCTION( toDispatch )
	END_FUNCTION_SPEC

END_CLASS


// acquire the ownership of the variant
bool NewComVariant( JSContext *cx, VARIANT *variant, JS::MutableHandleValue rval ) {

	JS::RootedObject varObj(cx, JL_NewObjectWithGivenProto(cx, JL_CLASS(ComVariant), JL_CLASS_PROTOTYPE(cx, ComVariant)));
	JL_CHK( varObj );
	rval.setObject(*varObj);
	JL_SetPrivate(varObj, variant);
	return true;
	JL_BAD;
}

bool NewComVariantCopy( JSContext *cx, VARIANT *variant, JS::MutableHandleValue rval ) {

	VARIANT *newvariant = (VARIANT*)JS_malloc(cx, sizeof(VARIANT));
	VariantInit(newvariant);
	VariantCopy(newvariant, variant);
	return NewComVariant(cx, newvariant, rval);
}

