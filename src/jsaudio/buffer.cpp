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

	JLStr buffer;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	JSObject *blobObj = JSVAL_TO_OBJECT(JL_ARG(1));

	int rate, channels, bits;
	JL_CHK( JL_GetProperty(cx, blobObj, "rate", &rate) );
	JL_CHK( JL_GetProperty(cx, blobObj, "channels", &channels) );
	JL_CHK( JL_GetProperty(cx, blobObj, "bits", &bits) );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &buffer) );

	ALenum format; // The sound data format
	switch (channels) {
		case 1:
			format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
			break;
		case 2:
			format = bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
			break;
		default:
			JL_ERR( E_PARAM, E_STR("channels"), E_RANGE, E_INTERVAL_NUM(1, 2) );
	}

	ALuint bid; // The OpenAL sound buffer ID

	alGenBuffers(1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	alBufferData(bid, format, buffer.GetConstStr(), (ALsizei)buffer.Length(), rate); // Upload sound data to buffer
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_SetPrivate(cx, obj, (void*)bid);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/*
DEFINE_FUNCTION( Free ) {

	JL_DEFINE_FUNCTION_OBJ;
	ALuint bid = (ALuint) JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	alBufferData(bid, AL_FORMAT_MONO8, NULL, 0, 0);
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
**/
DEFINE_FUNCTION( valueOf ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	ALuint bid = (ALuint) JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	JL_CHK( JL_NativeToJsval(cx, bid, JL_RVAL) );
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
DEFINE_PROPERTY_GETTER( frequency ) {

	JL_ASSERT_THIS_INSTANCE();

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	ALint frequency;

	alGetBufferi(bid, AL_FREQUENCY, &frequency);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeToJsval(cx, frequency, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the size (in bytes) of the sound hold by the buffer.
**/
DEFINE_PROPERTY_GETTER( size ) {

	JL_ASSERT_THIS_INSTANCE();

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	ALint size;

	alGetBufferi(bid, AL_SIZE, &size);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeToJsval(cx, size, vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the resolution (in bits) of the sound hold by the buffer.
**/
DEFINE_PROPERTY_GETTER( bits ) {

	JL_ASSERT_THIS_INSTANCE();

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	ALint bits;

	alGetBufferi(bid, AL_BITS, &bits);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeToJsval(cx, bits, vp) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the number of channels of the sound hold by the buffer.
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_ASSERT_THIS_INSTANCE();

	ALuint bid = (ALuint) JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( bid );
	ALint channels;

	alGetBufferi(bid, AL_CHANNELS, &channels);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeToJsval(cx, channels, vp) );
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GET( frequency )
		PROPERTY_GET( size )
		PROPERTY_GET( bits )
		PROPERTY_GET( channels )
	END_PROPERTY_SPEC

END_CLASS
