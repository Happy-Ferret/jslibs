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


struct ConstLink {
	ConstLink *_prev;
	const char *name;
	const JS::Value value;

	ConstLink(ConstLink *&last, const char *name, const JS::Value &value)
	: _prev(last), name(name), value(value) {
		
		last = this;
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		for ( ConstLink *it = this; it; it = it->_prev ) {

			JL_CHK( JS_DefineProperty(cx, obj, it->name, it->value, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
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

	NameLink(NameLink *&last, const char *name, int8_t tinyid = noTinyId )
	: _prev(last), name(name), tinyid(tinyid) {

		last = this;
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

		for ( PropLink *it = this; it; it = it->_prev ) {

			unsigned attrs = JSPROP_PERMANENT | JSPROP_SHARED | ( it->setter ? 0 : JSPROP_READONLY ); // https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes
			for ( NameLink *nameIt = it->nameLink; nameIt; nameIt = nameIt->_prev ) {
				
				if ( nameIt->tinyid == NameLink::noTinyId )
					JL_CHK( JS_DefineProperty(cx, obj, nameIt->name, JSVAL_VOID, it->getter, it->setter, attrs) );
				else
					JL_CHK( JS_DefinePropertyWithTinyId(cx, obj, nameIt->name, nameIt->tinyid, JSVAL_VOID, it->getter, it->setter, attrs) );
			}
		}
		return true;
		bad: return false;
	}

private:
	void operator =(const PropLink &);
	PropLink(const PropLink &);
};


struct FuncLink {
	FuncLink *_prev;

	const JSNative native;
	const char *name;
	const unsigned argcMin;
	const unsigned argcMax;

	FuncLink(FuncLink *&last, JSNative native, const char *name = NULL, unsigned argcMin = 0, unsigned argcMax = 4)
	: _prev(last), native(native), name(name), argcMin(argcMin), argcMax(argcMin > argcMax ? argcMin : argcMax) {

		last = this;
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		for ( FuncLink *it = this; it; it = it->_prev ) {

			JL_CHK( JS_DefineFunction(cx, obj, it->name, it->native, it->argcMax, 0) );
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
	FuncLink *funcLink;
	FuncLink *staticFuncLink;
	PropLink *propLink;
	PropLink *staticPropLink;
	ConstLink *staticConstLink;
	uint32_t reservedSlotCount;
	ClassInit_t init;
	const double buildDate;
	SourceId_t sourceId;

	Class(const char *className = NULL)
	: constructor(NULL), funcLink(NULL), staticFuncLink(NULL), propLink(NULL), staticPropLink(NULL), staticConstLink(NULL), reservedSlotCount(0), init(NULL), buildDate((double)__DATE__EPOCH * 1000), sourceId(0) {

		clasp.addProperty = JS_PropertyStub;
		clasp.delProperty = JS_DeletePropertyStub;
		clasp.getProperty = JS_PropertyStub;
		clasp.setProperty = JS_StrictPropertyStub;
		clasp.enumerate = JS_EnumerateStub;
		clasp.resolve = JS_ResolveStub;
		clasp.convert = JS_ConvertStub;

		clasp.name = className;
		clasp.flags = 0;
	}

	bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

		JS::RootedObject proto(cx); // doc: object that is the prototype for the newly initialized class.
		JS::RootedObject ctor(cx);

		if ( clasp.name == NULL ) { // static class

			ASSERT( parentProtoName == NULL );
			ASSERT( reservedSlotCount == 0 );
			ASSERT( constructor == NULL );
			ASSERT( clasp.flags == 0 );
			ASSERT( clasp.finalize == NULL );

			proto = obj;
			ctor = obj;
		} else {

			ASSERT( clasp.name && clasp.name[0] ); // Invalid class name.

			HostPrivate *hpv;
			hpv = JL_GetHostPrivate(cx);

			JSObject *parentProto;
			if ( parentProtoName != NULL ) {

				parentProto = JL_GetCachedProto(hpv, parentProtoName);
				JL_CHKM( parentProto != NULL, E_STR(parentProtoName), E_STR("prototype"), E_NOTFOUND );
			} else {

				parentProto = NULL;
			}

			clasp.flags |= JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount);
			ASSERT( JSCLASS_RESERVED_SLOTS(&clasp) == reservedSlotCount );

			proto = JS_InitClass(cx, obj, parentProto, &clasp, constructor ? constructor->native : NULL, constructor ? constructor->argcMax : 0, NULL, NULL, NULL, NULL);

			JL_ASSERT( proto != NULL, E_CLASS, E_NAME(clasp.name), E_CREATE ); //RTE

			ASSERT_IF( clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(proto) == NULL );

			JL_CHKM( JL_CacheClassProto(hpv, clasp.name, &clasp, proto), E_CLASS, E_NAME(clasp.name), E_INIT, E_COMMENT("CacheClassProto") );

			ASSERT( JL_GetCachedClass(hpv, clasp.name) == &clasp );
			ASSERT( JL_GetCachedProto(hpv, clasp.name) == proto );

			ctor = constructor ? JL_GetConstructor(cx, proto) : proto;
		}


		JL_CHK( funcLink->Register(cx, &proto) );
		JL_CHK( staticFuncLink->Register(cx, &ctor) );
		JL_CHK( propLink->Register(cx, &proto) );
		JL_CHK( staticPropLink->Register(cx, &ctor) );
		JL_CHK( staticConstLink->Register(cx, &ctor) );

		if ( JS_IsExtensible(ctor) ) {
		
			JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _sourceId), JS::NumberValue(sourceId), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
			JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _buildDate), JS::NumberValue(buildDate), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
		}

		if ( init )
			JL_CHK( init(cx, this, proto, ctor) );

		return true;
		bad: return false;
	}

private:
	void operator =(const Class &);
	Class(const Class &);
};


JSBool Unconstructible(JSContext *cx, unsigned argc, JS::Value *vp) {

	JL_ERR( E_CLASS, E_NOTCONSTRUCT );
	JL_BAD;
}


} // namespace jl3::



#define INITIALIZER_CALL_UNIQUE(ID) \
	struct ID { ID(); } const ID; \
	ID::ID()

#define INITIALIZER_CALL() \
	INITIALIZER_CALL_UNIQUE(JL_CONCAT(__initcall, __COUNTER__))


#define CLASS(CLASSNAME, ...) \
	namespace CLASSNAME { \
		static jl3::Class _class( #CLASSNAME ); \
	} \
	bool Register_##CLASSNAME( JSContext *cx, JSObject *obj ) { \
		JS::RootedObject rootedObj(cx, obj); \
		return CLASSNAME::_class.Register(cx, &rootedObj, ##__VA_ARGS__); \
	}; \
	namespace CLASSNAME


#define STATIC_CLASS(...) \
	namespace _static { \
		static jl3::Class _class; \
	} \
	bool Register__static( JSContext *cx, JSObject *obj ) { \
		JS::RootedObject rootedObj(cx, obj); \
		return _static::_class.Register(cx, &rootedObj, ##__VA_ARGS__); \
	}; \
	namespace _static


#define REGISTER_CLASS(CLASSNAME) \
	bool Register_##CLASSNAME( JSContext *cx, JSObject *obj ); \
	JL_CHK( Register_##CLASSNAME(cx, obj) );


#define REGISTER_STATIC() \
	bool Register__static( JSContext *cx, JSObject *obj ); \
	JL_CHK( Register__static(cx, obj) );


#define REV(NUMBER) \
	INITIALIZER_CALL_UNIQUE(__rev) { _class.sourceId = (NUMBER); }

#define FROZEN_PROTO \
	INITIALIZER_CALL_UNIQUE(__forzenProto) { _class.clasp.flags |= JSCLASS_FREEZE_PROTO; }

#define FROZEN_CTOR \
	INITIALIZER_CALL_UNIQUE(__frozenCtor) { _class.clasp.flags |= JSCLASS_FREEZE_CTOR; }

#define JL_HAS_PRIVATE \
	INITIALIZER_CALL_UNIQUE(__hasPrivate) { _class.clasp.flags |= JSCLASS_HAS_PRIVATE; }

#define PROTO(CLASSNAME) \
	INITIALIZER_CALL_UNIQUE(__proto) { _class.parentProtoName = #CLASSNAME; }

#define SLOT(NAME) \
	const jl3::ReservedSlot _slot_##NAME(_class.reservedSlotCount);

#define CONSTANT_NAME(NAME, VALUE, ...) \
	const jl3::ConstLink _const_##NAME(_class.staticConstLink, #NAME, JS::NumberValue(VALUE), ##__VA_ARGS__);

#define CONSTANT(NAME, ...) \
	const jl3::ConstLink _const_##NAME(_class.staticConstLink, #NAME, JS::NumberValue(NAME), ##__VA_ARGS__);

#define NAME(N, ...) \
	const jl3::NameLink _name_##N(_item.nameLink, #N, ##__VA_ARGS__);

#define NAME_ID(NID, ...) \
	const jl3::NameLink _name_##NID(_item.nameLink, #NID, NID, ##__VA_ARGS__);


#define __PROP(IDENTIFIER) \
	namespace IDENTIFIER { \
		jl3::PropLink _item(_class.propLink); \
	}; \
	namespace IDENTIFIER

#define PROP \
	__PROP(JL_CONCAT(prop, __COUNTER__))


#define __STATIC_PROP(IDENTIFIER) \
	namespace IDENTIFIER { \
		jl3::PropLink _item(_class.staticPropLink); \
	}; \
	namespace IDENTIFIER


#define STATIC_PROP \
	__STATIC_PROP(JL_CONCAT(prop, __COUNTER__))


#define PROP_END \
	}


#define GET(...) \
	static JSBool getter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp); \
	INITIALIZER_CALL_UNIQUE(__getter) { _item.getter = getter; } \
	static JSBool getter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp, ##__VA_ARGS__)

#define SET(...) \
	static JSBool setter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JSBool strict, JS::MutableHandle<JS::Value> vp); \
	INITIALIZER_CALL_UNIQUE(__setter) { _item.setter = setter; } \
	static JSBool setter(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JSBool strict, JS::MutableHandle<JS::Value> vp, ##__VA_ARGS__)

#define FUNC(NAME, ...) \
	static JSBool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl3::FuncLink NAME##_(_class.funcLink, _##NAME, #NAME, ##__VA_ARGS__); \
	static JSBool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp)

#define STATIC_FUNC(NAME, ...) \
	static JSBool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl3::FuncLink NAME##_(_class.staticFuncLink, _##NAME, #NAME, ##__VA_ARGS__); \
	static JSBool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp)


#define MAKE_UNCONSTRUCTIBLE \
	const jl3::FuncLink constructor_(_class.constructor, jl3::Unconstructible, NULL);

#define CONSTRUCTOR(...) \
	static JSBool _constructor(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl3::FuncLink constructor_(_class.constructor, _constructor, NULL, ##__VA_ARGS__); \
	static JSBool _constructor(JSContext *cx, unsigned argc, JS::Value *vp)

#define FINALIZE() \
	static void _finalize(JSFreeOp *fop, JSObject *obj); \
	INITIALIZER_CALL_UNIQUE(__finalize) { _class.clasp.finalize = _finalize; } \
	static void _finalize(JSFreeOp *fop, JSObject *obj)

#define FINALIZE_RET() \
	static void _finalize(JSFreeOp *fop, JSObject *obj); \
	ALWAYS_INLINE int __finalizeWithReturnValue(JSFreeOp *fop, JSObject *obj); \
	INITIALIZER_CALL_UNIQUE(__finalize) { _class.clasp.finalize = _finalize; } \
	static void _finalize(JSFreeOp *fop, JSObject *obj) { __finalizeWithReturnValue(fop, obj); } \
	ALWAYS_INLINE int __finalizeWithReturnValue(JSFreeOp *fop, JSObject *obj)

#define CALL(...) \
	static JSBool _call(JSContext *cx, unsigned argc, JS::Value *vp); \
	INITIALIZER_CALL_UNIQUE(__call) { _class.clasp.call = _call; } \
	static JSBool _call(JSContext *cx, unsigned argc, JS::Value *vp, ##__VA_ARGS__)

#define HAS_INSTANCE() \
	static JSBool _hasInstance(JSContext *cx, JS::Handle<JSObject*> obj, JS::MutableHandle<JS::Value> vp, JSBool *bp) \
	INITIALIZER_CALL_UNIQUE(__hasInstance) { _class.clasp.hasInstance = _hasInstance; } \
	static JSBool _hasInstance(JSContext *cx, JS::Handle<JSObject*> obj, JS::MutableHandle<JS::Value> vp, JSBool *bp)

#define ITERATOR() \
	static JSObject* _iteratorObject(JSContext *cx, JS::HandleObject obj, JSBool keysonly); \
	INITIALIZER_CALL_UNIQUE(__iteratorObject) { js::Valueify(&_class.clasp)->ext.iteratorObject = _iteratorObject; } \
	static JSObject* _iteratorObject(JSContext *cx, JS::HandleObject obj, JSBool keysonly)

#define INIT() \
	static bool _init(JSContext *cx, jl3::Class *sc, JSObject *proto, JSObject *obj); \
	INITIALIZER_CALL_UNIQUE(__init) { _class.init = _init; } \
	static bool _init(JSContext *cx, jl3::Class *sc, JSObject *proto, JSObject *obj)


#define DOC(TEXT) \
	static _pendingDoc = TEXT;

/*
#define HAS_ADD_PROPERTY cs.clasp.addProperty = AddProperty;
#define DEFINE_ADD_PROPERTY() static JSBool AddProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp)

#define HAS_DEL_PROPERTY cs.clasp.delProperty = DelProperty;
#define DEFINE_DEL_PROPERTY() static JSBool DelProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JSBool *succeeded)

#define HAS_GET_PROPERTY cs.clasp.getProperty = GetProperty;
#define DEFINE_GET_PROPERTY() static JSBool GetProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JS::MutableHandle<JS::Value> vp)

#define HAS_SET_PROPERTY cs.clasp.setProperty = SetProperty;
#define DEFINE_SET_PROPERTY() static JSBool SetProperty(JSContext *cx, JS::Handle<JSObject*> obj, JS::Handle<jsid> id, JSBool strict, JS::MutableHandle<JS::Value> vp)
*/


#define FUNC_HELPER \
	ASSERT( !JS_IsConstructing(cx, vp) ); \
	JS::CallArgs args; \
	args = JS::CallArgsFromVp(argc, vp); \
	JS::RootedObject obj(cx, &args.computeThis(cx).toObject());


// tools

#define SLOT_INDEX(NAME) \
	(_slot_##NAME.index)

