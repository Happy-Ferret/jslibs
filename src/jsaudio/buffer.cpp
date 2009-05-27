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
#include "error.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( OalBuffer )


DEFINE_FINALIZE() {

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	if ( bid )
		alDeleteBuffers(1, &bid);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( soundBlob )
  $H arguments
   $ARG Blob soundBlob:
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_OBJECT( JL_ARG(1) );

	JSObject *blobObj = JSVAL_TO_OBJECT(JL_ARG(1));

	int rate, channels, bits;
	JL_CHK( GetPropertyInt(cx, blobObj, "rate", &rate) );
	JL_CHK( GetPropertyInt(cx, blobObj, "channels", &channels) );
	JL_CHK( GetPropertyInt(cx, blobObj, "bits", &bits) );

	const char *buffer;
	size_t bufferLength;
	jsval tmp = OBJECT_TO_JSVAL(blobObj);
	JsvalToStringAndLength(cx, &tmp, &buffer, &bufferLength); // warning: GC on the returned buffer !

	ALenum format; // The sound data format
	switch (channels) {
		case 1:
			format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
			break;
		case 2:
			format = bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
			break;
		default:
			JL_REPORT_ERROR("Too may channels");
	}

	ALuint bid; // The OpenAL sound buffer ID

	alGenBuffers(1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	alBufferData(bid, format, buffer, bufferLength, rate); // Upload sound data to buffer
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_SetPrivate(cx, obj, (void*)bid) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/*
DEFINE_FUNCTION_FAST( Free ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( bid );
	alBufferData(bid, AL_FORMAT_MONO8, NULL, 0, 0);
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
**/
DEFINE_FUNCTION_FAST( valueOf ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( bid );
	JL_CHK( UIntToJsval(cx, bid, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the frquency (in Hz) of the sound hold by the buffer.
**/
DEFINE_PROPERTY( frequency ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bid );
	ALint frequency;

	alGetBufferi(bid, AL_FREQUENCY, &frequency);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( IntToJsval(cx, frequency, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the size (in bytes) of the sound hold by the buffer.
**/
DEFINE_PROPERTY( size ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bid );
	ALint size;

	alGetBufferi(bid, AL_SIZE, &size);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( IntToJsval(cx, size, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the resolution (in bits) of the sound hold by the buffer.
**/
DEFINE_PROPERTY( bits ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bid );
	ALint bits;

	alGetBufferi(bid, AL_BITS, &bits);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( IntToJsval(cx, bits, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the number of channels of the sound hold by the buffer.
**/
DEFINE_PROPERTY( channels ) {

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( bid );
	ALint channels;

	alGetBufferi(bid, AL_CHANNELS, &channels);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( IntToJsval(cx, channels, vp) );
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
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
