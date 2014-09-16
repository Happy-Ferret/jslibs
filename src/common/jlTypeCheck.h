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


JL_BEGIN_NAMESPACE


ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, JS::HandleId nameId ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasPropertyById(cx, obj, nameId, &found) );
	return found;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, const CStrSpec name ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasProperty(cx, obj, name.str(), &found) );
	return found;
	JL_BAD;
}

ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleObject obj, const WCStrSpec name ) {

	ASSERT( obj );
	bool found;
	JL_CHK( JS_HasUCProperty(cx, obj, name.str(), name.len(), &found) );
	return found;
	JL_BAD;
}


// hasProperty(cx, jsval, *)
template <class NAME>
ALWAYS_INLINE bool FASTCALL
hasProperty( JSContext *cx, JS::HandleValue objVal, NAME name ) {

	ASSERT( objVal.isObject() );
	JS::RootedObject obj(cx, &objVal.toObject());
	return hasProperty(cx, obj, name);
}


////


ALWAYS_INLINE bool FASTCALL
isInstanceOf( JSContext *, JS::HandleObject obj, JSClass *clasp ) {

	return obj && JL_GetClass(obj) == clasp;
}


ALWAYS_INLINE bool FASTCALL
isObjectObject( JSContext *cx, JS::HandleObject obj ) {

//	jl::Host::getJLHost(cx)._objectProto

	ASSERT( obj != NULL );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Object, &proto) && JL_GetClass(obj) == JL_GetClass(proto);
}


ALWAYS_INLINE bool FASTCALL
isClass( JSContext *cx, JS::HandleObject obj, const JSClass *clasp ) {

	ASSERT( clasp != NULL );

	return JL_GetClass(obj) == clasp;
}

ALWAYS_INLINE bool FASTCALL
isClass( JSContext *cx, JS::HandleValue value, const JSClass *clasp ) {

	ASSERT( clasp != NULL );

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isClass(cx, obj, clasp);
	}
	return false;
}

ALWAYS_INLINE bool FASTCALL
isError( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Error, &proto) && JL_GetClass(obj) == JL_GetClass(proto); // note: JS_GetClass( (new SyntaxError()) ) => JSProto_Error
}



ALWAYS_INLINE bool FASTCALL
isCallable( JSContext *cx, JS::HandleObject obj ) {

	return JS_ObjectIsCallable(cx, obj); // FunctionClassPtr || call
}

