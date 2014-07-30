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

namespace jl3 { //JL_BEGIN_NAMESPACE

struct Class;


/*
Doc("number FooBar.status: of the FooBar instance");
// ...
host.doc("FooBar.status"); ?
*/
struct Doc {
	const char *_name;
	const char *_summary;
	const char *_details;
	enum Type { typeUnknown, typeModule, typeClass, typeStaticClass, typeFunction, typeProperty, typeConst } _type;

	// creates a pending doc item
	Doc(const char *summary, const char *details = NULL)
	: _summary(summary), _details(details) {

		pendingDoc = this;
	}

	// get the ownership of the pending doc item
	Doc() {

		if ( pendingDoc ) {

			*this = *pendingDoc;
			pendingDoc = NULL;
		}
	}

	void SetName(const char *name) {

		_name = name;
	}

	void SetType(Type type) {

		_type = type;
	}

private:
	static const Doc *pendingDoc;
};

const Doc *Doc::pendingDoc = NULL;


struct ConstLink {
	ConstLink *_prev;
	const char *name;
	const JS::Value value;
	Doc doc;

	template <typename T>
	ConstLink(ConstLink *&last, const char *name, const T &value)
	: _prev(last), name(name), value(JS::NumberValue(value)), doc() {
		
		last = this;
		
		doc.SetName(name);
		doc.SetType(Doc::typeConst);
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		JS::RootedValue tmp(cx);
		for ( const ConstLink *it = this; it; it = it->_prev ) {
			
			tmp.set(it->value);
			JL_CHK( JS_DefineProperty(cx, obj, it->name, tmp, JSPROP_READONLY | JSPROP_PERMANENT) );
		}
		return true;
		bad: return false;
	}

private:
	void operator =(const ConstLink &);
	ConstLink(const ConstLink &);
};


struct ReservedSlot {
	const uint32_t index;

	ReservedSlot(uint32_t &reservedSlotCount)
	: index(reservedSlotCount++) {
	}

private:
	void operator =(const ReservedSlot &);
	ReservedSlot(const ReservedSlot &);
};


struct NameLink {
	enum { noTinyId = -1 };
	NameLink *_prev;
	const char *name;
	const int8_t tinyid;
	Doc doc;

	NameLink(NameLink *&last, const char *name, int8_t tinyid = noTinyId)
	: _prev(last), name(name), tinyid(tinyid), doc() {

		last = this;

		doc.SetName(name);
	}

private:
	void operator =(const NameLink &);
	NameLink(const NameLink &);
};


struct PropLink {
	PropLink *_prev;
	NameLink *nameLink;
	JSPropertyOp getter;
	JSStrictPropertyOp setter;

	PropLink(PropLink *&last)
	: _prev(last), nameLink(NULL), getter(NULL), setter(NULL) {
		
		last = this;
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		for ( const PropLink *it = this; it; it = it->_prev ) {

			unsigned attrs = JSPROP_PERMANENT | JSPROP_SHARED | ( it->setter ? 0 : JSPROP_READONLY ); // https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
			for ( NameLink *nameIt = it->nameLink; nameIt; nameIt = nameIt->_prev ) {

				nameIt->doc.SetType(Doc::typeProperty);

				// doc.Register(cx, ???); // link the object with the string (using weak maps ?)
				
				if ( nameIt->tinyid == NameLink::noTinyId ) {

					JL_CHK( JS_DefineProperty(cx, obj, nameIt->name, JL_UNDEFINED, attrs, it->getter, it->setter) );
				} else {

					ASSERT(false); //JL_CHK( JS_DefinePropertyWithTinyId(cx, obj, nameIt->name, nameIt->tinyid, JSVAL_VOID, it->getter, it->setter, attrs) );
				}
			}
		}
		return true;
		bad: return false;
	}

private:
	void operator =(const PropLink &);
	PropLink(const PropLink &);
};


/*
 class ArgCheck {
	virtual bool Check(JSContext *cx, JS::HandleValue val) = 0;
};

class ArgCheckBool : public ArgCheck {
public:
	bool Check(JSContext*, JS::HandleValue val) {

		return val.isBoolean();
	}
};

class ArgCheckNumberRange : public ArgCheck {
	const double min;
	const double max;
public:
	ArgCheckNumberRange(double min, double max)
	: min(min), max(max) {

		_asm { int 3 }
	}

	bool Check(JSContext*, JS::HandleValue val) {

		return val.isNumber() && val.toNumber() >= min && val.toNumber() <= max;
	}
};





struct TypeLink {
	TypeLink *_prev;
	const ArgCheck &argType;

	TypeLink(TypeLink *&last, const ArgCheck &argType)
	: _prev(last), argType(argType) {

		last = this;
	}

private:
	void operator =(const TypeLink &);
	TypeLink(const TypeLink &);
};
*/



struct FuncLink {
	FuncLink *_prev;
	NameLink *nameLink;
//	TypeLink *typeLink;
	JSNative native;
	unsigned argcMin;
	unsigned argcMax;

