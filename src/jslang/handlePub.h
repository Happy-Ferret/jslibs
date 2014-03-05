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

#include "stdafx.h"

#define JL_HANDLE_PUBLIC_SLOT_COUNT 4

#define JLHID(id) \
	jl::CastCStrToUint32(#id)

#define JL_HANDLE_TYPE uint32_t
#define JL_HANDLE_INVALID (0)


class HandlePrivate : public jl::CppAllocators {
public:
	virtual JL_HANDLE_TYPE typeId() const = 0;
	virtual ~HandlePrivate() {};
};


ALWAYS_INLINE const JSClass*
JL_HandleJSClass( JSContext *cx ) {

	// it's safe to use static keyword because JSClass do not depend on the rt or cx.
	static const JSClass *clasp = NULL;
	if (unlikely( clasp == NULL ))
		clasp = jl::Host::getHost(cx).getCachedClasp("Handle");
	return clasp;
}


template <class T>
INLINE bool
HandleCreate( JSContext *cx, T *data, OUT JS::MutableHandleValue handleVal ) {

	HandlePrivate *pv = data;

	const jl::ProtoCache::Item *classProtoCache = jl::Host::getHost(cx).getCachedClassProto("Handle");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME("Handle"), E_NOTFOUND );

	{
	JS::RootedObject handleObj(cx, JL_NewObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto));
	JL_CHK( handleObj );
	handleVal.setObject(*handleObj);
	JL_SetPrivate(handleObj, pv);
	}

	return true;
	JL_BAD;
}


INLINE bool
HandleClose( JSContext *cx, IN JS::HandleValue handleVal ) {
	
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	
	{
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );

	HandlePrivate *pv;
	pv = reinterpret_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	delete pv;
	JL_SetPrivate(handleObj, NULL);
	}

	return true;
	JL_BAD;
}




INLINE JL_HANDLE_TYPE
GetHandleType( JSContext *cx, JS::HandleValue handleVal ) {

	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");

	{
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );

	HandlePrivate *pv;
	pv = reinterpret_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	JL_CHK( pv );
	return pv->typeId();
	}

bad:
	return JL_HANDLE_INVALID;
}


ALWAYS_INLINE bool
IsHandle( JSContext *cx, JS::HandleValue handleVal ) {

	return JL_ValueIsClass(cx, handleVal, JL_HandleJSClass(cx));
}

ALWAYS_INLINE bool
IsHandle( JSContext *cx, JS::HandleObject handleObj ) {

	return JL_ObjectIsClass(handleObj, JL_HandleJSClass(cx));
}


ALWAYS_INLINE bool
IsHandleType( JSContext *cx, IN JS::HandleObject handleObj, JL_HANDLE_TYPE handleType ) {

	if ( !JL_ObjectIsClass(handleObj, JL_HandleJSClass(cx)) )
		return false;
	HandlePrivate *pv;
	pv = reinterpret_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	return pv != NULL && pv->typeId() == handleType;
}


ALWAYS_INLINE bool
IsHandleType( JSContext *cx, IN JS::HandleValue handleVal, JL_HANDLE_TYPE handleType ) {

	if ( !JL_ValueIsClass(cx, handleVal, JL_HandleJSClass(cx)) )
		return false;
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	return IsHandleType(cx, handleObj, handleType);
}



template <class T>
INLINE bool
GetHandlePrivate( JSContext *cx, IN JS::HandleObject handleObj, T *&data ) {

	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );
	HandlePrivate *pv;
	pv = reinterpret_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	data = static_cast<T*>(pv);
	return true;
	JL_BAD;
}


template <class T>
INLINE bool
GetHandlePrivate( JSContext *cx, IN JS::HandleValue handleVal, T *&data ) {

	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );
	HandlePrivate *pv;
	pv = reinterpret_cast<HandlePrivate*>(JL_GetPrivate(handleVal));
	data = static_cast<T*>(pv);
	return true;
	JL_BAD;
}


INLINE bool
SetHandleSlot( JSContext *cx, JS::HandleObject handleObj, uint32_t slotIndex, IN JS::HandleValue val ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );
	return JL_SetReservedSlot(handleObj, slotIndex, val);
	JL_BAD;
}

INLINE bool
SetHandleSlot( JSContext *cx, JS::HandleValue handleVal, uint32_t slotIndex, IN JS::HandleValue val ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");
	
	{
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	return SetHandleSlot(cx, handleObj, slotIndex, val);
	}
	
	JL_BAD;
}


INLINE bool
GetHandleSlot( JSContext *cx, JS::HandleObject handleObj, uint32_t slotIndex, OUT JS::MutableHandleValue value ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );
	return JL_GetReservedSlot(handleObj, slotIndex, value);
	JL_BAD;
}

INLINE bool
GetHandleSlot( JSContext *cx, JS::HandleValue handleVal, uint32_t slotIndex, OUT JS::MutableHandleValue value ) {

	ASSERT( slotIndex < JL_HANDLE_PUBLIC_SLOT_COUNT );
	JL_ASSERT_IS_OBJECT(handleVal, "(handle)");

	{
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	return GetHandleSlot(cx, handleObj, slotIndex, value);
	}

	JL_BAD;
}
