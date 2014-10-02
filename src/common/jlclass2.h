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


namespace jl2 { //JL_BEGIN_NAMESPACE


	// When it comes to DLLs, static initialization and the call to DllMain is bracketed by an internal critical section, so they are thread-safe.
	// A second thread will wait until the first is done before it loads the DLL.
	// DllMain is called by the Windows loader while holding an internal critical section known as the "loader lock," so your static constructors will
	// be called during the DLL_PROCESS_ATTACH event, which only occurs once, when your DLL is first loaded.

	struct ClassSpec;

	typedef int32_t SourceId_t;
	typedef bool (*ClassInit_t)(JSContext *cx, ClassSpec *sc, JS::HandleObject proto, JS::HandleObject obj);


	// const

	struct ConstItem {

		ConstItem *prev_;
		const char *name_;
		const JS::Value value;

		ConstItem(ConstItem *&last, const char *name, const JS::Value &value)
		:
		prev_(last),
		name_(name),
		value(value) {

			last = this;
		}

		bool RegisterAll( JSContext *cx, JS::MutableHandleObject obj ) {

			for ( ConstItem *it = this; it; it = it->prev_ ) {

				JL_CHK( JS_DefineProperty(cx, obj, it->name_, it->value, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
			}
			return true;
			bad: return false;
		}

	private:
		void operator =(const ConstItem &);
	};


	// functions

	struct FunctionItem {

		FunctionItem *prev_;
		const JSNative native_;
		const char *name_;
		const unsigned argcMin;
		const unsigned argcMax;

		FunctionItem(FunctionItem *&last, JSNative native, const char *name = NULL, unsigned argcMin = 0, unsigned argcMax = 4)
		:
		prev_(last),
		native_(native),
		name_(name),
		argcMin(argcMin),
		argcMax(argcMin > argcMax ? argcMin : argcMax) {

			last = this;
		}

		bool RegisterAll( JSContext *cx, JS::MutableHandleObject obj ) {

			for ( FunctionItem *it = this; it; it = it->prev_ ) {

				JL_CHK( JS_DefineFunction(cx, obj, it->name_, it->native_, it->argcMax, 0) );
			}
			return true;
			bad: return false;
		}

	private:
		void operator =(const FunctionItem &);
	};


	// properties

	struct PropertyItem {

		static const JSPropertyOp getter;
		static const JSStrictPropertyOp setter;

		PropertyItem *prev_;
		const JSPropertyOp getter_;
		const JSStrictPropertyOp setter_;
		const char *name_;
		const unsigned attrs_; // https://developer.mozilla.org/en-US/docs/SpiderMonkey/JSAPI_Reference/JS_GetPropertyAttributes

		PropertyItem(PropertyItem *&last, JSPropertyOp getter, JSStrictPropertyOp setter, const char *name)
		:
		prev_(last),
		getter_(getter),
		setter_(setter),
		name_(name),
		attrs_(JSPROP_READONLY | JSPROP_PERMANENT) {

			last = this;
		}

		bool RegisterAll( JSContext *cx, JS::MutableHandleObject obj ) {

			for ( PropertyItem *it = this; it; it = it->prev_ ) {

				JL_CHK( JS_DefineProperty(cx, obj, it->name_, JS::UndefinedHandleValue(), it->getter_, it->setter_, it->attrs_) );
			}
			return true;
			bad: return false;
		}

	private:
		void operator =(const PropertyItem &);
	};

	const JSPropertyOp PropertyItem::getter = NULL;
	const JSStrictPropertyOp PropertyItem::setter = NULL;


	// reserved slot

	struct ReservedSlot {
		
		const int index;
		ReservedSlot(int &reservedSlotCount)
		: 
		index(reservedSlotCount++) {}

	private:
		void operator =(const ReservedSlot &);
	};


	// ClassSpec

	struct ClassSpec {

		JSClass clasp;
		const char *parentProtoName;
		FunctionItem *constructor;
		FunctionItem *functionItem;
		FunctionItem *staticFunctionItem;
		PropertyItem *propertyItem;
		PropertyItem *staticPropertyItem;
		ConstItem *staticConstItem;
		int reservedSlotCount;
		ClassInit_t init;
		const double buildDate;
		SourceId_t sourceId;


		ClassSpec(const char *className = NULL)
		:
		parentProtoName(NULL),
		constructor(NULL),
		functionItem(NULL),
		staticFunctionItem(NULL),
		propertyItem(NULL),
		staticPropertyItem(NULL),
		reservedSlotCount(0),
		init(NULL),
		buildDate((double)__DATE__EPOCH * 1000) {

			clasp.addProperty = JS_PropertyStub;
			clasp.delProperty = JS_DeletePropertyStub;
			clasp.getProperty = JS_PropertyStub;
			clasp.setProperty = JS_StrictPropertyStub;
			clasp.enumerate = JS_EnumerateStub;
			clasp.resolve = JS_ResolveStub;
			clasp.convert = JS_ConvertStub;

			clasp.name = className;
		}

		bool Register( JSContext *cx, JS::MutableHandleObject obj ) {

			if ( clasp.name == NULL ) { // static class

				ASSERT( parentProtoName == NULL );
				ASSERT( clasp.flags == 0 );
				ASSERT( constructor == NULL );
				ASSERT( reservedSlotCount == 0 );

				if ( !functionItem->RegisterAll(cx, obj) )
					return false;

				if ( !staticFunctionItem->RegisterAll(cx, obj) )
					return false;

				if ( !propertyItem->RegisterAll(cx, obj) )
					return false;

				if ( !staticPropertyItem->RegisterAll(cx, obj) )
					return false;

				if ( !staticConstItem->RegisterAll(cx, obj) )
					return false;

				if ( JS_IsExtensible(obj) ) {
	
					JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _sourceId), JS::NumberValue(sourceId), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
					JL_CHK( JS_DefinePropertyById(cx, obj, JLID(cx, _buildDate), JS::NumberValue(buildDate), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
				}

				if ( init )
					JL_CHK( init(cx, this, NULL, obj) );

			} else {

				JS::RootedObject proto(cx); // doc: object that is the prototype for the newly initialized class.
				JS::RootedObject ctor(cx);

				ASSERT( clasp.name && clasp.name[0] ); // Invalid class name.

				HostPrivate *hpv;
				hpv = jl::Host::getJLHost(cx);

				JSObject *parentProto;
				if ( parentProtoName != NULL ) {

					parentProto = JL_GetCachedProto(hpv, parentProtoName);
					JL_CHKM( parentProto != NULL, E_STR(parentProtoName), E_STR("prototype"), E_NOTFOUND );
				} else {

					parentProto = NULL;
				}

				clasp.flags |= JSCLASS_HAS_RESERVED_SLOTS(reservedSlotCount);

				proto = JS_InitClass(cx, obj, parentProto, &clasp, constructor ? constructor->native_ : NULL, constructor ? constructor->argcMax : 0, NULL, NULL, NULL, NULL);

				JL_ASSERT( proto != NULL, E_CLASS, E_NAME(clasp.name), E_CREATE ); //RTE
				ASSERT_IF( clasp.flags & JSCLASS_HAS_PRIVATE, JL_GetPrivate(proto) == NULL );

				JL_CHKM( JL_CacheClassInfo(hpv, clasp.name, &clasp, proto), E_CLASS, E_NAME(clasp.name), E_INIT, E_COMMENT("ClassInfoCache") );

				if ( constructor ) {

					ctor = JL_GetConstructor(cx, proto);
				} else {

					ctor = proto;
				}

				if ( !functionItem->RegisterAll(cx, &proto) )
					return false;

				if ( !staticFunctionItem->RegisterAll(cx, &ctor) )
					return false;

				if ( !propertyItem->RegisterAll(cx, &proto) )
					return false;

				if ( !staticPropertyItem->RegisterAll(cx, &ctor) )
					return false;

				if ( !staticConstItem->RegisterAll(cx, &ctor) )
					return false;

				// info
				if ( JS_IsExtensible(ctor) ) {
		
					JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _sourceId), JS::NumberValue(sourceId), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
					JL_CHK( JS_DefinePropertyById(cx, ctor, JLID(cx, _buildDate), JS::NumberValue(buildDate), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );
				}

				if ( init )
					JL_CHK( init(cx, this, proto, ctor) );

				ASSERT( hpv.getCachedClasp(clasp.name) == &clasp );
				ASSERT( hpv.getCachedClasp(clasp.name) == proto );
			}

			return true;
			bad: return false;
		}
	};

} //JL_END_NAMESPACE



#define JL_INITIALIZER_CALL(VAR) \
	struct __##VAR { __##VAR(); } const __##VAR; \
	__##VAR::__##VAR()


#define CLASS(CLASSNAME) \
	namespace CLASSNAME { \
		static jl2::ClassSpec _classSpec( #CLASSNAME );

#define STATIC_CLASS() \
	namespace _static { \
		static jl2::ClassSpec _classSpec;

#define CLASS_END \
	}


#define JL_CONST(NAME, VAL) \
	const jl2::ConstItem _const_##NAME(_classSpec.staticConstItem, #NAME, JS::NumberValue(VAL));

#define JL_CONST_SINGLE(NAME) \
	const jl2::ConstItem _const_##NAME(_classSpec.staticConstItem, #NAME, JS::NumberValue(NAME));


#define JL_HAS_PRIVATE \
	JL_INITIALIZER_CALL(hasPrivate) { _classSpec.clasp.flags |= JSCLASS_HAS_PRIVATE; }


#define JL_PROTOTYPE(CLASSNAME) \
	JL_INITIALIZER_CALL(prototype) { _classSpec.parentProtoName = #CLASSNAME; }


#define JL_SLOT(NAME) \
	const jl2::ReservedSlot _slot_##NAME(_classSpec.reservedSlotCount);

#define JL_INIT() \
	static bool _init(JSContext *cx, jl2::ClassSpec *sc, JS::HandleObject proto, JS::HandleObject obj); \
	struct init_ { init_(jl2::ClassInit_t init) { _classSpec.init = init; } } const init_(_init); \
	static bool _init(JSContext *cx, jl2::ClassSpec *sc, JS::HandleObject proto, JSObject *obj)

#define JL_CONSTRUCTOR(...) \
	static bool _constructor(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl2::FunctionItem constructor_(_classSpec.constructor, _constructor, NULL, ##__VA_ARGS__); \
	static bool _constructor(JSContext *cx, unsigned argc, JS::Value *vp)



#define JL_FUNCTION(NAME, ...) \
	static bool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl2::FunctionItem NAME##_(_classSpec.functionItem, _##NAME, #NAME, ##__VA_ARGS__); \
	static bool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp)

#define JL_STATIC_FUNCTION(NAME, ...) \
	static bool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp); \
	const jl2::FunctionItem NAME##_(_classSpec.staticFunctionItem, _##NAME, #NAME, ##__VA_ARGS__); \
	static bool _##NAME(JSContext *cx, unsigned argc, JS::Value *vp)

/*
#define DEF_FUNCTION_START \
	ASSERT( !JS_IsConstructing(cx, vp) ); \
	JS::CallArgs args = JS::CallArgsFromVp(argc, vp); \
	JS::RootedObject obj(cx, &args.computeThis(cx).toObject());
*/



#define JL_PROPERTY(NAME, ...) \
	namespace _property_##NAME { \
		struct Prop : jl2::PropertyItem { \
			Prop() : jl2::PropertyItem(_classSpec.propertyItem, getter, setter, #NAME, ##__VA_ARGS__) {}

#define JL_STATIC_PROPERTY(NAME, ...) \
	namespace _property_##NAME { \
		struct Prop : jl2::PropertyItem { \
			Prop() : jl2::PropertyItem(_classSpec.staticPropertyItem, getter, setter, #NAME, ##__VA_ARGS__) {}

#define JL_PROPERTY_END \
		} const prop; \
	} // namespace


#define JL_GETTER() \
	static bool getter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)

#define JL_SETTER() \
	static bool setter(JSContext *cx, JS::HandleObject obj, JS::HandleId id, bool strict, JS::MutableHandleValue vp)