	FuncLink(FuncLink *&last)
	: _prev(last), nameLink(NULL), /*typeLink(NULL),*/ native(NULL), argcMin(0), argcMax(0) {

		last = this;
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		for ( const FuncLink *it = this; it; it = it->_prev ) {

			it->nameLink->doc.SetType(Doc::typeFunction);

			JSFunction *fun = JS_DefineFunction(cx, obj, it->nameLink->name, it->native, it->argcMax, 0);
			JL_CHK( fun );
			// doc.Register(cx, JS_GetFunctionObject(fun)); // link the object with the string (using weak maps ?)
		}
		return true;
		bad: return false;
	}

private:
	void operator =(const FuncLink &);
	FuncLink(const FuncLink &);
};


// ClassSpec

struct Class {
	typedef bool (*ClassInit_t)(JSContext *cx, Class *sc, JSObject *proto, JSObject *obj);
	typedef int32_t SourceId_t;

	JSClass clasp;
	const char *parentProtoName;
	FuncLink *constructor;
	FuncLink *callFuncLink;
	FuncLink *funcLink;
	FuncLink *staticFuncLink;
	PropLink *propLink;
	PropLink *staticPropLink;
	ConstLink *staticConstLink;
	uint32_t reservedSlotCount;
	ClassInit_t init;
	const double buildDate;
	SourceId_t sourceId;
	Doc doc;

	Class(const char *className = NULL)
	: constructor(NULL), callFuncLink(NULL), funcLink(NULL), staticFuncLink(NULL), propLink(NULL), staticPropLink(NULL), staticConstLink(NULL), reservedSlotCount(0), init(NULL), buildDate((double)__DATE__EPOCH * 1000), sourceId(0), doc() {

		clasp.addProperty = JS_PropertyStub;
		clasp.delProperty = JS_DeletePropertyStub;
		clasp.getProperty = JS_PropertyStub;
		clasp.setProperty = JS_StrictPropertyStub;
		clasp.enumerate = JS_EnumerateStub;
		clasp.resolve = JS_ResolveStub;
		clasp.convert = JS_ConvertStub;

		clasp.name = className;
		clasp.flags = 0;
		
		doc.SetName(className);
		doc.SetType(className ? Doc::typeClass : Doc::typeStaticClass);
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		JS::RootedObject proto(cx); // doc: object that is the prototype for the newly initialized class.
		JS::RootedObject ctor(cx);

		if ( clasp.name == NULL ) { // static class

			ASSERT( parentProtoName == NULL );
			ASSERT( reservedSlotCount == 0 );
			ASSERT( constructor == NULL );
			ASSERT( callFuncLink == NULL );
			ASSERT( clasp.flags == 0 );
			ASSERT( clasp.finalize == NULL );

			proto = obj;
			ctor = obj;
		} else {

			ASSERT( clasp.name && clasp.name[0] ); // Invalid class name.

			//HostPrivate *hpv;
			//hpv = JL_GetHostPrivate(cx);

			jl::Host &host = jl::Host::getJLHost(cx);

			JS::RootedObject parentProto(cx);
			if ( parentProtoName != NULL ) {

				parentProto = host.getCachedProto(parentProtoName);  //JL_GetCachedProto(hpv, parentProtoName);
				JL_CHKM( parentProto != NULL, E_STR(parentProtoName), E_STR("prototype"), E_NOTFOUND );
			}

			clasp.flags |= JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount);
			ASSERT( JSCLASS_RESERVED_SLOTS(&clasp) == reservedSlotCount );

			proto = JS_InitClass(cx, obj, parentProto, &clasp, constructor ? constructor->native : NULL, constructor ? constructor->argcMax : 0, NULL, NULL, NULL, NULL);

			JL_ASSERT( proto != NULL, E_CLASS, E_NAME(clasp.name), E_CREATE ); //RTE

			ASSERT_IF( clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(proto) == NULL );

			JL_CHKM( host.addCachedClassProto(clasp.name, &clasp, proto), E_CLASS, E_NAME(clasp.name), E_INIT, E_COMMENT("CacheClassProto") );

			ASSERT( host.getCachedClasp(clasp.name) == &clasp );
			ASSERT( host.getCachedProto(clasp.name) == proto );

			ctor = constructor ? JL_GetConstructor(cx, proto) : proto;
		}

