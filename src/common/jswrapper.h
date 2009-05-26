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

#ifndef _JSWRAPPER_H
#define _JSWRAPPER_H



class ToJsVal {

  jsval _jsval;
  JSContext *_cx;

public:

  ToJsVal() {

    _jsval = JSVAL_VOID; // JSVAL_NULL
  }

  ToJsVal( JSContext *cx, bool val ) : _cx(cx) {

    _jsval = BOOLEAN_TO_JSVAL( val );
  }

  ToJsVal( JSContext *cx, int val ) : _cx(cx) {

    _jsval = INT_TO_JSVAL( val );
  }

  ToJsVal( JSContext *cx, unsigned int val ) : _cx(cx) {

    _jsval = INT_TO_JSVAL( val );
  }

  ToJsVal( JSContext *cx, long val ) : _cx(cx) {

    // JS_NewNumberValue
    _jsval = INT_TO_JSVAL( val );
  }

  ToJsVal( JSContext *cx, double val ) : _cx(cx) {

    //_jsval = DOUBLE_TO_JSVAL( val );
    JS_NewDoubleValue( cx, val, &_jsval );
  }

  ToJsVal( JSContext *cx, const char *val ) {

    _jsval = STRING_TO_JSVAL( JS_NewStringCopyZ( cx, val ) );
  }

  ToJsVal( JSContext *cx, const String &val ) {

    _jsval = STRING_TO_JSVAL( JS_NewStringCopyN( cx, val.c_str(), val.size() ) );
  }

  operator jsval() {

    return _jsval;
  }
};


class JsValTo {
  jsval _jsval;
  JSContext *_cx;

public:
  JsValTo( JSContext *cx, jsval val ) : _cx(cx), _jsval(val) { }

  operator bool() {

    JSBool bp;
    JS_ValueToBoolean( _cx, _jsval, &bp );
    return bp == JS_TRUE;
  }

  operator unsigned int() {

    uint16 ip;
    JS_ValueToUint16( _cx, _jsval, &ip );
    return ip;
  }

  operator long() {

    int32 ip;
    JS_ValueToInt32( _cx, _jsval, &ip);
    return ip;
  }

  operator int() {

    return operator long();
  }

  operator double() {

    jsdouble dp;
    JS_ValueToNumber( _cx, _jsval, &dp);
    return dp;
  }

  operator char*() {

		return JS_GetStringBytes( JS_ValueToString(_cx,_jsval) );
	}

