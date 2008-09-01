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
#include <../../efx.h>
#include <../../EFX-Util.h>


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
 * $VOID $INAME( [ deviceName ] )
  $H arguments
   $ARG string deviceName
  $H OpenAL API
   alcOpenDevice, alcCreateContext, alcMakeContextCurrent
**/
DEFINE_FUNCTION_FAST( Open ) {

	// Initialize the OpenAL library (cf. alutInit)

	const char *deviceName;
	if ( J_FARG_ISDEF(1) )
		J_CHK( JsvalToString(cx, &J_FARG(1), &deviceName) );
	else
		deviceName = NULL;

	// Doc: alcOpenDevice() open the Device specified. Current options are:
	//   "Generic Hardware"
	//   "Generic Software"
	//   "DirectSound3D" (for legacy)
	//   "DirectSound"
	//   "MMSYSTEM"
	// If no device name is specified, we will attempt to use DS3D.
	ALCdevice *device = alcOpenDevice (deviceName);
	if (device == NULL)
		J_REPORT_ERROR("ALUT_ERROR_OPEN_DEVICE");
	ALCcontext *context = alcCreateContext (device, NULL);
	if (context == NULL) {
		alcCloseDevice (device);
		J_REPORT_ERROR("ALUT_ERROR_CREATE_CONTEXT");
	}
	if (!alcMakeContextCurrent(context)) {

		alcDestroyContext (context);
		alcCloseDevice (device);
		J_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");
	}

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME()
  $H OpenAL API
   alcGetCurrentContext, alcMakeContextCurrent, alcGetContextsDevice, alcDestroyContext, alcCloseDevice
**/
DEFINE_FUNCTION_FAST( Close ) {

	// cf. alutExit
	ALCcontext *context = alcGetCurrentContext();
	if ( context == NULL )
		J_REPORT_ERROR("Unable to get the current context.");
	ALCdevice *device;
	if (!alcMakeContextCurrent (NULL))
		J_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");
	device = alcGetContextsDevice (context);
	if (alcGetError (device) != ALC_NO_ERROR )
		J_REPORT_ERROR("ALUT_ERROR_ALC_ERROR_ON_ENTRY");
	alcDestroyContext (context);
	if (alcGetError (device) != ALC_NO_ERROR)
		J_REPORT_ERROR("ALUT_ERROR_DESTROY_CONTEXT");
	if (!alcCloseDevice (device))
		J_REPORT_ERROR("ALUT_ERROR_CLOSE_DEVICE");

	if (!alcMakeContextCurrent(NULL)) { // (TBD) check this

		J_REPORT_ERROR("ALUT_ERROR_MAKE_CONTEXT_CURRENT");
	}

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( value )
  $H arguments
   $ARG Number value
  $H OpenAL API
   alDopplerFactor
**/
DEFINE_FUNCTION_FAST( DopplerFactor ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	float value;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &value) );
	alDopplerFactor( value );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( value )
  $H arguments
   $ARG Number value
  $H OpenAL API
   alDopplerVelocity
**/
DEFINE_FUNCTION_FAST( DopplerVelocity ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	float value;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &value) );
	alDopplerVelocity( value );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( value )
  $H arguments
   $ARG Number value
  $H OpenAL API
   alSpeedOfSound
**/
DEFINE_FUNCTION_FAST( SpeedOfSound ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	float value;
	J_CHK( JsvalToFloat(cx, J_FARG(1), &value) );
	alSpeedOfSound( value );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( distanceModel )
  $H arguments
   $ARG Integer value
  $H OpenAL API
   alDistanceModel
**/
DEFINE_FUNCTION_FAST( DistanceModel ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));
	unsigned int distanceModel;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &distanceModel) );
	alDistanceModel( distanceModel );
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


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

	ALint params[16]; // (TBD) check if it is the max amount of data that alGetIntegerv may returns.
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
 * $VOID $INAME( pname, params )
  $H arguments
   $ARG ALenum pname:
   $ARG Array params: 
  $H OpenAL API
   alListeneri, alListenerf, alListenerfv
