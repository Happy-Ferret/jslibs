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

// BEWARE: the following helper macros are only valid inside a JS Native/FastNative function definition !

///////////////////////////////////////////////////////////////////////////////
// Safe Mode tools

#define JL_IS_SAFE (unlikely(!::_unsafeMode))
#define JL_IS_UNSAFE (likely(::_unsafeMode))

#define JL_SAFE_BEGIN if (JL_IS_SAFE) {
#define JL_SAFE_END }

#define JL_UNSAFE_BEGIN if (JL_IS_UNSAFE) {
#define JL_UNSAFE_END }

#define JL_SAFE(code) \
	JL_MACRO_BEGIN \
		if (JL_IS_SAFE) {code;} \
	JL_MACRO_END

#define JL_UNSAFE(code) \
	JL_MACRO_BEGIN \
		if (JL_IS_UNSAFE) {code;} \
	JL_MACRO_END



///////////////////////////////////////////////////////////////////////////////
// Error management

enum {
#define JL_NEW_ERR
#define DEF( NAME, TEXT, EXN ) \
	E_##NAME,
#include "jlerrors.msg"
#undef DEF
#undef JL_NEW_ERR
	E__END
};


// simple helpers
#define E_ERRNO( num )                E_ERRNO_1, num
#define E_STR( str )                  E_STR_1, str
#define E_STR16( str )                E_STR_1, static_cast<const wchar_t*>(str)
#define E_NAME( str )                 E_NAME_1, str
#define E_NAME16( str )               E_NAME_1, static_cast<const wchar_t*>(str)
#define E_NUM( num )                  E_NUM_1, num
#define E_COMMENT( str )              E_COMMENT_1, str
#define E_COMMENT2( str1, str2 )      E_COMMENT_2, str1, str2
#define E_INTERVAL_NUM( vMin, vMax )  E_INTERVAL_NUM_2, vMin, vMax
#define E_INTERVAL_STR( sMin, sMax )  E_INTERVAL_STR_2, sMin, sMax
#define E_TY_NARRAY( num )            E_TY_NARRAY_1, num
#define E_TY_NVECTOR( num )           E_TY_NVECTOR_1, num


#ifdef DEBUG
#define JL__REPORT_END_ARG E_COMMENT(JL_CODE_LOCATION), E__END
#else
#define JL__REPORT_END_ARG E__END
#endif


#define JL_BAD \
	bad:return(false)

#define JL_BADVAL(RETURN_VALUE) \
	bad:return(RETURN_VALUE)


// note: Support for variadic macros was introduced in Visual C++ 2005
#define JL_ERR( ... ) \
JL_MACRO_BEGIN \
		static const jl::ErrorManager::ErrArg args[] = { jl::ErrorManager::ErrArg() , ##__VA_ARGS__ , JL__REPORT_END_ARG }; \
		jl::Host::getJLHost(cx).errorManager().report( false, COUNTOF(args)-1, args+1 ); \
		goto bad; \
	JL_MACRO_END


#define JL_WARN( ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			static const jl::ErrorManager::ErrArg args[] = { jl::ErrorManager::ErrArg() , ##__VA_ARGS__ , JL__REPORT_END_ARG }; \
			if ( !jl::Host::getJLHost(cx).errorManager().report( true, COUNTOF(args)-1, args+1 ) ) \
				goto bad; \
		} \
	JL_MACRO_END


#define JL_CHK( CONDITION ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto bad; \
		} \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_CHKB( CONDITION, LABEL ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			goto LABEL; \
		} \
	JL_MACRO_END


#define JL_CHKM( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if (unlikely( !(CONDITION) )) { \
			JL_ERR( __VA_ARGS__ ); \
		} \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_ASSERT( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(CONDITION) )) { \
				JL_ERR( __VA_ARGS__ ); \
			} \
		} /* else if ( IS_DEBUG ) { ASSERT( (CONDITION) ); } */ \
		ASSUME(!!(CONDITION)); \
	JL_MACRO_END


