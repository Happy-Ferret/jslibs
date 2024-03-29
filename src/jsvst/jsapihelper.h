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


#pragma once

class JsException {
public:
	JsException( JSContext *cx, const char *text, bool force = false ) {
		
		bool hasException = JL_IsExceptionPending(cx);
		if ( force || !hasException )
			JS_ReportError(cx, text);
	}
};


class JSApiHelper {
private:
	JSContext *&_cx;
	JSApiHelper & operator =(const JSApiHelper &);

public:
	JSApiHelper( JSContext *&cx ) : _cx(cx) {}

protected:

	inline jsval GetProperty( JSObject *obj, const char *propertyName ) {

		jsval val;
		if ( !JS_GetProperty(_cx, obj, propertyName, &val) )
			throw JsException(_cx, "cannot get the property");
		return val;
	}

	inline void SetProperty( JSObject *obj, const char *propertyName, jsval val ) {

		if ( !JS_SetProperty(_cx, obj, propertyName, &val) )
//		if ( !JS_DefineProperty(_cx, obj, propertyName, &val, NULL, NULL, JSPROP_ENUMERATE) )
			throw JsException(_cx, "cannot set the property");
	}

	inline int 	JsvalToInt( jsval val ) {

		int32_t i;
		if ( !JS::ToInt32(_cx, val, &i) )
			throw JsException(_cx, "cannot convert to an integer");
		return i;
	}

	inline jsval IntToJsval( int i ) {

		if ( i >= JSVAL_INT_MIN && i <= JSVAL_INT_MAX )
			return INT_TO_JSVAL(i);
		jsval rval;
		rval.setNumber(i);
		return rval;
	}

	inline double JsvalToReal( jsval val ) {

		if ( val == JS_GetNaNValue(_cx) ) // (TBD) needed ?
			throw JsException(_cx, "not a number");
		double d;
		if ( !jl::getValue(_cx, val, &d) )
			throw JsException(_cx, "cannot convert to a real");
		return d;
	}

	inline jsval RealToJsval( double d ) {

		jsval rval;
		rval.setNumber(d);
		if ( rval == JS_GetNaNValue(_cx) ) // (TBD) needed ?
			throw JsException(_cx, "not a number");
		return rval;
	}

	inline bool JsvalToBool( jsval val ) {

		bool b;
		if ( !JS::ToBoolean(_cx, val, &b) )
			throw JsException(_cx, "cannot convert the value to a boolean");
		return b == true;
	}

	inline jsval BoolToJsval( bool b ) {

		return BOOLEAN_TO_JSVAL( b );
	}

	inline JLData JsvalToString( jsval &val ) { // beware: val is a reference !

		JSString *jsstr = JS::ToString(_cx, val);
		val = STRING_TO_JSVAL(jsstr);
		if ( jsstr == NULL )
			throw JsException(_cx, "cannot convert the value to a string");
		return JLData(_cx, jsstr);
	}

	inline void CopyJsvalToString( jsval val, char *str, size_t maxLength ) {

		if ( maxLength == 0 )
			return;
		if ( maxLength == 1 ) {
			str[0] = '\0';
			return;
		}
		JSString *jsstr = JS::ToString(_cx, val);
		if ( jsstr == NULL )
			throw JsException(_cx, "cannot convert the value to a string");
		size_t len = JL_GetStringLength(jsstr);
		if ( len > maxLength - 1 )
			 len = maxLength - 1;
		{
		JLData tmp(_cx, jsstr);
		jl::memcpy(str, tmp.GetConstStr(), tmp.Length());
		}
		str[len] = '\0';
	}

	inline jsval StringToJsval( const char *str ) {

		if ( str == NULL )
			return JSVAL_NULL;
		if ( str[0] == '\0' )
			return JL_GetEmptyStringValue(_cx);
		JSString *jsstr = JS_NewStringCopyZ(_cx, str);
		if ( jsstr == NULL )
			throw JsException(_cx, "cannot create the string");
		return STRING_TO_JSVAL(jsstr);
	}

	inline jsval StringToJsval( const char *str, size_t length ) {

		JSString *jsstr = JS_NewStringCopyN(_cx, str, length);
		if ( jsstr == NULL )
			throw JsException(_cx, "cannot create the string");
		return STRING_TO_JSVAL(jsstr);
	}

	inline bool JsvalIsString( jsval val ) {

		if ( val.isString() )
			return true;
		if ( !JSVAL_IS_PRIMITIVE(val) )
			return false;
		JSObject *stringPrototype;
		if ( !JS_GetClassObject(_cx, JL_GetGlobal(_cx), JSProto_String, &stringPrototype) ) // (TBD) see GetStringClass(cx);
			return false;
		if ( JL_GetPrototype(_cx, JSVAL_TO_OBJECT(val)) == stringPrototype )
			return true;
		return false;
	}

	inline bool jsvalIsReal( jsval val ) {

		return val.isDouble();
	}

	inline int AssertRange( int value, int min, int max ) {

		if ( value < min || value > max )
			throw JsException(_cx, "value out of range");
		return value;
	}

	inline jsval AssertString( jsval val ) {

		if ( !JsvalIsString(val) )
			throw JsException(_cx, "value is not a string");
		return val;
	}

	inline jsval AssertInt( jsval val ) {

		if ( !JSVAL_IS_INT(val) )
			throw JsException(_cx, "value is not an integer");
		return val;
	}

	inline jsval AssertNumber( jsval val ) {

		if ( !val.isNumber() )
			throw JsException(_cx, "value is not a number");
		return val;
	}

	inline jsval AssertDefined( jsval val ) {

		if ( val.isUndefined() )
			throw JsException(_cx, "value is undefined");
		return val;
	}

	inline bool InstanceOf( JSObject *obj, JSClass *jsClass ) {

		return jl::isInstanceOf(_cx, obj, jsClass) == true;
	}

	inline bool InstanceOf( jsval val, JSClass *jsClass ) {

		if ( !JSVAL_IS_PRIMITIVE(val) )
			return false;
		return jl::isInstanceOf(_cx, JSVAL_TO_OBJECT(val), jsClass) == true;
	}

	inline bool JsvalIsCallable( jsval val ) {

		return JS_TypeOfValue(_cx, val) == JSTYPE_FUNCTION;
	}

	inline jsval FunctionCall0( JSObject *obj, jsval fval ) {

		jsval rval;
		if ( JS_CallFunctionValue(_cx, obj, fval, 0, NULL, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall1( JSObject *obj, jsval fval, jsval arg1 ) {

		jsval rval;
		if ( JS_CallFunctionValue(_cx, obj, fval, 1, &arg1, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall2( JSObject *obj, jsval fval, jsval arg1, jsval arg2 ) {

		jsval rval, args[] = { arg1, arg2 };
		if ( JS_CallFunctionValue(_cx, obj, fval, 2, args, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall3( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3 ) {

		jsval rval, args[] = { arg1, arg2, arg3 };
		if ( JS_CallFunctionValue(_cx, obj, fval, sizeof(args)/sizeof(*args), args, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall4( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3, jsval arg4 ) {

		jsval rval, args[] = { arg1, arg2, arg3, arg4 };
		if ( JS_CallFunctionValue(_cx, obj, fval, sizeof(args)/sizeof(*args), args, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall5( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3, jsval arg4, jsval arg5 ) {

		jsval rval, args[] = { arg1, arg2, arg3, arg4, arg5 };
		if ( JS_CallFunctionValue(_cx, obj, fval, sizeof(args)/sizeof(*args), args, &rval) != true )
			throw JsException(_cx, "unable to call the function" );
		return rval;
	}
};