		if ( callFuncLink != NULL ) {
		
			clasp.call = callFuncLink->native;
		}
		JL_CHK( funcLink->Register(cx, &proto) );
		JL_CHK( staticFuncLink->Register(cx, &ctor) );
		JL_CHK( propLink->Register(cx, &proto) );
		JL_CHK( staticPropLink->Register(cx, &ctor) );
		JL_CHK( staticConstLink->Register(cx, &ctor) );


		bool isExtensible;
		JL_CHK( JS_IsExtensible(cx, ctor, &isExtensible) );
		if ( isExtensible ) {
		
			JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _sourceId), sourceId, JSPROP_READONLY | JSPROP_PERMANENT) );
			JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _buildDate), buildDate, JSPROP_READONLY | JSPROP_PERMANENT) );
		}

		if ( init ) {

			JL_CHK( init(cx, this, proto, ctor) );
		}

		// doc.Register(cx, ctor); // link the object with the string (using weak maps ?)
	

		return true;
		bad: return false;
	}

private:
	void operator =(const Class &);
	Class(const Class &);
};


bool Unconstructible(JSContext *cx, unsigned, JS::Value *) {

	JL_ERR( E_CLASS, E_NOTCONSTRUCT );
	JL_BAD;
}

enum Defining { defConstructor, defFunction, defStaticFunction, defProperty, defStaticProperty, defCall };

} // namespace jl3::




#define MAKE_UNIQUE(N) \
	JL_CONCAT(N, __COUNTER__)

#define NS_BEGIN \
	namespace MAKE_UNIQUE(__ns) {

#define NS_END \
	;}

#define NEW_NS(N) \
	NS_END namespace N {

#define NEW_NS_UNIQUE \
	NS_END NS_BEGIN



#define DYNAMIC_INITIALIZER_UNIQUE(ID) \
	struct ID { inline ID(); } static const ID; \
	ID::ID()

#define DYNAMIC_INITIALIZER() \
	DYNAMIC_INITIALIZER_UNIQUE(MAKE_UNIQUE(__di))


#define DOC(SUMMARY, ...) \
	static const jl3::Doc MAKE_UNIQUE(__doc)(SUMMARY, ##__VA_ARGS__);


// class

#define CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		static jl3::Class _class( #CLASSNAME ); \
	} \
	bool Register_##CLASSNAME( JSContext *cx, JSObject *obj ) { \
		JS::RootedObject rootedObj(cx, obj); \
		return CLASSNAME::_class.Register(cx, &rootedObj); \
	}; \
	namespace CLASSNAME { \
		NS_BEGIN


#define STATIC_CLASS_NAME _static

#define STATIC_CLASS() \
	namespace STATIC_CLASS_NAME { \
		static jl3::Class _class; \
	} \
	bool Register_##STATIC_CLASS_NAME( JSContext *cx, JSObject *obj ) { \
		JS::RootedObject rootedObj(cx, obj); \
		return STATIC_CLASS_NAME::_class.Register(cx, &rootedObj); \
	}; \
	namespace STATIC_CLASS_NAME { \
		NS_BEGIN

#define CLASS_END \
		NS_END \
	}

