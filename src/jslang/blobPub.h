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

#define JL_BLOB_LENGTH 0

/*
INLINE
bool
BlobBufferGet( JSContext *cx, JS::HandleObject obj, jl::BufString *str ) {

	JS::RootedValue val(cx);
	JL_CHK( jl::getSlot(cx, obj, JL_BLOB_LENGTH, &val) );
	if ( !val.isUndefined() ) {
		
		ASSERT( val.isInt32() );
		int32_t size = val.toInt32();
		if ( size > 0 ) {
			
			str->get( reinterpret_cast<const uint8_t*>(JL_GetPrivate( obj )), size, false );
		} else {
			
			str->setEmpty();
		}
	} else {

		JL_WARN( E_OBJ, E_STATE, E_INVALID );
		str->setEmpty();
	}
	return true;
	JL_BAD;
}
*/

/*
ALWAYS_INLINE const JSClass*
BlobJSClass( JSContext *cx ) {

	// it's safe to use static keyword because JSClass do not depend on the rt or cx.
	static const JSClass *clasp = NULL;
	if (unlikely( clasp == NULL ))
		clasp = jl::Host::getJLHost(cx).getCachedClasp("Blob");
	return clasp;
}
*/


/*
rules:
	length > 0 && private != NULL => blob
	length == 0 => empty blob
	length == 0 && data == NULL => empty blob
	length == undefined => invalidated blob
	length > 0 && private == NULL => invalid case
	length == undefined && data != NULL => invalid case
*/
INLINE bool
BlobCreate( JSContext *cx, void *ownData, int32_t size, OUT JS::MutableHandleValue rval ) {

	const jl::ClassInfo *classProtoCache = jl::Global::getGlobal(cx)->getCachedClassInfo("Blob");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME( "Blob" ), E_NOTFOUND );

	{

		ASSERT_IF( ownData == nullptr, size == 0 );

		JS::RootedObject blobObj( cx, jl::newObjectWithGivenProto( cx, classProtoCache->clasp, classProtoCache->proto ) );
		//JS::RootedObject blobObj(cx, jl::construct(cx, classProtoCache->proto));

		JL_ASSERT_ALLOC( blobObj );

		//JL_CHK( setBufferGetInterface( cx, blobObj, BlobBufferGet ) );

		rval.setObject( *blobObj );
		JL_CHK( jl::setSlot( cx, blobObj, JL_BLOB_LENGTH, size ) );
		JL_SetPrivate( blobObj, ownData ); // from here, blob owns the data
		
		if ( size )
			JL_updateMallocCounter( cx, size );

	}

	return true;
	JL_BAD;
}


ALWAYS_INLINE jl::BufBase::Type
BlobCreate( JSContext *cx, jl::BufBase &buffer, OUT JS::MutableHandleValue rval ) {

	if ( !buffer.owner() )
		buffer.own(true);
	else
		buffer.maybeCrop();
	if ( !BlobCreate(cx, buffer.data(), buffer.used(), rval) )
		return NULL;
	buffer.dropOwnership();
	return buffer.data();
}


ALWAYS_INLINE jl::BufBase::Type
BlobCreate( JSContext *cx, jl::BufString &buffer, OUT JS::MutableHandleValue rval ) {

	uint8_t *data = buffer.toData<uint8_t*>();
	if ( !BlobCreate(cx, data, buffer.length(), rval) )
		return NULL;
	buffer.dropOwnership();
	return data;
}


ALWAYS_INLINE bool
BlobCreateEmpty( JSContext *cx, OUT JS::MutableHandleValue rval ) {

	return BlobCreate(cx, NULL, 0, rval);
}


ALWAYS_INLINE bool
BlobCreateCopy( JSContext *cx, const void *data, int32_t size, OUT JS::MutableHandleValue rval ) {

	void *ownData = jl_malloc(size);
	JL_ASSERT_ALLOC( ownData );
	jl::memcpy(ownData, data, size);
	JL_CHK( BlobCreate(cx, ownData, size, rval) );
	return true;
	JL_BAD;
}
