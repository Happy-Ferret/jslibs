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

#include <AL/al.h>
#include <AL/alc.h>

BEGIN_CLASS( OalBuffer )


DEFINE_FINALIZE() {

	ALuint bid = (ALuint) JS_GetPrivate(cx, obj);
	if ( bid )
		alDeleteBuffers(1, &bid);
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_ARG(1) );

	JSObject *blobObj = JSVAL_TO_OBJECT(J_ARG(1));

	int rate, channels, bits;
	J_CHK( GetPropertyInt(cx, blobObj, "rate", &rate) );
	J_CHK( GetPropertyInt(cx, blobObj, "channels", &channels) );
	J_CHK( GetPropertyInt(cx, blobObj, "bits", &bits) );

	const char *buffer;
	size_t bufferLength;
	jsval tmp = OBJECT_TO_JSVAL(blobObj);
	JsvalToStringAndLength(cx, &tmp, &buffer, &bufferLength); // warning: GC on the returned buffer !

	ALenum format; // The sound data format
	if ( channels == 1 )
		format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	else
		format = bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
	
	// (TBD) report an error for other unsuported formats.

	ALuint bid; // The OpenAL sound buffer ID
	alGenBuffers(1, &bid);
	alBufferData(bid, format, buffer, bufferLength, rate); // Upload sound data to buffer
	ALenum err = alGetError(); // 0xA004 (= AL_INVALID_OPERATION
	// (TBD) throw exception ?

	J_CHK( JS_SetPrivate(cx, obj, (void*)bid) );
	return JS_TRUE;
}

/*
DEFINE_FUNCTION_FAST( Free ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( bid );
	alBufferData(bid, AL_FORMAT_MONO8, NULL, 0, 0);
	return JS_TRUE;
}
*/

DEFINE_FUNCTION_FAST( valueOf ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( bid );
	J_CHK( UIntToJsval(cx, bid, J_FRVAL) );
	return JS_TRUE;
}



DEFINE_PROPERTY( frequency ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( bid );
	ALint frequency;
	alGetBufferi(bid, AL_FREQUENCY, &frequency);
	J_CHK( IntToJsval(cx, frequency, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( size ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( bid );
	ALint size;
	alGetBufferi(bid, AL_SIZE, &size);
	J_CHK( IntToJsval(cx, size, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( bits ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( bid );
	ALint bits;
	alGetBufferi(bid, AL_BITS, &bits);
	J_CHK( IntToJsval(cx, bits, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( channels ) {

	ALuint bid = (ALuint) JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( bid );
	ALint channels;
	alGetBufferi(bid, AL_CHANNELS, &channels);
	J_CHK( IntToJsval(cx, channels, vp) );
	return JS_TRUE;
}



CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( frequency )
		PROPERTY_READ( size )
		PROPERTY_READ( bits )
		PROPERTY_READ( channels )
	END_PROPERTY_SPEC

END_CLASS