#define REGISTER_CLASS(CLASSNAME) \
	JL_MACRO_BEGIN \
	bool Register_##CLASSNAME( JSContext *cx, JSObject *obj ); \
	JL_CHK( Register_##CLASSNAME(cx, obj) ); \
	JL_MACRO_END


#define REGISTER_STATIC() \
	JL_MACRO_BEGIN \
	bool Register_##STATIC_CLASS_NAME( JSContext *cx, JSObject *obj ); \
	JL_CHK( Register_##STATIC_CLASS_NAME(cx, obj) ); \
	JL_MACRO_END


// classs configuration

#define REV(NUMBER) \
	NEW_NS(config) \
	DYNAMIC_INITIALIZER_UNIQUE(__rev) { _class.sourceId = (NUMBER); }

#define FROZEN_PROTO \
	NEW_NS(config) \
	DYNAMIC_INITIALIZER_UNIQUE(__forzenProto) { _class.clasp.flags |= JSCLASS_FREEZE_PROTO; }

#define FROZEN_CTOR \
	NEW_NS(config) \
	DYNAMIC_INITIALIZER_UNIQUE(__frozenCtor) { _class.clasp.flags |= JSCLASS_FREEZE_CTOR; }

#define JL_HAS_PRIVATE \
	NEW_NS(config) \
	DYNAMIC_INITIALIZER_UNIQUE(__hasPrivate) { _class.clasp.flags |= JSCLASS_HAS_PRIVATE; }

#define PRIVATE \
	JL_HAS_PRIVATE \
	struct Private

#define PROTO(CLASSNAME) \
	NEW_NS(config) \
	DYNAMIC_INITIALIZER_UNIQUE(__proto) { _class.parentProtoName = #CLASSNAME; }

#define SLOT(N) \
	NEW_NS(slot) \
	static const jl3::ReservedSlot _slot_##N(_class.reservedSlotCount);


// name

#define NAME(N) \
	static const jl3::NameLink _name_##N(_item.nameLink, #N);

#define NAME_ID(NID) \
	static const jl3::NameLink _name_##NID(_item.nameLink, #NID, NID);


// contants

#define CONSTANT_NAME(N, VALUE) \
	NEW_NS(constant) \
	static const jl3::ConstLink _const_##N(_class.staticConstLink, #N, VALUE);

#define CONSTANT(N) \
	CONSTANT_NAME(N, N);


// properties

#define PROP \
	NEW_NS_UNIQUE \
	static const jl3::Defining _defining = jl3::defProperty; \
	static jl3::PropLink _item(_class.propLink);

#define STATIC_PROP \
	NEW_NS_UNIQUE \
	static const jl3::Defining _defining = jl3::defStaticProperty; \
	static jl3::PropLink _item(_class.staticPropLink);

#define GET() \
	static bool getter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp); \
	DYNAMIC_INITIALIZER_UNIQUE(__getter) { _item.getter = getter; } \
	static bool getter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp)

#define SET() \
	static bool setter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, bool strict, JS::MutableHandle<JS::Value> vp); \
	DYNAMIC_INITIALIZER_UNIQUE(__setter) { _item.setter = setter; } \
	static bool setter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, bool strict, JS::MutableHandle<JS::Value> vp)


// functions

#define JL_FUNCTION(N, ARGCMIN, ARGCMAX) \
	FUNC NAME(N) ARGC(ARGCMIN, ARGCMAX) NATIVE_NAME(N)

#define FUNC \
	NEW_NS_UNIQUE \
	static const jl3::Defining _defining = jl3::defFunction; \
	static jl3::FuncLink _item(_class.funcLink);

#define STATIC_FUNC \
	NEW_NS_UNIQUE \
	static const jl3::Defining _defining = jl3::defStaticFunction; \
	static jl3::FuncLink _item(_class.staticFuncLink);

#define NATIVE() \
	static bool native(JSContext *cx, unsigned argc, JS::Value *vp); \
	DYNAMIC_INITIALIZER_UNIQUE(__native) { _item.native = native; } \
	static bool native(JSContext *cx, unsigned argc, JS::Value *vp)

#define NATIVE_NAME(N) \
	static bool N(JSContext *cx, unsigned argc, JS::Value *vp); \
	DYNAMIC_INITIALIZER_UNIQUE(__native) { _item.native = N; } \
	static bool N(JSContext *cx, unsigned argc, JS::Value *vp)

#define ARGMIN(MIN) \
	DYNAMIC_INITIALIZER_UNIQUE(__argcmin) { _item.argcMin = (MIN); }

#define ARGMAX(MAX) \
	DYNAMIC_INITIALIZER_UNIQUE(__argcmax) { _item.argcMax = (MAX); }

#define ARGC(MIN, MAX) \
	ARGMIN(MIN) \
	ARGMAX(MAX)