#define JL_ASSERT_WARN( CONDITION, ... ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(CONDITION) )) { \
				JL_WARN( __VA_ARGS__ ); \
			} \
		} \
	JL_MACRO_END


// misc

#define JL_ASSERT_ALLOC( PTR ) \
	JL_MACRO_BEGIN \
		if ( JL_IS_SAFE ) { \
			if (unlikely( !(PTR) )) { \
				ASSERT( !JL_IsExceptionPending(cx) ); \
				JS_ReportOutOfMemory(cx); \
				goto bad; \
			} \
		} \
		else if ( IS_DEBUG ) { \
			ASSERT( (PTR) ); \
		} \
		ASSUME(PTR); \
	JL_MACRO_END


// val

#define JL_ASSERT_IS_BOOLEAN(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsBoolean)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_BOOLEAN )

#define JL_ASSERT_IS_INTEGER(val, context) \
	JL_ASSERT( NOIL(isInt)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGER )

#define JL_ASSERT_IS_INTEGER_NUMBER(val, context) \
	JL_ASSERT( NOIL(jl::isInt)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_INTEGERDOUBLE )

#define JL_ASSERT_IS_NUMBER(val, context) \
	JL_ASSERT( NOIL(jl::isNumber)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_NUMBER )

#define JL_ASSERT_IS_CALLABLE(val, context) \
	JL_ASSERT( NOIL(jl::isCallable)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_FUNC )

#define JL_ASSERT_IS_ARRAY(val, context) \
	JL_ASSERT( NOIL(jl::isArray)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_ARRAY )

#define JL_ASSERT_IS_OBJECT(val, context) \
	JL_ASSERT( val.isObject(), E_VALUE, E_STR(context), E_TYPE, E_TY_OBJECT )

#define JL_ASSERT_IS_OBJECT_OR_NULL(val, context) \
	JL_ASSERT( val.isObjectOrNull(), E_VALUE, E_STR(context), E_TYPE, E_TY_OBJECT )

#define JL_ASSERT_IS_STRING(val, context) \
	JL_ASSERT( NOIL(JL_ValueIsData)(cx, val), E_VALUE, E_STR(context), E_TYPE, E_TY_STRINGDATA )

//

#define JL_ASSERT_RANGE(val, valMin, valMax, context) \
	JL_ASSERT( jl::isInRange((int)val, (int)valMin, (int)valMax), E_VALUE, E_STR(context), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )


// arg

#define JL_ASSERT_ARGC_MIN(minCount) \
	JL_ASSERT( JL_ARGC >= (minCount), E_ARGC, E_MIN, E_NUM(minCount) )

#define JL_ASSERT_ARGC_MAX(maxCount) \
	JL_ASSERT( JL_ARGC <= (maxCount), E_ARGC, E_MAX, E_NUM(maxCount) )

#define JL_ASSERT_ARGC_RANGE(minCount, maxCount) \
	JL_ASSERT( jl::isInRange((int)JL_ARGC, (int)minCount, (int)maxCount), E_ARGC, E_RANGE, E_INTERVAL_NUM(unsigned(minCount), unsigned(maxCount)) )

#define JL_ASSERT_ARGC(count) \
	JL_ASSERT( JL_ARGC == (count), E_ARGC, E_EQUALS, E_NUM(count) )

#define JL_ASSERT_ARG_IS_BOOLEAN(argNum) \
	JL_ASSERT( NOIL(jl::isBool)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("boolean") )