**/
DEFINE_FUNCTION_FAST( Listener ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_INT(J_FARG(1));

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(2)) ) {

		alListeneri( JSVAL_TO_INT( J_FARG(1) ), JSVAL_TO_INT( J_FARG(2) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(2)) ) {

		jsdouble param;
		J_CHK( JS_ValueToNumber(cx, J_FARG(2), &param) );
		alListenerf( JSVAL_TO_INT( J_FARG(1) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, J_FARG(2)) ) {

		ALfloat params[16];
		size_t length;
//		J_JSVAL_TO_REAL_VECTOR( J_FARG(2), params, length );
		J_CHK( JsvalToFloatVector(cx, J_FARG(2), params, 16, &length) );
		alListenerfv( JSVAL_TO_INT(J_FARG(1)), params );
		return JS_TRUE;
	}

	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


/**doc
 * $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG integer source:
   $ARG ALenum pname:
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetListenerfv
**/
DEFINE_FUNCTION_FAST( GetListenerReal ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	ALfloat params[16]; // (TBD) check if it is the max amount of data that alGetSourcef may returns.
	alGetListenerfv(JSVAL_TO_INT(J_FARG(1)), params);

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
 * $INT $INAME()
  $H OpenAL API
   alSourcei, alSourcef, alSourcefv
**/
DEFINE_FUNCTION_FAST( GenSource ) {

	ALuint sourceID; // The OpenAL sound source
	alGenSources(1, &sourceID);
	J_CHK( UIntToJsval(cx, sourceID, J_FRVAL) );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source, pname, params )
  $H arguments
   $ARG integer source:
   $ARG ALenum pname:
   $ARG Array params: 
  $H OpenAL API
   alSourcei, alSourcef, alSourcefv
**/
DEFINE_FUNCTION_FAST( Source ) {

	J_S_ASSERT_ARG_MIN(3);
	J_S_ASSERT_NUMBER(J_FARG(1));
	J_S_ASSERT_INT(J_FARG(2));

	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );

	*J_FRVAL = JSVAL_VOID;
	if ( JSVAL_IS_INT(J_FARG(3)) ) {

		alSourcei( sid, JSVAL_TO_INT( J_FARG(2) ), JSVAL_TO_INT( J_FARG(3) ) );
		return JS_TRUE;
	}
	if ( JSVAL_IS_DOUBLE(J_FARG(3)) ) {

		jsdouble param;
		J_CHK( JS_ValueToNumber(cx, J_FARG(3), &param) );
		alSourcef( sid, JSVAL_TO_INT( J_FARG(2) ), param );
		return JS_TRUE;
	}
	if ( JsvalIsArray(cx, J_FARG(3)) ) {

		ALfloat params[16];
		size_t length;
		J_CHK( JsvalToFloatVector(cx, J_FARG(3), params, COUNTOF(params), &length ) );
		alSourcefv( sid, JSVAL_TO_INT(J_FARG(2)), params );
		return JS_TRUE;
	}
	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


/**doc
 * $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG integer source:
   $ARG ALenum pname:
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetSourcef
**/
DEFINE_FUNCTION_FAST( GetSourceReal ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_NUMBER(J_FARG(1));
	J_S_ASSERT_INT(J_FARG(2));

	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );

	ALfloat params[16]; // (TBD) check if it is the max amount of data that alGetSourcef may returns.

	ALenum pname = JSVAL_TO_INT(J_FARG(2));
	alGetSourcef(sid, pname, params);

	int err = alGetError();

	if ( J_FARG_ISDEF(3) ) {

		J_S_ASSERT_INT( J_FARG(3) );
		int count = JSVAL_TO_INT( J_FARG(3) );
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
 * $REAL | $ARRAY $INAME( source, pname [, count] )
**/
DEFINE_FUNCTION_FAST( GetSourceInteger ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_NUMBER(J_FARG(1));
	J_S_ASSERT_INT(J_FARG(2));

	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );

	ALint params[16]; // (TBD) check if it is the max amount of data that alGetSourcef may returns.

	ALenum pname = JSVAL_TO_INT(J_FARG(2));
	alGetSourcei(sid, pname, params);

	int err = alGetError();

	if ( J_FARG_ISDEF(3) ) {

		J_S_ASSERT_INT( J_FARG(3) );
		int count = JSVAL_TO_INT( J_FARG(3) );
		JSObject *arrayObj = JS_NewArrayObject(cx, 0, NULL);
		J_S_ASSERT_ALLOC(arrayObj);
		*J_FRVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			J_CHK( IntToJsval(cx, params[count], &tmpValue) );
			J_CHK( JS_SetElement(cx, arrayObj, count, &tmpValue) );
		}
	} else {

		J_CHK( JS_NewDoubleValue(cx, params[0], J_FRVAL) );
	}
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( source )
  $H arguments
   $ARG integer source: the source id.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( DeleteSource ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );
	alDeleteSources(1, &sid);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source, buffer | bufferArray )
  $H arguments
   $ARG integer source: the source id.
   $ARG integer buffer: the buffer id.
   $ARG Array bufferArray: an Array of buffer id.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( SourceQueueBuffers ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );

	if ( JSVAL_IS_INT(J_FARG(2)) ) {
		
		ALuint buffer;
		J_CHK( JsvalToUInt(cx, J_FARG(2), &buffer) );
		alSourceQueueBuffers( sid, 1, &buffer );
		ALenum err = alGetError();
		return JS_TRUE;
	}

	if ( JsvalIsArray(cx, J_FARG(2)) ) {

		ALuint params[1024];
 		unsigned int length = sizeof(params)/sizeof(*params);
//		J_JSVAL_TO_INT_VECTOR( J_FARG(2), params, length );
		J_CHK( JsvalToUIntVector(cx, J_FARG(2), params, sizeof(params)/sizeof(*params), &length) );
		alSourceQueueBuffers( sid, length, params );
		ALenum err = alGetError();
		return JS_TRUE;
	}

	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( source, buffer | bufferArray )
  $H arguments
   $ARG integer buffer: the buffer id.
   $ARG Array bufferArray: an Array of buffer id.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( SourceUnqueueBuffers ) {

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );

	if ( JSVAL_IS_INT(J_FARG(2)) ) {
		
		ALuint buffer;
		J_CHK( JsvalToUInt(cx, J_FARG(2), &buffer) );
		alSourceUnqueueBuffers( sid, 1, &buffer );
		return JS_TRUE;
	}

	if ( JsvalIsArray(cx, J_FARG(2)) ) {

		ALuint params[1024];
		unsigned int length;

//		J_JSVAL_TO_INT_VECTOR( J_FARG(2), params, length );
		J_CHK( JsvalToUIntVector(cx, J_FARG(2), params, sizeof(params)/sizeof(*params), &length) );

		alSourceUnqueueBuffers( sid, length, params );
		return JS_TRUE;
	}

	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}



/**doc
 * $INT $INAME( soundObject )
  Creates a new buffer and attach a sound data to it. The data comming from the soundObject is copied into the OpenAL system.
  $note
   Buffers containing audio data with more than one channel will be played without 3D spatialization features � these formats are normally used for background music.
  $H arguments
   $ARG soundObject sound: a sound object that contains PCM audio data and the following properties: rate, channels and bits.
**/
DEFINE_FUNCTION_FAST( Buffer ) {

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

	ALuint bufferID; // The OpenAL sound buffer ID
	alGenBuffers(1, &bufferID);

	ALenum format; // The sound data format
	if (channels == 1)
		format = bits == 16 ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	else
		format = bits == 16 ? AL_FORMAT_STEREO16 : AL_FORMAT_STEREO8;
	
	// (TBD) report an error for other unsuported formats.

	// Upload sound data to buffer
	alBufferData(bufferID, format, buffer, bufferLength, rate);
	ALenum err = alGetError(); // 0xA004 (= AL_INVALID_OPERATION
	// (TBD) throw exception ?

	J_CHK( UIntToJsval(cx, bufferID, J_FRVAL) );
	return JS_TRUE;
}



/**doc
 * $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG integer source:
   $ARG ALenum pname:
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetBufferfv
**/
DEFINE_FUNCTION_FAST( GetBufferReal ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	ALfloat params[16]; // (TBD) check if it is the max amount of data that alGetSourcef may returns.
	alGetBufferfv(JSVAL_TO_INT(J_FARG(1)), JSVAL_TO_INT(J_FARG(2)), params);

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
 * $INT | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG integer source:
   $ARG ALenum pname:
   $ARG integer count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetBufferiv
**/
DEFINE_FUNCTION_FAST( GetBufferInteger ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	ALint params[16]; // (TBD) check if it is the max amount of data that alGetSourcef may returns.
	alGetBufferiv(JSVAL_TO_INT(J_FARG(1)), JSVAL_TO_INT(J_FARG(2)), params);

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
 * $VOID $INAME( buffer )
  $H arguments
   $ARG integer buffer: the buffer id.
  $H note
   Buffers that have been unqueued from all sources are UNUSED. Buffers that are UNUSED can be deleted, or changed by alBufferData commands.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( DeleteBuffer ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint bufferId;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &bufferId ) );
	alBufferData(bufferId, 0, NULL, 0, 0); // (TBD) needed ?
	alDeleteBuffers(1, &bufferId);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source )
  Plays the given source.
  $H arguments
   $ARG integer source: the ID of the source to play.
**/
DEFINE_FUNCTION_FAST( PlaySource ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );
	alSourcePlay(sid);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source )
  Stop the given source.
  $H arguments
   $ARG integer source: the ID of the source to play.
