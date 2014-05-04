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

#include "blobPub.h"

///////////////////////////////////////////////////////////////////////////////
// Generic Audio object

template <class T, class U, class V, class W>
ALWAYS_INLINE uint8_t* FASTCALL
JL_NewByteAudioObject( JSContext *cx, T bits, U channels, V frames, W rate, OUT JS::MutableHandleValue vp ) {

	ASSERT( bits > 0 && (bits % 8) == 0 && channels > 0 && frames >= 0 && rate > 0 );

	jl::AutoBuffer buffer;

	JS::RootedObject audioObj(cx, JL_NewObj(cx));
	JS::RootedValue dataVal(cx);
	JL_CHK( audioObj );
	vp.setObject(*audioObj);
	//data = JL_NewBuffer(cx, (bits/8) * channels * frames, &dataVal);
	buffer.alloc((bits/8) * channels * frames);
	JL_ASSERT_ALLOC( buffer );
	
	uint8_t *data = BlobCreate(cx, buffer, &dataVal);
	JL_CHK( data );

	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, data), dataVal) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, rate), rate) );
	return data;
	JL_BADVAL(NULL);
}

template <class T, class U, class V, class W>
ALWAYS_INLINE bool FASTCALL
JL_NewByteAudioObjectOwner( JSContext *cx, uint8_t* buffer, T bits, U channels, V frames, W rate, OUT JS::MutableHandleValue vp ) {

	ASSERT_IF( buffer == NULL, frames == 0 );
	ASSERT( bits > 0 && (bits % 8) == 0 && channels > 0 && frames >= 0 && rate > 0 );
	ASSERT_IF( buffer != NULL, jl_msize(buffer) >= (size_t)( (bits/8) * channels * frames ) );

	JS::RootedObject audioObj(cx, JL_NewObj(cx));
	JS::RootedValue dataVal(cx);
	JL_CHK( audioObj );
	vp.setObject(*audioObj);
	//JL_CHK( JL_NewBufferGetOwnership(cx, buffer, (bits/8) * channels * frames, &dataVal) );
	JL_CHK( BlobCreate(cx, buffer, (bits/8) * channels * frames, &dataVal) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, data), dataVal) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, rate), rate) );
	return true;
	JL_BAD;
}

template <class T, class U, class V, class W>
ALWAYS_INLINE bool FASTCALL
JL_NewByteAudioObjectOwner( JSContext *cx, jl::Buffer &buffer, T bits, U channels, V frames, W rate, OUT JS::MutableHandleValue vp ) {

	ASSERT( bits > 0 && (bits % 8) == 0 && channels > 0 && frames >= 0 && rate > 0 );

	JS::RootedObject audioObj(cx, JL_NewObj(cx));
	JS::RootedValue dataVal(cx);
	JL_CHK( buffer.toArrayBufferObject(cx, &dataVal) );
	JL_CHK( audioObj );
	vp.setObject(*audioObj);
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, data), dataVal) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( jl::setProperty(cx, audioObj, JLID(cx, rate), rate) );
	return true;
	JL_BAD;
}



template <class T, class U, class V, class W>
ALWAYS_INLINE JLData FASTCALL
JL_GetByteAudioObject( IN JSContext *cx, IN JS::HandleValue val, T *bits, U *channels, V *frames, W *rate ) {

	JLData data;
	//JL_ASSERT_IS_OBJECT(val, "audio");
	JL_CHK( val.isObject() );
	JS::RootedObject audioObj(cx, &val.toObject());
	JL_CHK( jl::getProperty(cx, audioObj, JLID(cx, data), &data) );
	JL_CHK( jl::getProperty(cx, audioObj, JLID(cx, bits), bits) );
	JL_CHK( jl::getProperty(cx, audioObj, JLID(cx, channels), channels) );
	JL_CHK( jl::getProperty(cx, audioObj, JLID(cx, frames), frames) );
	JL_CHK( jl::getProperty(cx, audioObj, JLID(cx, rate), rate) );

	//JL_ASSERT( *bits > 0 && (*bits % 8) == 0 && *rate > 0 && *channels > 0 && *frames >= 0, E_STR("audio"), E_FORMAT );
	//JL_ASSERT( data.IsSet() && data.Length() == (size_t)( (*bits/8) * *channels * *frames ), E_DATASIZE, E_INVALID );
	JL_CHK( *bits > 0 && (*bits % 8) == 0 && *rate > 0 && *channels > 0 && *frames >= 0 );
	JL_CHK( data.IsSet() && data.Length() == (size_t)( (*bits/8) * *channels * *frames ) );
	return data;
bad:
	return JLData();
}

