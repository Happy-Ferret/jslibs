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

#define JL_HANDLE_TYPE uint32_t
#define JL_HANDLE_INVALID (0)

#define JLHID(id) \
	jl::CastCStrToUint32(#id)

/*
ALWAYS_INLINE
void
JL_HIDToStr(JL_HANDLE_TYPE hid, char *dst) {
}
*/


class HandlePrivate : public jl::CppAllocators {

	JS::Heap<JS::Value> _slots[JL_HANDLE_PUBLIC_SLOT_COUNT];
	JS::Heap<JS::Value> *_dynSlots;
	uint32_t _dynSlotsCount;
public:

	void
	trace(JSTracer *trc, JSObject *obj) {

		uint32_t i;

		// in that case, you need your JS::Value vector to be JS::Heap<JS::Value>, and to call JS_CallHeapValueTracer on each item in the vector

		//doc: The argument to JS_Call*Tracer is an in-out param: when the function returns, the garbage collector might have moved the GC thing

		for ( i = 0; i < JL_HANDLE_PUBLIC_SLOT_COUNT; ++i ) {
			
			if ( _slots[i].isMarkable() ) {
				
				JS_CallHeapValueTracer(trc, &_slots[i], "HandlePrivate slot");
			}
		}

		for ( i = 0; i < dynSlotsCount(); ++i ) {

			if ( _dynSlots[i].isMarkable() ) {
				
				JS_CallHeapValueTracer(trc, &_dynSlots[i], "HandlePrivate dynSlot");
			}
		}
	}


	virtual JL_HANDLE_TYPE typeId() const = 0;
	virtual ~HandlePrivate() {

		if ( _dynSlots )
			delete [] _dynSlots;
	};

	HandlePrivate() {

		_dynSlotsCount = 0;
		_dynSlots = nullptr;
	}

	JS::Value
	getSlot(uint32_t index) const {

		ASSERT( index < JL_HANDLE_PUBLIC_SLOT_COUNT );
		JS::ExposeValueToActiveJS(_slots[index]);
		return _slots[index];
	}

	void
	setSlot(uint32_t index, JS::HandleValue val) {

		ASSERT( index < JL_HANDLE_PUBLIC_SLOT_COUNT );
		_slots[index].set(val);
	}


	bool
	allocDynSlots(uint32_t count) {

		if ( _dynSlotsCount != count ) {
			
			delete [] _dynSlots;
			_dynSlots = new JS::Heap<JS::Value>[count];
			_dynSlotsCount = count;
		}
		return _dynSlots != 0;
	}

	uint32_t
	dynSlotsCount() const {
		
		return _dynSlotsCount;
	}

	JS::Value
	getDynSlot(uint32_t index) const {

		ASSERT( _dynSlots && index < _dynSlotsCount );
		JS::ExposeValueToActiveJS(_dynSlots[index]);
		return _dynSlots[index];
	}

	void
	setDynSlot(uint32_t index, JS::HandleValue val) {

		ASSERT( _dynSlots && index < _dynSlotsCount );
		_dynSlots[index].set(val);
	}
};


ALWAYS_INLINE const JSClass*
JL_HandleJSClass( JSContext *cx ) {

	// it's safe to use static keyword because JSClass do not depend on the rt or cx.
	static const JSClass *clasp = NULL;
	if (unlikely( clasp == NULL ))
		clasp = jl::Host::getJLHost(cx).getCachedClasp("Handle");
	return clasp;
}


template <class T>
INLINE bool
HandleCreate( JSContext *cx, T *data, OUT JS::MutableHandleValue handleVal ) {

	const jl::ClassInfo *classProtoCache = jl::Host::getJLHost(cx).getCachedClassInfo("Handle");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME("Handle"), E_NOTFOUND );

	{
		JS::RootedObject handleObj(cx, jl::newObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto));
		JL_ASSERT_ALLOC( handleObj );
		handleVal.setObject(*handleObj);
		HandlePrivate *pv = data;
		JL_SetPrivate(handleObj, pv);
		JL_updateMallocCounter(cx, sizeof(T));
	}

	return true;
	JL_BAD;
}


INLINE bool
HandleClose( JSContext *cx, IN JS::HandleValue handleVal ) {
	
	JS::RootedObject handleObj(cx, handleVal.toObjectOrNull());
	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );

	HandlePrivate *pv;
	pv = static_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	delete pv;
	JL_SetPrivate(handleObj, NULL);

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
		pv = static_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
		JL_CHK( pv );
		return pv->typeId();
	}

bad:
	return JL_HANDLE_INVALID;
}


// IsHandle

ALWAYS_INLINE bool
IsHandle( JSContext *cx, JS::HandleValue handleVal ) {

	return jl::isClass(cx, handleVal, JL_HandleJSClass(cx));
}

ALWAYS_INLINE bool
IsHandle( JSContext *cx, JS::HandleObject handleObj ) {

	return jl::isClass(cx, handleObj, JL_HandleJSClass(cx));
}


// IsHandleType

ALWAYS_INLINE bool
IsHandleType( JSContext *cx, IN JS::HandleObject handleObj, JL_HANDLE_TYPE handleType ) {

	if ( !jl::isClass(cx, handleObj, JL_HandleJSClass(cx)) )
		return false;
	HandlePrivate *pv;
	pv = static_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	return pv != NULL && pv->typeId() == handleType;
}

ALWAYS_INLINE bool
IsHandleType( JSContext *cx, IN JS::HandleValue handleVal, JL_HANDLE_TYPE handleType ) {

	if ( !jl::isClass(cx, handleVal, JL_HandleJSClass(cx)) )
		return false;
	JS::RootedObject handleObj(cx, &handleVal.toObject());
	return IsHandleType(cx, handleObj, handleType);
}


// GetHandlePrivate

template <class T>
INLINE bool
GetHandlePrivate( JSContext *cx, IN JS::HandleObject handleObj, T *&data ) {

	JL_ASSERT_INSTANCE( handleObj, JL_HandleJSClass(cx) );
	HandlePrivate *pv;
	pv = static_cast<HandlePrivate*>(JL_GetPrivate(handleObj));
	data = static_cast<T*>(pv);
	return true;
	JL_BAD;
}

template <class T>
INLINE bool
GetHandlePrivate( JSContext *cx, IN JS::HandleValue handleVal, T *&data ) {

	JL_ASSERT_INSTANCE( handleVal, JL_HandleJSClass(cx) );
	HandlePrivate *pv;
	pv = static_cast<HandlePrivate*>(JL_GetPrivate(handleVal));
	data = static_cast<T*>(pv);
	return true;
	JL_BAD;
}


// SetHandleSlot

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


// GetHandleSlot

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
