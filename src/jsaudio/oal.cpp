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
//#include "jsobj.h"

#include "oal.h"

//#include "jstransformation.h"

#include "../jslang/blob.h"

//TextureJSClass


//#define _USE_MATH_DEFINES
//#include "math.h"

#include <AL/al.h>
#include <AL/alc.h>

#define LOAD_OPENAL_EXTENSION( name, proto ) \
	static proto name = (proto) alGetProcAddress( #name ); \
	J_S_ASSERT_1( name != NULL, "OpenAL extension %s unavailable.", #name );



/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Oal )

/**doc
=== Static functions ===
**/


/**doc
 * $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alEnable
**/
DEFINE_FUNCTION_FAST( Enable ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	alEnable( JSVAL_TO_INT(J_FARG(1)) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alDisable
**/
DEFINE_FUNCTION_FAST( Disable ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	alDisable( JSVAL_TO_INT(J_FARG(1)) );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alIsEnabled
**/
DEFINE_FUNCTION_FAST( IsEnabled ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	*J_FRVAL = BOOLEAN_TO_JSVAL( alIsEnabled( JSVAL_TO_INT(J_FARG(1)) ) );
	return JS_TRUE;
}


/**doc
 * $BOOL $INAME( pname )
  $H arguments
   $ARG GLenum pname
  $H return value
   value of a selected parameter.
  $H OpenAL API
   alGetString
**/
DEFINE_FUNCTION_FAST( GetString ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	const ALchar* str = alGetString(JSVAL_TO_INT(J_FARG(1)));
	if ( str == NULL ) {

		*J_FRVAL = JSVAL_VOID;
		return JS_TRUE;
	}
	JSString *jsstr = JS_NewStringCopyZ(cx, str);
	J_S_ASSERT_ALLOC( jsstr );
	*J_FRVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}


/**doc
 * $BOOL $INAME( pname )
  $H arguments
   $ARG ALenum pname
  $H return value
   value of a selected parameter.
  $H OpenAL API
   alGetBooleanv
**/
DEFINE_FUNCTION_FAST( GetBoolean ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	ALboolean params;
	alGetBooleanv(JSVAL_TO_INT(J_FARG(1)), &params);
	*J_FRVAL = BOOLEAN_TO_JSVAL(params);
	return JS_TRUE;
}



/**doc
 * $INT | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG ALenum pname
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
  $H return value
   value or values of a selected parameter.
  $H OpenAL API
   alGetIntegerv
**/
DEFINE_FUNCTION_FAST( GetInteger ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	ALint params[16]; // (TBD) check if it is the max amount of data that glGetIntegerv may returns.
	alGetIntegerv(JSVAL_TO_INT( J_FARG(1) ), params);

	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_INT( J_FARG(2) );
		int count = JSVAL_TO_INT( J_FARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		J_S_ASSERT_ALLOC(arrayObj);
		*J_FRVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			tmpValue = INT_TO_JSVAL( params[count] );
			J_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		*J_FRVAL = INT_TO_JSVAL( params[0] );
	}
	return JS_TRUE;
}


/**doc
 * $REAL | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG ALenum pname
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetDoublev
**/
DEFINE_FUNCTION_FAST( GetDouble ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	ALdouble params[16]; // (TBD) check if it is the max amount of data that alGetDoublev may returns.
	alGetDoublev(JSVAL_TO_INT(J_FARG(1)), params);

	if ( J_FARG_ISDEF(2) ) {

		J_S_ASSERT_INT( J_FARG(2) );
		int count = JSVAL_TO_INT( J_FARG(2) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		J_S_ASSERT_ALLOC(arrayObj);
		*J_FRVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			J_CHK( JS_NewDoubleValue(cx, params[count], &tmpValue) );
			J_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		J_CHK( JS_NewDoubleValue(cx, params[0], J_FRVAL) );
	}
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( sound )
  Plays a sound on the default playback device.
  $H arguments
   $ARG soundObject sound: sound object to play.
**/
DEFINE_FUNCTION_FAST( PlaySound ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JSObject *blobObj = JSVAL_TO_OBJECT(J_FARG(1));

	int rate, channels, bits;
	J_CHK( GetPropertyInt(cx, blobObj, "rate", &rate) );
	J_CHK( GetPropertyInt(cx, blobObj, "channels", &channels) );
	J_CHK( GetPropertyInt(cx, blobObj, "bits", &bits) );

	const char *buffer;
	size_t bufferLength;
	jsval tmp = OBJECT_TO_JSVAL(blobObj);
	JsvalToStringAndLength(cx, &tmp, &buffer, &bufferLength); // warning: GC on the returned buffer !
	
	ALint state;                // The state of the sound source
	ALuint bufferID;            // The OpenAL sound buffer ID
	ALuint sourceID;            // The OpenAL sound source
	ALenum format;              // The sound data format

	if (channels == 1)
		format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	else
		format = bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;

	// Create sound buffer and source
	alGenBuffers(1, &bufferID);

  alGenSources(1, &sourceID);

  ALenum err = alGetError(); // 0xA004 = AL_INVALID_OPERATION

  // Set the source and listener to the same location
  alListener3i(AL_POSITION, 0,0,0 );
    
  alSource3i(sourceID, AL_POSITION, 0,0,0 );

  // Upload sound data to buffer
  alBufferData(bufferID, format, buffer, bufferLength, rate);

  
  // Attach sound buffer to source
  alSourcei(sourceID, AL_BUFFER, bufferID);


  // This is a busy wait loop but should be good enough for example purpose
  
  {
  // get the remaining time to play
  ALint offset;
  alGetSourcei(sourceID, AL_SAMPLE_OFFSET, &offset);

  ALint freq, bits, channels, size;
  alGetBufferi(bufferID, AL_FREQUENCY, &freq);
  alGetBufferi(bufferID, AL_BITS, &bits);
  alGetBufferi(bufferID, AL_CHANNELS, &channels);
  alGetBufferi(bufferID, AL_SIZE, &size);

	size_t totalTime = size / (channels * (bits/8) * freq) * 1000;

	// Finally, play the sound!!!
	alSourcePlay(sourceID);

	Sleep(totalTime);

	// Query the state of the souce
	alGetSourcei(sourceID, AL_SOURCE_STATE, &state); // do { } while (state != AL_STOPPED);
  }
		
  // Clean up sound buffer and source
  alDeleteBuffers(1, &bufferID);
  alDeleteSources(1, &sourceID);

//  *J_FRVAL = JSVAL_VOID;

  return JS_TRUE;
}






/**doc
=== Static properties ===
**/

/**doc
 * $INAME $READONLY
  Obtain the most recent error generated in the AL state machine.
**/
DEFINE_PROPERTY(error) {

	*vp = INT_TO_JSVAL(alGetError());
	return JS_TRUE;
}


JSBool Init( JSContext *cx, JSObject *obj ) {

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_INIT

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( NONE                      ,AL_NONE                       )
		CONST_INTEGER( FALSE                     ,AL_FALSE                      )
		CONST_INTEGER( TRUE                      ,AL_TRUE                       )
		CONST_INTEGER( SOURCE_RELATIVE           ,AL_SOURCE_RELATIVE            )
		CONST_INTEGER(	CONE_INNER_ANGLE	  		  ,AL_CONE_INNER_ANGLE				)
		CONST_INTEGER(	CONE_OUTER_ANGLE	  		  ,AL_CONE_OUTER_ANGLE				)
		CONST_INTEGER(	PITCH					  		  ,AL_PITCH								)
		CONST_INTEGER(	POSITION				  		  ,AL_POSITION							)
		CONST_INTEGER(	DIRECTION			  		  ,AL_DIRECTION						)
		CONST_INTEGER(	VELOCITY				  		  ,AL_VELOCITY							)
		CONST_INTEGER(	LOOPING				  		  ,AL_LOOPING							)
		CONST_INTEGER(	BUFFER				  		  ,AL_BUFFER							)
		CONST_INTEGER(	GAIN					  		  ,AL_GAIN								)
		CONST_INTEGER(	MIN_GAIN				  		  ,AL_MIN_GAIN							)
		CONST_INTEGER(	MAX_GAIN				  		  ,AL_MAX_GAIN							)
		CONST_INTEGER(	ORIENTATION			  		  ,AL_ORIENTATION						)
		CONST_INTEGER(	SOURCE_STATE		  		  ,AL_SOURCE_STATE					)
		CONST_INTEGER(	INITIAL				  		  ,AL_INITIAL							)
		CONST_INTEGER(	PLAYING				  		  ,AL_PLAYING							)
		CONST_INTEGER(	PAUSED				  		  ,AL_PAUSED							)
		CONST_INTEGER(	STOPPED				  		  ,AL_STOPPED							)
		CONST_INTEGER(	BUFFERS_QUEUED		  		  ,AL_BUFFERS_QUEUED					)
		CONST_INTEGER(	BUFFERS_PROCESSED	  		  ,AL_BUFFERS_PROCESSED				)
		CONST_INTEGER(	SEC_OFFSET			  		  ,AL_SEC_OFFSET						)
		CONST_INTEGER(	SAMPLE_OFFSET		  		  ,AL_SAMPLE_OFFSET					)
		CONST_INTEGER(	BYTE_OFFSET			  		  ,AL_BYTE_OFFSET						)
		CONST_INTEGER(	SOURCE_TYPE			  		  ,AL_SOURCE_TYPE						)
		CONST_INTEGER(	STATIC				  		  ,AL_STATIC							)
		CONST_INTEGER(	STREAMING			  		  ,AL_STREAMING						)
		CONST_INTEGER(	UNDETERMINED		  		  ,AL_UNDETERMINED					)
		CONST_INTEGER(	FORMAT_MONO8		  		  ,AL_FORMAT_MONO8					)
		CONST_INTEGER(	FORMAT_MONO16		  		  ,AL_FORMAT_MONO16					)
		CONST_INTEGER(	FORMAT_STEREO8		  		  ,AL_FORMAT_STEREO8					)
		CONST_INTEGER(	FORMAT_STEREO16	  		  ,AL_FORMAT_STEREO16				)
		CONST_INTEGER(	REFERENCE_DISTANCE  		  ,AL_REFERENCE_DISTANCE			)
		CONST_INTEGER( ROLLOFF_FACTOR            ,AL_ROLLOFF_FACTOR					)
		CONST_INTEGER(	CONE_OUTER_GAIN			  ,AL_CONE_OUTER_GAIN				)
		CONST_INTEGER(	MAX_DISTANCE				  ,AL_MAX_DISTANCE					)
		CONST_INTEGER(	FREQUENCY					  ,AL_FREQUENCY						)
		CONST_INTEGER(	BITS							  ,AL_BITS								)
		CONST_INTEGER(	CHANNELS						  ,AL_CHANNELS							)
		CONST_INTEGER(	SIZE							  ,AL_SIZE								)
		CONST_INTEGER(	UNUSED						  ,AL_UNUSED							)
		CONST_INTEGER(	PENDING						  ,AL_PENDING							)
		CONST_INTEGER(	PROCESSED					  ,AL_PROCESSED						)
		CONST_INTEGER(	NO_ERROR						  ,AL_NO_ERROR							)
		CONST_INTEGER(	INVALID_NAME				  ,AL_INVALID_NAME					)
		CONST_INTEGER(	INVALID_ENUM				  ,AL_INVALID_ENUM					)
		CONST_INTEGER(	INVALID_VALUE				  ,AL_INVALID_VALUE					)
		CONST_INTEGER(	INVALID_OPERATION			  ,AL_INVALID_OPERATION				)
		CONST_INTEGER(	OUT_OF_MEMORY				  ,AL_OUT_OF_MEMORY					)
		CONST_INTEGER(	VENDOR						  ,AL_VENDOR							)
		CONST_INTEGER(	VERSION						  ,AL_VERSION							)
		CONST_INTEGER(	RENDERER						  ,AL_RENDERER							)
		CONST_INTEGER(	EXTENSIONS					  ,AL_EXTENSIONS						)
		CONST_INTEGER(	DOPPLER_FACTOR				  ,AL_DOPPLER_FACTOR             )
		CONST_INTEGER(	DOPPLER_VELOCITY			  ,AL_DOPPLER_VELOCITY				)
		CONST_INTEGER(	SPEED_OF_SOUND				  ,AL_SPEED_OF_SOUND					)
		CONST_INTEGER(	DISTANCE_MODEL				  ,AL_DISTANCE_MODEL					)
		CONST_INTEGER(	INVERSE_DISTANCE			  ,AL_INVERSE_DISTANCE				)
		CONST_INTEGER(	INVERSE_DISTANCE_CLAMPED  ,AL_INVERSE_DISTANCE_CLAMPED	)
		CONST_INTEGER(	LINEAR_DISTANCE			  ,AL_LINEAR_DISTANCE				)
		CONST_INTEGER(	LINEAR_DISTANCE_CLAMPED	  ,AL_LINEAR_DISTANCE_CLAMPED		)
		CONST_INTEGER(	EXPONENT_DISTANCE			  ,AL_EXPONENT_DISTANCE				)
		CONST_INTEGER(	EXPONENT_DISTANCE_CLAMPED ,AL_EXPONENT_DISTANCE_CLAMPED	)
	END_CONST_INTEGER_SPEC

	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_FAST_ARGC( Enable, 1 )
		FUNCTION_FAST_ARGC( Disable, 1 )
		FUNCTION_FAST_ARGC( IsEnabled, 1 )
		FUNCTION_FAST_ARGC( GetString, 1 )
		FUNCTION_FAST_ARGC( GetBoolean, 1 )
		FUNCTION_FAST_ARGC( GetInteger, 2 )
		FUNCTION_FAST_ARGC( GetDouble, 2 )

		FUNCTION_FAST_ARGC( PlaySound, 1 ) // non-openal API

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)
	END_STATIC_PROPERTY_SPEC

END_CLASS


/*
ogg test files:
	http://xiph.org/vorbis/listen.html

*/
