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


ALWAYS_INLINE const JSClass*
BlobJSClass( JSContext *cx ) {

	// it's safe to use static keyword because JSClass do not depend on the rt or cx.
	static const JSClass *clasp = NULL;
	if (unlikely( clasp == NULL ))
		clasp = jl::Host::getHost(cx).getCachedClasp("Blob");
	return clasp;
}

INLINE bool
BlobCreate( JSContext *cx, void *ownData, int32_t size, OUT JS::MutableHandleValue rval ) {

	const jl::ProtoCache::Item *classProtoCache = jl::Host::getHost(cx).getCachedClassProto("Blob");
	JL_ASSERT( classProtoCache != NULL, E_CLASS, E_NAME("Blob"), E_NOTFOUND );

	{

	// length > 0 && private != NULL => blob
	// length == 0 => empty blob
	// length == undefined => invalidated blob
	// length > 0 && private == NULL => invalid case
	// length == undefined && data != NULL => invalid case

	ASSERT_IF( !ownData, size == 0 );

	JS::RootedObject blobObj(cx, jl::newObjectWithGivenProto(cx, classProtoCache->clasp, classProtoCache->proto));
	//JS::RootedObject blobObj(cx, jl::construct(cx, classProtoCache->proto));

	JL_CHK( blobObj );
	rval.setObject(*blobObj);
	JL_SetPrivate(blobObj, ownData);
	JL_CHK( jl::setSlot(cx, blobObj, JL_BLOB_LENGTH, size) );
	JL_updateMallocCounter(cx, size);
	}

	return true;
	JL_BAD;
}


ALWAYS_INLINE jl::Buffer::DataType
BlobCreate( JSContext *cx, jl::Buffer &buffer, OUT JS::MutableHandleValue rval ) {

	size_t size = buffer.getSize();
	jl::Buffer::DataType data = buffer.stealData();
	if ( !BlobCreate(cx, data, size, rval) )
		return NULL;
	return data;
}

ALWAYS_INLINE bool
BlobCreateEmpty( JSContext *cx, OUT JS::MutableHandleValue rval ) {

	return BlobCreate(cx, NULL, 0, rval);
}

ALWAYS_INLINE bool
BlobCreateCopy( JSContext *cx, const void *data, int32_t size, OUT JS::MutableHandleValue rval ) {

	void *ownData;
	IFDEBUG( ownData = NULL );
	jl::memcpy(ownData, data, size);
	return BlobCreate(cx, ownData, size, rval);
}
