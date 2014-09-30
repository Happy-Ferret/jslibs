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
#include "handlePub.h"
#include <js/TracingAPI.h>

// the aim of *globalKey* is to ensure that a pointer in the process's virtual memory space can be serialized and unserializes safely.
static uint32_t globalKey = 0;

BEGIN_CLASS( Handle )

DEFINE_FINALIZE() { // see HandleClose()

	// is there a simple way to detect that we are finalizing the prototype of an object ?
	HandlePrivate *pv = static_cast<HandlePrivate*>(JL_GetPrivateFromFinalize(obj));
	if ( pv ) {

		if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
			pv->HandlePrivate::~HandlePrivate();
		else
			delete pv; // see FreeOp::get(fop)->delete_()
	}
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;
	JL_IGNORE( argc );
	JL_ASSERT_THIS_INSTANCE();

	HandlePrivate *pv = JL_HasPrivate(JL_OBJ) ? (HandlePrivate*)JL_GetPrivate(JL_OBJ) : NULL; // the prototype has no ptivate slot

	ASSERT( !pv == (JL_OBJ == JL_THIS_CLASS_PROTOTYPE) );

	char str[] = "[Handle \0\0\0\0\0";
	if ( pv != NULL ) {

		str[8 + jl::CastUint32ToCStr(pv->typeId(), str+8)] = ']';
	} else {

		// this handle host.stdout( Handle.prototype )
		str[7] = ']';
	}
	JL_CHK( jl::setValue( cx, JL_RVAL, jl::CStrSpec( str ) ) );
	return true;
	JL_BAD;
}


DEFINE_TRACER() {
	
	HandlePrivate *pv = static_cast<HandlePrivate*>(js::GetObjectPrivate(obj));
	if ( pv )
		pv->trace(trc, obj);
}


DEFINE_INIT() {

	JL_IGNORE(cx, cs, proto, obj);
	JL_SAFE( globalKey = jl::SessionId() );
	return true;
}

/*
DEFINE_FUNCTION( _serialize ) {

		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	ser->Write(cx, globalKey);

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	uint32_t gKey;
	unser->Read(cx, gKey);
	JL_CHKM( gKey == globalKey, E_THISOPERATION, E_INVALID );

	return true;
	JL_BAD;
}
*/

/**qa
	
//	QA.ASSERT_NOEXCEPTION( function() { Handle.prototype.xxx = 123; } );
//	QA.ASSERTOP( Handle.prototype.xxx, '!=', 123, 'FROZEN_PROTOTYPE' );

	QA.ASSERTOP( new Date(Handle._buildDate + new Date().getTimezoneOffset() * 60 * 1000), '<=', Date.now(), 'build date validity' );
	QA.ASSERTOP( Handle._buildDate, '>', 0, 'build date validity' );

	var handle = timeoutEvents(100);

	QA.ASSERTOP( handle, 'instanceof', Handle);

**/
CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_INIT

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(JL_HANDLE_PUBLIC_SLOT_COUNT)
	HAS_FINALIZE
	HAS_TRACER
	
	IS_UNCONSTRUCTIBLE

	BEGIN_FUNCTION_SPEC

		FUNCTION( toString )
/*
		FUNCTION( _serialize )
		FUNCTION( _unserialize )
*/
	END_FUNCTION_SPEC

END_CLASS


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////




/*

#define JL_HANDLE2_PUBLIC_SLOT_COUNT 4


class Handle2Private : public jl::CppAllocators {
public:
	virtual const char* typeId() const = 0;
	virtual ~Handle2Private() {};
};

BEGIN_CLASS( Handle2 )


DEFINE_FINALIZE() {

	Handle2Private *pv = static_cast<Handle2Private*>(js::GetObjectPrivate(obj));
	if ( pv ) {
		
		delete pv;
	}
}

CONFIGURE_CLASS
	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_PRIVATE
	HAS_FINALIZE
	HAS_RESERVED_SLOTS(JL_HANDLE2_PUBLIC_SLOT_COUNT)
	IS_UNCONSTRUCTIBLE
END_CLASS


// HandleCreate( JSContext *cx, const JL_HANDLE_TYPE handleType, Struct **userStruct, HandleFinalizeCallback_t finalizeCallback, OUT JS::MutableHandleValue handleVal ) {

ALWAYS_INLINE const JSClass*
JL_Handle2JSClass( JSContext *cx ) {

	//static const JSClass *clasp = NULL; // it's safe to use static keyword because JSClass do not depend on the rt or cx.
	//if (unlikely( clasp == NULL ))
	//	clasp = jl::Host::getJLHost(cx).getCachedClasp("Handle");  //clasp = JL_GetCachedClass(JL_GetHostPrivate(cx), "Handle");
	//return clasp;

	return &Handle2::classSpec->clasp;
}


void handletest(JSContext *cx) {

	class MyHandleData : public Handle2Private {
	public:
		const char *typeId(void) const {

			return "abc";
		}

		~MyHandleData() {

		}
		int test;
	};

	JS::RootedValue rval(cx);

	MyHandleData *data = new MyHandleData();
	data->test = 987;

	Handle2Create(cx, data, &rval);

	JS::RootedObject rvalObj(cx, &rval.toObject());

	MyHandleData *tmp;
	GetHandle2Private(cx, rvalObj, tmp);
}
*/



/*

template <class UserClass>
UserClass*
NewHandle2(JSContext *cx, jsval *rval) {

//	uint32_t handleType = Type;

	const ClassProtoCache *classProtoCache = JL_GetCachedClassInfo(JL_GetHostPrivate(cx), "Handle2");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME("Handle2"), E_NOTFOUND );

	JSObject *handleObj;
	handleObj = jl::newObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto, NULL);
	JL_CHK( handleObj );
	*rval = OBJECT_TO_JSVAL(handleObj);

	UserClass* userData = ::new(jl_malloc(sizeof(UserClass))) UserClass;
	JL_SetPrivate( handleObj, userData);
	return userData;
bad:
	return NULL;
}


int test() {

	class Foo {
	};

	Foo *foo = NewHandle2<Foo>(NULL, NULL);

	return 0;
}

*/