  operator String() {

    JSString *str = JS_ValueToString(_cx,_jsval);
	  return String( JS_GetStringBytes( str ), JS_GetStringLength( str ) );
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////


template<class T_JsWrapper, class T_Class>
class JsClassWrapper {
protected:

  static JSPropertySpec* ObjectProperties() {
    return NULL;
  }

  static JSFunctionSpec* ObjectFunctions() {
    return NULL;
  }

  static JSPropertySpec* ClassProperties() {
    return NULL;
  }

  static JSFunctionSpec* ClassFunctions() {
    return NULL;
  }

  JsClassWrapper( JSContext *cx, JSObject *obj, const char* className ) {

    _jsClass.name = className;

    JSPropertySpec* x = ObjectProperties();

    JS_InitClass( cx, obj, NULL, &_jsClass, Constructor, 0, T_JsWrapper::ObjectProperties(), T_JsWrapper::ObjectFunctions(), T_JsWrapper::ClassProperties(), T_JsWrapper::ClassFunctions() );
  }

  ~JsClassWrapper() {
  }

	static JSClass* Class() {

    return &_jsClass;
  }

	static T_Class* Object(JSContext *cx, JSObject *obj) {

		if ( !JS_InstanceOf( cx, obj, &_jsClass, NULL) // (TBD) ... && !HasPrototype(cx, obj) ...  )
      return NULL;

    return (T_Class *) JL_GetPrivate(cx, obj);
	}


// default implementation of construct
  static T_Class* Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv) {

		return new T_Class();
	}

private:

  static JSBool Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {

    if ( !JS_IsConstructing(cx) )
			  return JS_FALSE;

		T_Class *o = T_JsWrapper::Construct(cx, obj, argc, argv);
		if ( o == NULL )
      return JS_FALSE;

		JL_SetPrivate(cx, obj, o);
		return JS_TRUE;
  }

  static void Destructor(JSContext *cx, JSObject *obj) {

    // (TBD) check obj and JL_GetPrivate return
    T_Class *o = (T_Class *) JL_GetPrivate(cx, obj);
		if ( o != NULL )
			delete o;
	}

private:
  static JSClass _jsClass;
  typedef T_Class ClassType;
};


template<class T_JsWrapper, class T_Class>
JSClass JsClassWrapper<T_JsWrapper, T_Class>::_jsClass = {
	NULL, // fill in the constructor
	JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JsClassWrapper<T_JsWrapper, T_Class>::Destructor,
  JSCLASS_NO_OPTIONAL_MEMBERS
};



// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_predir_preprocessor_operators.asp

#define HAS_RETURN *rval = ToJsVal( cx,
#define NO_RETURN (

#define ARG(N) JsValTo( cx, argv[N] )
#define NO_ARG
#define ARG_1 ARG(0)
#define ARG_2 ARG_1,ARG(1)
#define ARG_3 ARG_2,ARG(2)
#define ARG_4 ARG_4,ARG(3)
#define ARG_5 ARG_4,ARG(4)

#define DEFINE_CONSTRUCTOR(ARGS) \
  static ClassType* Construct(JSContext *cx, JSObject *obj, uintN argc, jsval *argv) { return new ClassType( ARGS ); }

#define DEFINE_FUNCTION(RET,ARGS,FUNCTION) \
  static JSBool FUNCTION( JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval ) { RET Object( cx, obj )->FUNCTION( ARGS ) );return JS_TRUE;}

#define DEFINE_PROPERTY(PROPERTY) \
  static JSBool PROPERTY##_getter( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) { *vp = ToJsVal( cx, Object( cx, obj )->PROPERTY ); return JS_TRUE; } \
  static JSBool PROPERTY##_setter( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) { Object( cx, obj )->PROPERTY = JsValTo( cx, *vp ); return JS_TRUE; }

#define DEFINE_GETTER_FUNCTION(PROPERTY) \
  static JSBool PROPERTY##_getter( JSContext *cx, JSObject *obj, jsval id, jsval *vp ) { *vp = ToJsVal( cx, Object( cx, obj )->PROPERTY() ); return JS_TRUE; }

//

//#define DECLARE_OBJECT_PROPERTY_MAP  JSPropertySpec* ObjectProperties();
#define BEGIN_OBJECT_PROPERTY_MAP static JSPropertySpec *ObjectProperties() { static JSPropertySpec p[] = {
#define BEGIN_CLASS_PROPERTY_MAP static JSPropertySpec *ClassProperties() { static JSPropertySpec p[] = {
 #define PROPERTY(name) { #name, -1, JSPROP_ENUMERATE | JSPROP_PERMANENT, name##_getter, name##_setter },
 #define PROPERTY_READONLY(name) { #name, -1, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, name##_getter, NULL },
#define END_PROPERTY_MAP {0}};return p;}

//

//#define DECLARE_OBJECT_FUNCTION_MAP  JSFunctionSpec* ObjectFunctions();
#define BEGIN_OBJECT_FUNCTION_MAP static JSFunctionSpec *ObjectFunctions() { static JSFunctionSpec f[] = {
#define BEGIN_CLASS_FUNCTION_MAP static JSFunctionSpec *ClassFunctions() { static JSFunctionSpec f[] = {
 #define FUNCTION(name,count) { #name, name, count, 0, 0 },
#define END_FUNCTION_MAP {0}};return f;}



#define INIT_CLASS(WRAPPER,CLASS) class WRAPPER : public JsClassWrapper<WRAPPER,CLASS> {public: WRAPPER( JSContext *cx, JSObject *obj ) : JsClassWrapper<WRAPPER,CLASS>( cx, obj, #CLASS ) {}
#define INIT_DONE };

#endif //  _JSWRAPPER_H

/*


  Curiously Recursive Template Pattern


*/