**/
DEFINE_FUNCTION_FAST( StopSource ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );
	alSourceStop(sid);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source )
  Pause the given source.
  $H arguments
   $ARG integer source: the ID of the source to play.
**/
DEFINE_FUNCTION_FAST( PauseSource ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );
	alSourcePause(sid);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( source )
  Rewind the given source. set playback postiton to beginning.
  $H arguments
   $ARG integer source: the ID of the source to play.
**/
DEFINE_FUNCTION_FAST( RewindSource ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_NUMBER(J_FARG(1));
	ALuint sid;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &sid ) );
	alSourceRewind(sid);
	return JS_TRUE;
}


/**doc
 * $VOID $INAME()
  $H OpenaL API
   alGenEffects
**/
DEFINE_FUNCTION_FAST( GenEffect ) {

	LOAD_OPENAL_EXTENSION( alGenEffects, LPALGENEFFECTS );

	ALuint effect;
	alGenEffects(1, &effect);

	return JS_TRUE;
}





/**doc
 * $VOID $INAME( sound ) $DEPRECATED
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
 * $INAME
  Obtain the most recent error generated in the AL state machine.
**/
DEFINE_PROPERTY(error) {

	*vp = INT_TO_JSVAL(alGetError());
	return JS_TRUE;
}

/*
DEFINE_PROPERTY_SETTER(error) {

	J_S_ASSERT( *vp == JSVAL_VOID, "Invalid error value." );
	alSetError(AL_NONE);
	return JS_TRUE;
}
*/