// JL_ASSERT_ARG_IS_BOOLEAN(argNum)
// JL_ASSERT_ARG_IS_INTEGER(argNum)
// JL_ASSERT_ARG_IS_INTEGER_NUMBER(argNum)
// JL_ASSERT_ARG_IS_NUMBER(argNum)
// JL_ASSERT_ARG_IS_OBJECT(argNum)
// JL_ASSERT_ARG_IS_OBJECT_OR_NULL(argNum)
// JL_ASSERT_ARG_IS_STRING(argNum)
// JL_ASSERT_ARG_IS_ARRAY(argNum)
// JL_ASSERT_ARG_IS_ARRAYLIKE(argNum)
// JL_ASSERT_ARG_IS_CALLABLE(argNum)
// JL_ASSERT_ARG_TYPE(condition, argNum, typeStr)
// JL_ASSERT_ARG_VAL_RANGE(val, valMin, valMax, argNum)

/* try1
bool ReturnTrue() {
	
	return true;
}

template <class T1 = ReturnTrue, class T2 = ReturnTrue, class T3 = ReturnTrue, class T4 = ReturnTrue>
class Checker {
	bool Check() {
		return t1() && t2() && t3() && t4();
	}
};
*/


/* try2
struct ArgCheckNumberRange {
	const double min;
	const double max;

	ArgCheckNumberRange(double min, double max)
	: min(min), max(max) {
	}

	bool operator()(JSContext *, const JS::CallArgs &args, unsigned index) const {

		JS::MutableHandleValue val = args.handleAt(index);
		return val.isNumber() && val.toNumber() >= min && val.toNumber() <= max;
	}
};

bool ArgCheckBool(JSContext *, const JS::CallArgs &args, unsigned index) {

	return args.handleAt(index).isBoolean();
}


template <class T1>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1) { return t1(cx, args, 0); }

template <class T1, class T2>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1, T2 t2) { return t1(cx, args, 0) && t2(cx, args, 1); }

template <class T1, class T2, class T3>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1, T2 t2, T3 t3) { return t1(cx, args, 0) && t2(cx, args, 1) && t3(cx, args, 2); }

template <class T1, class T2, class T3, class T4>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1, T2 t2, T3 t3, T4 t4) { return t1(cx, args, 0) && t2(cx, args, 1) && t3(cx, args, 2) && t4(cx, args, 3); }

template <class T1, class T2, class T3, class T4, class T5>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) { return t1(cx, args, 0) && t2(cx, args, 1) && t3(cx, args, 2) && t4(cx, args, 3) && t5(cx, args, 4); }

template <class T1, class T2, class T3, class T4, class T5, class T6>
bool ArgCheck(JSContext *cx, const JS::CallArgs &args, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) { return t1(cx, args, 0) && t2(cx, args, 1) && t3(cx, args, 2) && t4(cx, args, 3) && t5(cx, args, 4) && t6(cx, args, 5); }


#define ARGTYPE(...) \
	bool _CheckArgs(JSContext *cx, const JS::CallArgs &args) { \
		return ArgCheck(cx, args, ##__VA_ARGS__); \
	}

// eg. ARGTYPE(ArgCheckBool, ArgCheckNumberRange(1,2))
*/

/* try3
#define TOPT

#define TBOOL \
	(ok = ok && args.handleAt(index++).isBoolean())

ALWAYS_INLINE bool _TRANGE(JSContext *, const JS::CallArgs &args, unsigned index, double min, double max) {

	const JS::MutableHandleValue val = args.handleAt(index);
	return val.isNumber() && val.toNumber() >= min && val.toNumber() <= max;
}

#define TRANGE(MIN, MAX) \
	(ok = ok && _TRANGE(cx, args, index++, MIN, MAX))
*/

/*
// eg.  ARGTYPE(TBOOL TVOID, TRANGE(1,2) TOPT); ... JL_CHK( _CheckArgs(cx, args) );
struct TRANGE {
	double min, max;
	TRANGE(double min, double max)
	: min(min), max(max) {
	}
	bool operator()(JSContext *, const JS::CallArgs &args, unsigned index) {

		const JS::MutableHandleValue val = args.handleAt(index);
		return val.isNumber() && val.toNumber() >= min && val.toNumber() <= max;
	}
};

#define ARGTYPE(...) \
	bool _CheckArgs(JSContext *cx, const JS::CallArgs &args) { \
		unsigned index = 0; \
		bool ok = true; \
		__VA_ARGS__; \
		return ok; \
	}
*/