ALWAYS_INLINE bool FASTCALL
isCallable( JSContext *cx, JS::HandleValue value ) {

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isCallable(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isBool( JSContext *cx, JS::HandleObject obj ) {

	if ( !obj.get() )
		return false;
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Boolean, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isBool( JSContext *cx, JS::HandleValue value ) {

	if ( value.isBoolean() )
		return true;
	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isBool(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isInt( JSContext *, IN JS::HandleValue value ) {

	return value.isInt32() || value.isDouble() && jl::IsIntegerValue(value.toDouble()) && jl::isInBounds<int32_t>(value.toDouble());
}


ALWAYS_INLINE bool FASTCALL
isNumber( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj.get() );
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_Number, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isNumber( JSContext *cx, JS::HandleValue value ) {

	if ( value.isNumber() )
		return true;
	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isNumber(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isDate( JSContext *cx, JS::HandleObject obj ) {

	ASSERT( obj.get() );
	return JS_ObjectIsDate(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isDate( JSContext *cx, JS::HandleValue value ) {

	if ( value.isObject() ) {

		JS::RootedObject obj(cx, &value.toObject());
		return isDate(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isNaN( JSContext *cx, IN JS::HandleValue val ) {

	ASSERT( (val.isDouble() && mozilla::IsNaN(val.toDouble())) == (val == JS_GetNaNValue(cx)) );
	return val == JS_GetNaNValue(cx);
}

ALWAYS_INLINE bool FASTCALL
isZero( JSContext *cx, IN JS::HandleValue val ) {

	return (val.isInt32() && val.toInt32() == 0) || ( val.isDouble() && val.toDouble() == 0.0); // handle "0" string too ?
}

ALWAYS_INLINE bool FASTCALL
isPInfinity( JSContext *cx, IN JS::HandleValue val ) {

	return val == JS_GetPositiveInfinityValue(cx);
}

ALWAYS_INLINE bool FASTCALL
isNInfinity( JSContext *cx, IN JS::HandleValue val ) {

	return val == JS_GetNegativeInfinityValue(cx);
}


ALWAYS_INLINE bool FASTCALL
isPositive( JSContext *cx, IN JS::HandleValue val ) {

	// handle string conversion and valueOf ?
	return ( val.toInt32() && val.toInt32() > 0 ) || ( val.isDouble() && val.toDouble() > 0 );

	//return ( val.toInt32() && val.toInt32() > 0 )
	//	|| ( val.isDouble() && val.toDouble() > 0 )
	//    || jl::isPInfinity(cx, val);
}

ALWAYS_INLINE bool FASTCALL
isNegative( JSContext *cx, IN JS::HandleValue val ) {

	// handle string conversion and valueOf ?
	return ( val.toInt32() && val.toInt32() < 0 ) || ( val.isDouble() && jl::DoubleIsNeg(val.toDouble()) );

	//return ( val.toInt32() && val.toInt32() < 0 )
	//    || ( val.isDouble() && jl::DoubleIsNeg(val.toDouble()) ) // handle NEGZERO ?
	//    || jl::isNInfinity(cx, val);
}


ALWAYS_INLINE bool FASTCALL
isString( JSContext *cx, JS::HandleObject obj ) {

	if ( !obj.get() )
		return false;
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_String, &proto) && isClass(cx, obj, JL_GetClass(proto));
}

ALWAYS_INLINE bool FASTCALL
isString( JSContext *cx, JS::HandleValue val ) {

	if ( val.isString() )
		return true;
	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isString(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
hasLatin1Chars( JSContext *cx, JS::HandleString str ) {
	
	return js::StringHasLatin1Chars(str); // jsfriendapi.h
}


ALWAYS_INLINE bool FASTCALL
isArray( JSContext *cx, JS::HandleObject obj ) {

	return JS_IsArrayObject(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isArray( JSContext *cx, IN JS::HandleValue val ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isArray(cx, obj);
	}
	return false;
}


// note that TypedArray, String and Array objects have a length property (ArrayBuffer does not), and unfortunately Function also have a length property.
ALWAYS_INLINE bool FASTCALL
isArrayLike( JSContext *cx, JS::HandleObject obj ) {

	if ( JS_IsArrayObject(cx, obj) )
		return true;
	return hasProperty(cx, obj, JLID(cx, length)) && !JS_ObjectIsFunction(cx, obj); // exclude (function(){}).length
}

ALWAYS_INLINE bool FASTCALL
isArrayLike( JSContext *cx, IN JS::HandleValue val ) {

	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return isArrayLike(cx, obj);
	}
	if ( isString(cx, val) )
		return true;
	return false;
}


ALWAYS_INLINE bool FASTCALL
isData( JSContext * cx, JS::HandleObject obj ) {

	return bufferGetInterface(cx, obj) != NULL || JS_IsArrayBufferObject(obj) || isArrayLike(cx, obj);
}

ALWAYS_INLINE bool FASTCALL
isData( JSContext *cx, JS::HandleValue val ) {

	if ( val.isString() )
		return true;
	if ( val.isObject() ) {

		JS::RootedObject obj(cx, &val.toObject());
		return NOIL(isData)(cx, obj);
	}
	return false;
}


ALWAYS_INLINE bool FASTCALL
isIterable( JSContext * cx, JS::HandleObject obj ) {

	return hasProperty(cx, obj, JLID(cx, next));
}

ALWAYS_INLINE bool FASTCALL
isIterable( JSContext * cx, JS::HandleValue val ) {

	return hasProperty(cx, val, JLID(cx, next));
}


ALWAYS_INLINE bool FASTCALL
isGenerator( JSContext *cx, JS::HandleValue val ) {

	// Function.prototype.isGenerator.call(Gen)
	JS::RootedObject valueObj(cx, &val.toObject());
	JS::RootedObject proto(cx);
	JS::RootedValue fct(cx), rval(cx);
	return val.isObject()
		&& JL_GetClassPrototype(cx, JSProto_Function, &proto)
	    && JS_GetPropertyById(cx, proto, JLID(cx, isGenerator), &fct)
		&& JS_CallFunctionValue(cx, valueObj, fct, JS::HandleValueArray::empty(), &rval)
		&& rval == JSVAL_TRUE;
}


ALWAYS_INLINE bool FASTCALL
isStopIteration( JSContext *cx, JS::HandleObject obj ) {

	JS::RootedValue val(cx);
	val.setObjectOrNull(obj);
	return JS_IsStopIteration(val);
/*
	JS::RootedObject proto(cx);
	return JL_GetClassPrototype(cx, JSProto_StopIteration, &proto)
		&& JL_GetClass(obj) == JL_GetClass(proto);
*/
}

ALWAYS_INLINE bool FASTCALL
isStopIterationExceptionPending( JSContext *cx ) {

	JS::RootedValue ex(cx);
	if ( !JS_GetPendingException(cx, &ex) || !ex.isObject() ) // note: JS_GetPendingException returns false if no exception is pending.
		return false;
	JS::RootedObject exObj(cx, &ex.toObject());
	return isStopIteration(cx, exObj);
}

JL_END_NAMESPACE