CONFIGURE_CLASS

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

		FUNCTION_FAST_ARGC( Open, 1 )
		FUNCTION_FAST_ARGC( Close, 0 )

		FUNCTION_FAST_ARGC( DopplerFactor, 1 )
		FUNCTION_FAST_ARGC( DopplerVelocity, 1 )
		FUNCTION_FAST_ARGC( SpeedOfSound, 1 )
		FUNCTION_FAST_ARGC( DistanceModel, 1 )

		FUNCTION_FAST_ARGC( Enable, 1 )
		FUNCTION_FAST_ARGC( Disable, 1 )
		FUNCTION_FAST_ARGC( IsEnabled, 1 )
		FUNCTION_FAST_ARGC( GetString, 1 )
		FUNCTION_FAST_ARGC( GetBoolean, 1 )
		FUNCTION_FAST_ARGC( GetInteger, 2 )
		FUNCTION_FAST_ARGC( GetDouble, 2 )

		FUNCTION_FAST_ARGC( Listener, 2 )
		FUNCTION_FAST_ARGC( GetListenerReal, 3 )
		FUNCTION_FAST_ARGC( GenSource, 0 )
		FUNCTION_FAST_ARGC( Source, 3 )
		FUNCTION_FAST_ARGC( GetSourceInteger, 3 )
		FUNCTION_FAST_ARGC( GetSourceReal, 3 )
		FUNCTION_FAST_ARGC( DeleteSource, 1 )
		FUNCTION_FAST_ARGC( SourceQueueBuffers, 2 )
		FUNCTION_FAST_ARGC( SourceUnqueueBuffers, 2 )
		FUNCTION_FAST_ARGC( Buffer, 1 )
		FUNCTION_FAST_ARGC( GetBufferReal, 3 )
		FUNCTION_FAST_ARGC( GetBufferInteger, 3 )
		FUNCTION_FAST_ARGC( DeleteBuffer, 1 )
		FUNCTION_FAST_ARGC( PlaySource, 1 )
		FUNCTION_FAST_ARGC( StopSource, 1 )
		FUNCTION_FAST_ARGC( PauseSource, 1 )
		FUNCTION_FAST_ARGC( RewindSource, 1 )

		FUNCTION_FAST_ARGC( PlaySound, 1 ) // non-openal API

		// OpenAL extensions
		FUNCTION_FAST_ARGC( GenEffect, 0 )
		

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(error)
	END_STATIC_PROPERTY_SPEC