#define CALL \
	NEW_NS(call) \
	static const jl3::Defining _defining = jl3::defCall; \
	static jl3::FuncLink _item(_class.callFuncLink);


// other callbacks

#define CONSTRUCTOR \
	NEW_NS(constructor) \
	static const jl3::Defining _defining = jl3::defConstructor; \
	static jl3::FuncLink _item(_class.constructor);

#define UNCONSTRUCTABLE \
	DYNAMIC_INITIALIZER() { _item.native = jl3::Unconstructible; }

#define FINALIZE() \
	NEW_NS(config) \
	static void _finalize(JSFreeOp *fop, JSObject *obj); \
	DYNAMIC_INITIALIZER_UNIQUE(__finalize) { _class.clasp.finalize = _finalize; } \
	static void _finalize(JSFreeOp *fop, JSObject *obj)

#define FINALIZE_RET() \
	NEW_NS(config) \
	static void _finalize(JSFreeOp *fop, JSObject *obj); \
	ALWAYS_INLINE int __finalizeWithReturnValue(JSFreeOp *fop, JSObject *obj); \
	DYNAMIC_INITIALIZER_UNIQUE(__finalize) { _class.clasp.finalize = _finalize; } \
	static void _finalize(JSFreeOp *fop, JSObject *obj) { __finalizeWithReturnValue(fop, obj); } \
	ALWAYS_INLINE int __finalizeWithReturnValue(JSFreeOp *fop, JSObject *obj)

#define HAS_INSTANCE() \
	NEW_NS(config) \
	static bool _hasInstance(JSContext *cx, JS::Handle<JSObject*> obj, JS::MutableHandle<JS::Value> vp, bool *bp) \
	DYNAMIC_INITIALIZER_UNIQUE(__hasInstance) { _class.clasp.hasInstance = _hasInstance; } \
	static bool _hasInstance(JSContext *cx, JS::Handle<JSObject*> obj, JS::MutableHandle<JS::Value> vp, bool *bp)

#define ITERATOR() \
	NEW_NS(config) \
	static JSObject* _iteratorObject(JSContext *cx, JS::HandleObject obj, bool keysonly); \
	DYNAMIC_INITIALIZER_UNIQUE(__iteratorObject) { js::Valueify(&_class.clasp)->ext.iteratorObject = _iteratorObject; } \
	static JSObject* _iteratorObject(JSContext *cx, JS::HandleObject obj, bool keysonly) \

#define INIT() \
	NEW_NS(config) \
	static bool _init(JSContext *cx, jl3::Class *sc, JSObject *proto, JSObject *obj); \
	DYNAMIC_INITIALIZER_UNIQUE(__init) { _class.init = _init; } \
	static bool _init(JSContext *cx, jl3::Class *sc, JSObject *proto, JSObject *obj)


/*
#define HAS_ADD_PROPERTY cs.clasp.addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static bool AddProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp)

#define HAS_DEL_PROPERTY cs.clasp.delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static bool DelProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, bool *succeeded)

#define HAS_GET_PROPERTY cs.clasp.getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static bool GetProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp)

#define HAS_SET_PROPERTY cs.clasp.setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static bool SetProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, bool strict, JS::MutableHandle<JS::Value> vp)
*/

// inline tools

#define CHECKS() \
	JL_MACRO_BEGIN \
	if ( _defining == jl3::defFunction || _defining == jl3::defStaticFunction || _defining == jl3::defConstructor || _defining == jl3::defCall ) { \
		JL_ASSERT_ARGC_MIN(_item.argcMin); \
		JL_ASSERT_ARGC_MAX(_item.argcMax); \
	} \
	JL_MACRO_END

#define FUNC_HELPER \
	ASSERT( !JS_IsConstructing(cx, vp) ); \
	JS::CallArgs args; \
	args = JS::CallArgsFromVp(argc, vp); \
	JS::RootedObject obj(cx, &args.computeThis(cx).toObject());

#define SLOT_INDEX(N) \
	(slot::_slot_##N.index)

#undef JL_THIS_CLASS
#define JL_THIS_CLASS \
	(&(_class.clasp))

#undef JL_THIS_CLASS_PROTOTYPE
#define JL_THIS_CLASS_PROTOTYPE \
	(JL_GetCachedProto(jl::Host::getJLHost(cx), JL_THIS_CLASS->name))