#define JL_ASSERT_ARG_IS_INTEGER(argNum) \
	JL_ASSERT( NOIL(jl::isInt)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer") )

#define JL_ASSERT_ARG_IS_INTEGER_NUMBER(argNum) \
	JL_ASSERT( NOIL(jl::isInt)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("integer < 2^53") )

#define JL_ASSERT_ARG_IS_NUMBER(argNum) \
	JL_ASSERT( NOIL(jl::isNumber)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("number") )

#define JL_ASSERT_ARG_IS_OBJECT(argNum) \
	JL_ASSERT( JL_ARG(argNum).isObject(), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object") )

#define JL_ASSERT_ARG_IS_OBJECT_OR_NULL(argNum) \
	JL_ASSERT( JL_ARG(argNum).isObjectOrNull(), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("object"), E_OR, E_NAME("null") )

#define JL_ASSERT_ARG_IS_STRING(argNum) \
	JL_ASSERT( NOIL(jl::isData)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("string || data") )

#define JL_ASSERT_ARG_IS_ARRAY(argNum) \
	JL_ASSERT( NOIL(jl::isArray)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_ARRAYLIKE(argNum) \
	JL_ASSERT( NOIL(jl::isArrayLike)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("array") )

#define JL_ASSERT_ARG_IS_CALLABLE(argNum) \
	JL_ASSERT( NOIL(jl::isCallable)(cx, JL_ARG(argNum)), E_ARG, E_NUM(argNum), E_TYPE, E_NAME("function") )

// fallback
#define JL_ASSERT_ARG_TYPE(condition, argNum, typeStr) \
	JL_ASSERT( condition, E_ARG, E_NUM(argNum), E_TYPE, E_NAME(typeStr) )

#define JL_ASSERT_ARG_VAL_RANGE(val, valMin, valMax, argNum) \
	JL_ASSERT( jl::isInRange((int)val, (int)valMin, (int)valMax), E_ARG, E_NUM(argNum), E_RANGE, E_INTERVAL_NUM(valMin, valMax) )



// obj

// note:
//   If JS_IsConstructing is true, JS_THIS must not be used, the constructor should construct and return a new object.
//   JS_IsConstructing must be called from within a native given the native's original cx and vp arguments !
#define JL_ASSERT_CONSTRUCTING() \
	JL_ASSERT( args.isConstructing() /*(JL_ARGC, JS_IsConstructing(cx, vp))*/, E_THISOBJ, E_CONSTRUCT )

// note: JL_GetClass(JL_GetPrototype(... because |JL_ASSERT_THIS_INSTANCE( new Stream() )| must pass whereas |JL_ASSERT_THIS_INSTANCE( Stream.prototype )| must fail.
#define JL_ASSERT_INSTANCE( jsObject, jsClass ) \
	JL_ASSERT( JL_GetClassOfPrototype(cx, jsObject) == jsClass, E_OBJ, E_INSTANCE, E_NAME((jsClass)->name) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_THIS_INSTANCE() \
	JL_ASSERT( JL_GetClassOfPrototype(cx, JL_OBJ) == JL_THIS_CLASS, E_THISOBJ, E_INSTANCE, E_NAME(JL_THIS_CLASS_NAME) ) // ReportIncompatibleMethod(cx, CallReceiverFromArgv(argv), Valueify(clasp));

#define JL_ASSERT_INHERITANCE( jsObject, jsClass ) \
	JL_ASSERT( NOIL(jl::protoOfInheritFrom)(cx, jsObject, (jsClass)), E_OBJ, E_INHERIT, E_NAME((jsClass)->name) )

#define JL_ASSERT_THIS_INHERITANCE() \
	JL_ASSERT( NOIL(jl::protoOfInheritFrom)(cx, JL_OBJ, JL_THIS_CLASS), E_THISOBJ, E_INHERIT, E_NAME(JL_THIS_CLASS_NAME) )


#define JL_ASSERT_OBJECT_STATE( condition, name ) \
	JL_ASSERT( condition, E_OBJ, E_NAME(name), E_STATE, E_INVALID )

#define JL_ASSERT_THIS_OBJECT_STATE( condition ) \
	JL_ASSERT( condition, E_THISOBJ, E_NAME(JL_THIS_CLASS_NAME), E_STATE, E_INVALID )

