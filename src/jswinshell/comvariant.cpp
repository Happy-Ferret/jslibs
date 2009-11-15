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

// acquire the ownership of the variant
JSBool NewComVariant( JSContext *cx, VARIANT *variant, jsval *rval ) {

	JSObject *varObj = JS_NewObject(cx, classComVariant, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( varObj );
	JL_SetPrivate(cx, varObj, variant);
	return JS_TRUE;
}


JSBool JsvalToVariant( JSContext *cx, jsval *value, VARIANT *variant ) {

	if ( JSVAL_IS_STRING(*value) ) {
		
		JSString *jsStr = JSVAL_TO_STRING(*value);
		V_VT(variant) = VT_BSTR;
		V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)JS_GetStringChars(jsStr), JS_GetStringLength(jsStr));
		return JS_TRUE;
	}

	if ( JSVAL_IS_BOOLEAN(*value) ) {

		V_VT(variant) = VT_BOOL;
		V_BOOL(variant) = JSVAL_TO_BOOLEAN(*value) == JS_TRUE;
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
		time.wMilliseconds = 0; // or js_DateGetMsecSinceEpoch(cx, obj) - ???

		V_VT(variant) = VT_DATE;
		SystemTimeToVariantTime(&time, &V_DATE(variant));
		return JS_TRUE;
	}

	// last resort
	JSString *str = JS_ValueToString(cx, *value);
	JL_S_ASSERT( str, "Unable to convert to string." );

	V_VT(variant) = VT_BSTR;
	V_BSTR(variant) = SysAllocStringLen( (OLECHAR*)JS_GetStringChars(str), JS_GetStringLength(str) );

	return JS_TRUE;
	JL_BAD;
}

// acquire the ownership of the variant
JSBool VariantToJsval( JSContext *cx, VARIANT *variant, jsval *rval ) {
	
	BOOL isRef = V_ISBYREF(variant);

	switch (V_VT(variant)) {

		case VT_HRESULT:
		case VT_ERROR:

			//JS_NewObject(cx, js_ErrorClass, NULL, NULL);
			// V_ERROR / V_ERRORREF
			//JS_NewObject(cx, ComError, NULL, NULL); V_ERROR(variant)
			*rval = JSVAL_VOID;
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
		//case VT_SAFEARRAY: {

		//	HRESULT res;
		//	void *data;
		//	res = SafeArrayAccessData(V_ARRAY(variant), &data);
		//	if ( FAILED(res) )
		//		JL_CHK( WinThrowError(cx, res) );

		//	// (TBD) and now ?
		//	JL_REPORT_ERROR("Not implemented yet!");

		//	res = SafeArrayUnaccessData(V_UNION(pVar, data));
		//	if ( FAILED(res) )
		//		JL_CHK( WinThrowError(cx, res) );
		//	}
		//	break;


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

		case VT_DECIMAL: {

			HRESULT res;
			VARIANT tmpVariant;
			res = VariantChangeType(&tmpVariant, variant, 0, VT_R8);
			if ( FAILED(res) )
				JL_CHK( WinThrowError(cx, res) );
			*rval = DOUBLE_TO_JSVAL( JS_NewDouble(cx, V_ISBYREF(&tmpVariant) ? *V_R8REF(&tmpVariant) : V_R8(&tmpVariant)) );
			res = VariantClear(&tmpVariant);
			if ( FAILED(res) )
				JL_CHK( WinThrowError(cx, res) );
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

	HRESULT res = VariantClear(variant);
	if ( FAILED(res) )
		JL_CHK( WinThrowError(cx, res) );
	JS_free(cx, variant);

	return JS_TRUE;
	JL_BAD;
}


BEGIN_CLASS( ComVariant )

DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	VARIANT *variant = (VARIANT*)JL_GetPrivate(cx, obj);
	HRESULT res = VariantClear(variant);
	JS_free(cx, variant);
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
END_CLASS