END_CLASS

/**doc
=== Examples ===
$H example 1
 A simple ogg player
 {{{
 LoadModule('jsio');
 LoadModule('jsstd');
 LoadModule('jssound');
 LoadModule('jsaudio');
 
 var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').Open(File.RDONLY));
 var sourceId = Oal.GenSource();

 var pcm;
 while ( pcm = decoder.Read(10000) ) {
 	
 	var bufferId = Oal.Buffer(pcm);
 	Oal.SourceQueueBuffers(sourceId, bufferId);
 	if ( Oal.GetSourceInteger(sourceId, Oal.SOURCE_STATE) == Oal.INITIAL )
 		Oal.PlaySource(sourceId);
 };
 
 var totalTime = decoder.frames/decoder.rate;
 var currentTimeOffset = Oal.GetSourceReal(sourceId, Oal.SEC_OFFSET);
 Sleep( 1000 * (totalTime - currentTimeOffset) );
 }}}
**/
 
 
/* ogg test files:
	http://xiph.org/vorbis/listen.html
*/

/* Introduction to EFX
	http://connect.creativelabs.com/developer/Wiki/Introduction%20to%20EFX.aspx
*/


/* full List of EFX functions

	alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS )alGetProcAddress("alDeleteEffects");
	alIsEffect = (LPALISEFFECT )alGetProcAddress("alIsEffect");
	alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
	alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
	alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
	alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
	alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
	alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
	alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
	alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
	alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
	alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
	alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
	alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
	alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
	alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
	alGetFilteri = (LPALGETFILTERI )alGetProcAddress("alGetFilteri");
	alGetFilteriv = (LPALGETFILTERIV )alGetProcAddress("alGetFilteriv");
	alGetFilterf = (LPALGETFILTERF )alGetProcAddress("alGetFilterf");
	alGetFilterfv = (LPALGETFILTERFV )alGetProcAddress("alGetFilterfv");
	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
	alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
	alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
	alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
	alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
	alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
	alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
	alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
	alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetAuxiliaryEffectSlotfv");

*/