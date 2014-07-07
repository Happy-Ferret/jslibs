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
BEGIN_CLASS( Oal )

/**doc
=== Static functions ===
**/


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ deviceName ] )
  Open an audio device.
  $H arguments
   $ARG $STR deviceName: "Generic Hardware", "Generic Software", "DirectSound3D" (for legacy), "DirectSound", "MMSYSTEM"
    If no device name is specified, we will attempt to use DS3D.
  $H OpenAL API
   alcOpenDevice, alcCreateContext, alcMakeContextCurrent
**/
DEFINE_FUNCTION( open ) {

	JL_DEFINE_ARGS;
	
	// Initialize the OpenAL library (cf. alutInit)
	JLData deviceName;
	JL_ASSERT_WARN( alcGetCurrentContext() == NULL, E_LIB, E_STR("OpenAL"), E_OPEN );

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &deviceName) );

	// Doc: alcOpenDevice() open the Device specified. Current options are:
	//   "Generic Hardware"
	//   "Generic Software"
	//   "DirectSound3D" (for legacy)
	//   "DirectSound"
	//   "MMSYSTEM"
	// If no device name is specified, we will attempt to use DS3D.
	ALCdevice *device = alcOpenDevice (deviceName.GetStrConstOrNull());
	JL_CHKM( device != NULL, E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_OPEN_DEVICE") );

//	ALint attribs[4] = { 0 };
//	attribs[0] = ALC_MAX_AUXILIARY_SENDS;
//	attribs[1] = 4;

	ALCcontext *context = alcCreateContext (device, NULL);
	if ( context == NULL ) {
		alcCloseDevice (device);
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_CREATE_CONTEXT") );
	}
	if ( !alcMakeContextCurrent(context) ) {

		alcDestroyContext (context);
		alcCloseDevice (device);
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_MAKE_CONTEXT_CURRENT") );
	}

	InitEfxApi();

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close the current audio device.
  $H OpenAL API
   alcGetCurrentContext, alcMakeContextCurrent, alcGetContextsDevice, alcDestroyContext, alcCloseDevice
**/
DEFINE_FUNCTION( close ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;

	ResetEfxApi();
	// cf. alutExit
	ALCcontext *context = alcGetCurrentContext();
	JL_ASSERT_WARN( context != NULL, E_LIB, E_STR("OpenAL"), E_CLOSED );

	ALCdevice *device;
	if (!alcMakeContextCurrent (NULL))
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_MAKE_CONTEXT_CURRENT") );
	device = alcGetContextsDevice (context);
	if (alcGetError (device) != ALC_NO_ERROR )
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_ALC_ERROR_ON_ENTRY") );
	alcDestroyContext (context);
	if (alcGetError (device) != ALC_NO_ERROR)
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_DESTROY_CONTEXT") );
	if (!alcMakeContextCurrent(NULL))
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_MAKE_CONTEXT_CURRENT") );
	if (!alcCloseDevice (device))
		JL_ERR( E_LIB, E_STR("OpenAL"), E_OPERATION, E_COMMENT("ALUT_ERROR_CLOSE_DEVICE") );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  is true if EFX extension is available.
**/
DEFINE_PROPERTY_GETTER( hasEfx ) {

	JL_IGNORE( id, obj );

	ALCcontext *pContext = alcGetCurrentContext();
	JL_ASSERT( pContext != NULL, E_LIB, E_STR("OpenAL"), E_INTERNAL );
	ALCdevice *pDevice = alcGetContextsDevice(pContext);
	vp.setBoolean( alcIsExtensionPresent(pDevice, (ALCchar*)ALC_EXT_EFX_NAME) != ALC_FALSE );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  is the number of aux sends per source.
**/
DEFINE_PROPERTY_GETTER( maxAuxiliarySends ) {

	JL_IGNORE( id, obj );

	ALCcontext *pContext = alcGetCurrentContext();
	JL_ASSERT( pContext != NULL, E_LIB, E_STR("OpenAL"), E_INTERNAL );
	ALCdevice *pDevice = alcGetContextsDevice(pContext);
	ALCint numSends;
	alcGetIntegerv(pDevice, ALC_MAX_AUXILIARY_SENDS, 1, &numSends);
	JL_CHK( jl::setValue(cx, vp, numSends) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
  Selects the OpenAL Doppler factor value. The default Doppler factor value is 1.0 .
  $H arguments
   $ARG $NUM value
  $H OpenAL API
   alDopplerFactor
**/
DEFINE_FUNCTION( dopplerFactor ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	float value;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &value) );
	alDopplerFactor( value );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
  Selects the OpenAL Doppler velocity value. The default Doppler velocity value is 343.3 .
  $H arguments
   $ARG $NUM value
  $H OpenAL API
   alDopplerVelocity
**/
DEFINE_FUNCTION( dopplerVelocity ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	float value;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &value) );
	alDopplerVelocity( value );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
  Selects the OpenAL Speed of Sound value.
  $H arguments
   $ARG $NUM value
  $H OpenAL API
   alSpeedOfSound
**/
DEFINE_FUNCTION( speedOfSound ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	float value;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &value) );
	alSpeedOfSound( value );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( distanceModel )
  $H arguments
   $ARG $INT value
  $H OpenAL API
   alDistanceModel
**/
DEFINE_FUNCTION( distanceModel ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	unsigned int distanceModel;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &distanceModel) );
	alDistanceModel( distanceModel );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alEnable
**/
DEFINE_FUNCTION( enable ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	alEnable( JL_ARG(1).toInt32() );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alDisable
**/
DEFINE_FUNCTION( disable ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	alDisable( JL_ARG(1).toInt32() );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( cap )
  $H arguments
   $ARG GLenum cap
  $H OpenAL API
   alIsEnabled
**/
DEFINE_FUNCTION( isEnabled ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	*JL_RVAL = BOOLEAN_TO_JSVAL( alIsEnabled( JL_ARG(1).toInt32() ) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( pname )
  $H arguments
   $ARG GLenum pname
  $H return value
   value of a selected parameter.
  $H OpenAL API
   alGetString
**/
DEFINE_FUNCTION( getString ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	const ALchar* str = alGetString(JL_ARG(1).toInt32());
	if ( str == NULL ) {

		JL_RVAL.setUndefined();
		return true;
	}
	JL_CHK( jl::setValue( cx, JL_RVAL, jl::CStrSpec( str ) ) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME( pname )
  $H arguments
   $ARG ALenum pname
  $H return value
   value of a selected parameter.
  $H OpenAL API
   alGetBooleanv
**/
DEFINE_FUNCTION( getBoolean ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALboolean params;
	alGetBooleanv(JL_ARG(1).toInt32(), &params);
	*JL_RVAL = BOOLEAN_TO_JSVAL(params);

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG ALenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else it returns a single value.
  $H return value
   A single value or an array of values of a selected parameter.
  $H OpenAL API
   alGetIntegerv
**/
DEFINE_FUNCTION( getInteger ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALint params[16];
	alGetIntegerv(JL_ARG(1).toInt32(), params);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_INTEGER(2);
		int count = JL_ARG(2).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			tmpValue = INT_TO_JSVAL( params[count] );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		*JL_RVAL = INT_TO_JSVAL( params[0] );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( pname [, count] )
  $H arguments
   $ARG ALenum pname
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetDoublev
**/
DEFINE_FUNCTION( getDouble ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALdouble params[16];
	alGetDoublev(JL_ARG(1).toInt32(), params);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_INTEGER(2);
		int count = JL_ARG(2).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK( jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( pname, params )
  $H arguments
   $ARG ALenum pname:
   $ARG $ARRAY params:
  $H OpenAL API
   alListeneri, alListenerf, alListenerfv
**/
DEFINE_FUNCTION( listener ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_INTEGER(1);

	JL_RVAL.setUndefined();
	if ( JSVAL_IS_INT(JL_ARG(2)) ) {

		alListeneri( JL_ARG(1).toInt32(), JL_ARG(2).toInt32() );
		return true;
	}
	if ( JL_ARG(2).isDouble() ) {

		float param;
		JL_CHK( jl::getValue(cx, JL_ARG(2), &param) );
		alListenerf( JL_ARG(1).toInt32(), param );
		return true;
	}
	if ( JL_ValueIsArrayLike(cx, JL_ARG(2)) ) {

		ALfloat params[16];
		uint32_t length;
//		J_JSVAL_TO_REAL_VECTOR( JL_ARG(2), params, length );
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, 16, &length) );
		alListenerfv( JL_ARG(1).toInt32(), params );
		return true;
	}

	JL_ERR( E_ARG, E_NUM(2), E_INVALID );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG $INT source:
   $ARG ALenum pname:
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetListenerfv
**/
DEFINE_FUNCTION( getListenerReal ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALfloat params[16];
	alGetListenerfv(JL_ARG(1).toInt32(), params);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_INTEGER(2);
		int count = JL_ARG(2).toInt32();
		JS::RootedObject arrayObj(JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while ( count-- ) {
			
			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenAL API
   alSourcei, alSourcef, alSourcefv
**/
DEFINE_FUNCTION( genSource ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
	ALuint sourceID; // The OpenAL sound source
	alGenSources(1, &sourceID);
	JL_CHK( jl::setValue(cx, JL_RVAL, sourceID) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source, pname, params )
  $H arguments
   $ARG $INT source:
   $ARG ALenum pname:
   $ARG $ARRAY params:
  $H OpenAL API
   alSourcei, alSourcef, alSourcefv
**/
DEFINE_FUNCTION( source ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(3);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );

	JL_RVAL.setUndefined();
	if ( JSVAL_IS_INT(JL_ARG(3)) ) {

		alSourcei( sid, JL_ARG(2).toInt32(), JL_ARG(3).toInt32() );
		return true;
	}
	if ( JL_ARG(3).isDouble() ) {

		float param;
		JL_CHK( jl::getValue(cx, JL_ARG(3), &param) );
		alSourcef( sid, JL_ARG(2).toInt32(), param );
		return true;
	}
	if ( JL_ValueIsArrayLike(cx, JL_ARG(3)) ) {

		ALfloat params[16];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(3), params, COUNTOF(params), &length ) );
		alSourcefv( sid, JL_ARG(2).toInt32(), params );
		return true;
	}

	JL_ERR( E_ARG, E_NUM(3), E_INVALID );
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG $INT source:
   $ARG ALenum pname:
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetSourcef
**/
DEFINE_FUNCTION( getSourceReal ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );

	ALfloat params[16];

	ALenum pname = JL_ARG(2).toInt32();
	alGetSourcef(sid, pname, params);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	if ( JL_ARG_ISDEF(3) ) {

		JL_ASSERT_ARG_IS_INTEGER(3);
		int count = JL_ARG(3).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( source, pname [, count] )
**/
DEFINE_FUNCTION( getSourceInteger ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_NUMBER(1);
	JL_ASSERT_ARG_IS_INTEGER(2);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );

	ALint params[16];

	ALenum pname = JL_ARG(2).toInt32();
	alGetSourcei(sid, pname, params);

	JL_CHK( CheckThrowCurrentOalError(cx) );

	if ( JL_ARG_ISDEF(3) ) {

		JL_ASSERT_ARG_IS_INTEGER(3);
		int count = JL_ARG(3).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK( jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source )
  $H arguments
   $ARG $INT source: the source id.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( deleteSource ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	alDeleteSources(1, &sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source, buffer | bufferArray )
  $H arguments
   $ARG $INT source: the source id.
   $ARG $INT buffer: the buffer id.
   $ARG $ARRAY bufferArray: an Array of buffer id.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( sourceQueueBuffers ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	JL_RVAL.setUndefined();

	if ( JL_ValueIsArray(cx, JL_ARG(2)) ) { // no array-like. We must exclude strings that may be converted in integer below.

		ALuint params[1024];
 		uint32_t length = COUNTOF(params);
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, COUNTOF(params), &length) );
		alSourceQueueBuffers( sid, length, params );
		JL_CHK( CheckThrowCurrentOalError(cx) );
		return true;
	}

	ALuint buffer;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &buffer) );
	alSourceQueueBuffers( sid, 1, &buffer );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source, buffer | bufferArray )
  $H arguments
   $ARG $INT buffer: the buffer id.
   $ARG $ARRAY bufferArray: an Array of buffer id.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( sourceUnqueueBuffers ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(2);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	JL_RVAL.setUndefined();

	if ( JL_ValueIsArray(cx, JL_ARG(2)) ) { // no array-like. We must exclude strings that may be converted in integer below.

		ALuint params[1024];
		uint32_t length;
		JL_CHK( jl::getVector(cx, JL_ARG(2), params, COUNTOF(params), &length) );
		alSourceUnqueueBuffers( sid, length, params );
		return true;
	}

	ALuint buffer;
	JL_CHK( jl::getValue(cx, JL_ARG(2), &buffer) );
	alSourceUnqueueBuffers( sid, 1, &buffer );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $INT $INAME( soundObject )
  Creates a new buffer and attach a sound data to it. The data comming from the soundObject is copied into the OpenAL system.
  $note
   Buffers containing audio data with more than one channel will be played without 3D spatialization features  these formats are normally used for background music.
  $H arguments
   $ARG soundObject sound: a sound object that contains PCM audio data and the following properties: rate, channels and bits.
**/
DEFINE_FUNCTION( buffer ) {

	JL_DEFINE_ARGS;
	JLData data;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	int bits, rate, channels, frames;
	data = JL_GetByteAudioObject(cx, JL_ARG(1), &bits, &channels, &frames, &rate);

	ALuint bufferID; // The OpenAL sound buffer ID
	alGenBuffers(1, &bufferID);
	JL_CHK( CheckThrowCurrentOalError(cx) );

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

	// Upload sound data to buffer
	alBufferData(bufferID, format, data.GetConstStr(), (ALsizei)data.Length(), rate);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( jl::setValue(cx, JL_RVAL, bufferID) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $REAL | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG $INT source:
   $ARG ALenum pname:
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetBufferfv
**/
DEFINE_FUNCTION( getBufferReal ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALfloat params[16];
	alGetBufferfv(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_INTEGER(2);
		int count = JL_ARG(2).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while ( count-- ) {

			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK(jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT | $ARRAY $INAME( source, pname [, count] )
  $H arguments
   $ARG $INT source:
   $ARG ALenum pname:
   $ARG $INT count: is the number of expected values. If _count_ is defined, the function will returns an array of values, else a single value.
  $H return value
   single value or Array of values of the selected parameter.
  $H OpenAL API
   alGetBufferiv
**/
DEFINE_FUNCTION( getBufferInteger ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	ALint params[16];
	alGetBufferiv(JL_ARG(1).toInt32(), JL_ARG(2).toInt32(), params);

	if ( JL_ARG_ISDEF(2) ) {

		JL_ASSERT_ARG_IS_INTEGER(2);
		int count = JL_ARG(2).toInt32();
		JS::RootedObject arrayObj(cx, JS_NewArrayObject(cx, count));
		JL_CHK( arrayObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrayObj);
		jsval tmpValue;
		while (count--) {

			JL_CHK( jl::setValue(cx, tmpValue, params[count]) );
			JL_CHK( JL_SetElement(cx, arrayObj, count, tmpValue) );
		}
	} else {

		JL_CHK( jl::setValue(cx, JL_RVAL, params[0]) );
	}
	return true;
	JL_BAD;
}





/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( buffer )
  $H arguments
   $ARG $INT buffer: the buffer id.
  $H note
   Buffers that have been unqueued from all sources are UNUSED. Buffers that are UNUSED can be deleted, or changed by alBufferData commands.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( deleteBuffer ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint bufferId;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &bufferId ) );
//	alBufferData(bufferId, 0, NULL, 0, 0);
	alDeleteBuffers(1, &bufferId);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source )
  Plays the given source.
  $H arguments
   $ARG $INT source: the ID of the source to play.
**/
DEFINE_FUNCTION( playSource ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	alSourcePlay(sid);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source )
  Stop the given source.
  $H arguments
   $ARG $INT source: the ID of the source to play.
**/
DEFINE_FUNCTION( stopSource ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	alSourceStop(sid);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source )
  Pause the given source.
  $H arguments
   $ARG $INT source: the ID of the source to play.
**/
DEFINE_FUNCTION( pauseSource ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	alSourcePause(sid);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( source )
  Rewind the given source. set playback postiton to beginning.
  $H arguments
   $ARG $INT source: the ID of the source to play.
**/
DEFINE_FUNCTION( rewindSource ) {

	JL_DEFINE_ARGS;
	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_NUMBER(1);

	ALuint sid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &sid ) );
	alSourceRewind(sid);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/* *doc
$TOC_MEMBER $INAME
 $INT $INAME()
  $H OpenaL API
   alGenEffects
**/
/*
DEFINE_FUNCTION( genEffect ) {

	ALuint eid;
	alGenEffects(1, &eid);
	JL_CHK( JL_NativeToJsval(cx, eid, JL_RVAL) );
	return true;
	JL_BAD;
}
*/

/* *doc
$TOC_MEMBER $INAME
 $VOID $INAME( effectId )
  $H OpenaL API
   alGenEffects
**/
/*
DEFINE_FUNCTION( deleteEffect ) {

	ALuint eid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &eid) );
	alDeleteEffects(1, eid);
	return true;
	JL_BAD;
}
*/



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( sound ) $DEPRECATED
  Plays a sound on the default playback device.
  $H arguments
   $ARG soundObject sound: sound object to play.
**/
DEFINE_FUNCTION( playSound ) {

	JL_DEFINE_ARGS;
	JLData data;

	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_OBJECT(1);

	int bits, rate, channels, frames;
	data = JL_GetByteAudioObject(cx, JL_ARG(1), &bits, &channels, &frames, &rate);

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

	JL_CHK( CheckThrowCurrentOalError(cx) );

	// Set the source and listener to the same location
	alListener3i(AL_POSITION, 0,0,0 );

	alSource3i(sourceID, AL_POSITION, 0,0,0 );

	// Upload sound data to buffer
	alBufferData(bufferID, format, data.GetConstStr(), (ALsizei)data.Length(), rate);

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

	uint32_t totalTime = size / (channels * (bits/8) * freq) * 1000;

	// Finally, play the sound!!!
	alSourcePlay(sourceID);

	jl::SleepMilliseconds(totalTime);

	// Query the state of the souce
	alGetSourcei(sourceID, AL_SOURCE_STATE, &state); // do { } while (state != AL_STOPPED);
	}

	// Clean up sound buffer and source
	alDeleteBuffers(1, &bufferID);
	alDeleteSources(1, &sourceID);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision$"))
	BEGIN_CONST
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


		CONST_INTEGER(METERS_PER_UNIT                                , AL_METERS_PER_UNIT                                )
		CONST_INTEGER(DIRECT_FILTER                                  , AL_DIRECT_FILTER                                  )
		CONST_INTEGER(AUXILIARY_SEND_FILTER                          , AL_AUXILIARY_SEND_FILTER                          )
		CONST_INTEGER(AIR_ABSORPTION_FACTOR                          , AL_AIR_ABSORPTION_FACTOR                          )
		CONST_INTEGER(ROOM_ROLLOFF_FACTOR                            , AL_ROOM_ROLLOFF_FACTOR                            )
		CONST_INTEGER(CONE_OUTER_GAINHF                              , AL_CONE_OUTER_GAINHF                              )
		CONST_INTEGER(DIRECT_FILTER_GAINHF_AUTO                      , AL_DIRECT_FILTER_GAINHF_AUTO                      )
		CONST_INTEGER(AUXILIARY_SEND_FILTER_GAIN_AUTO                , AL_AUXILIARY_SEND_FILTER_GAIN_AUTO                )
		CONST_INTEGER(AUXILIARY_SEND_FILTER_GAINHF_AUTO              , AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO              )
		CONST_INTEGER(REVERB_DENSITY                                 , AL_REVERB_DENSITY                                 )
		CONST_INTEGER(REVERB_DIFFUSION                               , AL_REVERB_DIFFUSION                               )
		CONST_INTEGER(REVERB_GAIN                                    , AL_REVERB_GAIN                                    )
		CONST_INTEGER(REVERB_GAINHF                                  , AL_REVERB_GAINHF                                  )
		CONST_INTEGER(REVERB_DECAY_TIME                              , AL_REVERB_DECAY_TIME                              )
		CONST_INTEGER(REVERB_DECAY_HFRATIO                           , AL_REVERB_DECAY_HFRATIO                           )
		CONST_INTEGER(REVERB_REFLECTIONS_GAIN                        , AL_REVERB_REFLECTIONS_GAIN                        )
		CONST_INTEGER(REVERB_REFLECTIONS_DELAY                       , AL_REVERB_REFLECTIONS_DELAY                       )
		CONST_INTEGER(REVERB_LATE_REVERB_GAIN                        , AL_REVERB_LATE_REVERB_GAIN                        )
		CONST_INTEGER(REVERB_LATE_REVERB_DELAY                       , AL_REVERB_LATE_REVERB_DELAY                       )
		CONST_INTEGER(REVERB_AIR_ABSORPTION_GAINHF                   , AL_REVERB_AIR_ABSORPTION_GAINHF                   )
		CONST_INTEGER(REVERB_ROOM_ROLLOFF_FACTOR                     , AL_REVERB_ROOM_ROLLOFF_FACTOR                     )
		CONST_INTEGER(REVERB_DECAY_HFLIMIT                           , AL_REVERB_DECAY_HFLIMIT                           )
		CONST_INTEGER(CHORUS_WAVEFORM                                , AL_CHORUS_WAVEFORM                                )
		CONST_INTEGER(CHORUS_PHASE                                   , AL_CHORUS_PHASE                                   )
		CONST_INTEGER(CHORUS_RATE                                    , AL_CHORUS_RATE                                    )
		CONST_INTEGER(CHORUS_DEPTH                                   , AL_CHORUS_DEPTH                                   )
		CONST_INTEGER(CHORUS_FEEDBACK                                , AL_CHORUS_FEEDBACK                                )
		CONST_INTEGER(CHORUS_DELAY                                   , AL_CHORUS_DELAY                                   )
		CONST_INTEGER(DISTORTION_EDGE                                , AL_DISTORTION_EDGE                                )
		CONST_INTEGER(DISTORTION_GAIN                                , AL_DISTORTION_GAIN                                )
		CONST_INTEGER(DISTORTION_LOWPASS_CUTOFF                      , AL_DISTORTION_LOWPASS_CUTOFF                      )
		CONST_INTEGER(DISTORTION_EQCENTER                            , AL_DISTORTION_EQCENTER                            )
		CONST_INTEGER(DISTORTION_EQBANDWIDTH                         , AL_DISTORTION_EQBANDWIDTH                         )
		CONST_INTEGER(ECHO_DELAY                                     , AL_ECHO_DELAY                                     )
		CONST_INTEGER(ECHO_LRDELAY                                   , AL_ECHO_LRDELAY                                   )
		CONST_INTEGER(ECHO_DAMPING                                   , AL_ECHO_DAMPING                                   )
		CONST_INTEGER(ECHO_FEEDBACK                                  , AL_ECHO_FEEDBACK                                  )
		CONST_INTEGER(ECHO_SPREAD                                    , AL_ECHO_SPREAD                                    )
		CONST_INTEGER(FLANGER_WAVEFORM                               , AL_FLANGER_WAVEFORM                               )
		CONST_INTEGER(FLANGER_PHASE                                  , AL_FLANGER_PHASE                                  )
		CONST_INTEGER(FLANGER_RATE                                   , AL_FLANGER_RATE                                   )
		CONST_INTEGER(FLANGER_DEPTH                                  , AL_FLANGER_DEPTH                                  )
		CONST_INTEGER(FLANGER_FEEDBACK                               , AL_FLANGER_FEEDBACK                               )
		CONST_INTEGER(FLANGER_DELAY                                  , AL_FLANGER_DELAY                                  )
		CONST_INTEGER(FREQUENCY_SHIFTER_FREQUENCY                    , AL_FREQUENCY_SHIFTER_FREQUENCY                    )
		CONST_INTEGER(FREQUENCY_SHIFTER_LEFT_DIRECTION               , AL_FREQUENCY_SHIFTER_LEFT_DIRECTION               )
		CONST_INTEGER(FREQUENCY_SHIFTER_RIGHT_DIRECTION              , AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION              )
		CONST_INTEGER(VOCAL_MORPHER_PHONEMEA                         , AL_VOCAL_MORPHER_PHONEMEA                         )
		CONST_INTEGER(VOCAL_MORPHER_PHONEMEA_COARSE_TUNING           , AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING           )
		CONST_INTEGER(VOCAL_MORPHER_PHONEMEB                         , AL_VOCAL_MORPHER_PHONEMEB                         )
		CONST_INTEGER(VOCAL_MORPHER_PHONEMEB_COARSE_TUNING           , AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING           )
		CONST_INTEGER(VOCAL_MORPHER_WAVEFORM                         , AL_VOCAL_MORPHER_WAVEFORM                         )
		CONST_INTEGER(VOCAL_MORPHER_RATE                             , AL_VOCAL_MORPHER_RATE                             )
		CONST_INTEGER(PITCH_SHIFTER_COARSE_TUNE                      , AL_PITCH_SHIFTER_COARSE_TUNE                      )
		CONST_INTEGER(PITCH_SHIFTER_FINE_TUNE                        , AL_PITCH_SHIFTER_FINE_TUNE                        )
		CONST_INTEGER(RING_MODULATOR_FREQUENCY                       , AL_RING_MODULATOR_FREQUENCY                       )
		CONST_INTEGER(RING_MODULATOR_HIGHPASS_CUTOFF                 , AL_RING_MODULATOR_HIGHPASS_CUTOFF                 )
		CONST_INTEGER(RING_MODULATOR_WAVEFORM                        , AL_RING_MODULATOR_WAVEFORM                        )
		CONST_INTEGER(AUTOWAH_ATTACK_TIME                            , AL_AUTOWAH_ATTACK_TIME                            )
		CONST_INTEGER(AUTOWAH_RELEASE_TIME                           , AL_AUTOWAH_RELEASE_TIME                           )
		CONST_INTEGER(AUTOWAH_RESONANCE                              , AL_AUTOWAH_RESONANCE                              )
		CONST_INTEGER(AUTOWAH_PEAK_GAIN                              , AL_AUTOWAH_PEAK_GAIN                              )
		CONST_INTEGER(COMPRESSOR_ONOFF                               , AL_COMPRESSOR_ONOFF                               )
		CONST_INTEGER(EQUALIZER_LOW_GAIN                             , AL_EQUALIZER_LOW_GAIN                             )
		CONST_INTEGER(EQUALIZER_LOW_CUTOFF                           , AL_EQUALIZER_LOW_CUTOFF                           )
		CONST_INTEGER(EQUALIZER_MID1_GAIN                            , AL_EQUALIZER_MID1_GAIN                            )
		CONST_INTEGER(EQUALIZER_MID1_CENTER                          , AL_EQUALIZER_MID1_CENTER                          )
		CONST_INTEGER(EQUALIZER_MID1_WIDTH                           , AL_EQUALIZER_MID1_WIDTH                           )
		CONST_INTEGER(EQUALIZER_MID2_GAIN                            , AL_EQUALIZER_MID2_GAIN                            )
		CONST_INTEGER(EQUALIZER_MID2_CENTER                          , AL_EQUALIZER_MID2_CENTER                          )
		CONST_INTEGER(EQUALIZER_MID2_WIDTH                           , AL_EQUALIZER_MID2_WIDTH                           )
		CONST_INTEGER(EQUALIZER_HIGH_GAIN                            , AL_EQUALIZER_HIGH_GAIN                            )
		CONST_INTEGER(EQUALIZER_HIGH_CUTOFF                          , AL_EQUALIZER_HIGH_CUTOFF                          )
		CONST_INTEGER(EFFECT_FIRST_PARAMETER                         , AL_EFFECT_FIRST_PARAMETER                         )
		CONST_INTEGER(EFFECT_LAST_PARAMETER                          , AL_EFFECT_LAST_PARAMETER                          )
		CONST_INTEGER(EFFECT_TYPE                                    , AL_EFFECT_TYPE                                    )
		CONST_INTEGER(EFFECT_NULL                                    , AL_EFFECT_NULL                                    )
		CONST_INTEGER(EFFECT_REVERB                                  , AL_EFFECT_REVERB                                  )
		CONST_INTEGER(EFFECT_CHORUS                                  , AL_EFFECT_CHORUS                                  )
		CONST_INTEGER(EFFECT_DISTORTION                              , AL_EFFECT_DISTORTION                              )
		CONST_INTEGER(EFFECT_ECHO                                    , AL_EFFECT_ECHO                                    )
		CONST_INTEGER(EFFECT_FLANGER                                 , AL_EFFECT_FLANGER                                 )
		CONST_INTEGER(EFFECT_FREQUENCY_SHIFTER                       , AL_EFFECT_FREQUENCY_SHIFTER                       )
		CONST_INTEGER(EFFECT_VOCAL_MORPHER                           , AL_EFFECT_VOCAL_MORPHER                           )
		CONST_INTEGER(EFFECT_PITCH_SHIFTER                           , AL_EFFECT_PITCH_SHIFTER                           )
		CONST_INTEGER(EFFECT_RING_MODULATOR                          , AL_EFFECT_RING_MODULATOR                          )
		CONST_INTEGER(EFFECT_AUTOWAH                                 , AL_EFFECT_AUTOWAH                                 )
		CONST_INTEGER(EFFECT_COMPRESSOR                              , AL_EFFECT_COMPRESSOR                              )
		CONST_INTEGER(EFFECT_EQUALIZER                               , AL_EFFECT_EQUALIZER                               )
		CONST_INTEGER(EFFECTSLOT_EFFECT                              , AL_EFFECTSLOT_EFFECT                              )
		CONST_INTEGER(EFFECTSLOT_GAIN                                , AL_EFFECTSLOT_GAIN                                )
		CONST_INTEGER(EFFECTSLOT_AUXILIARY_SEND_AUTO                 , AL_EFFECTSLOT_AUXILIARY_SEND_AUTO                 )
		CONST_INTEGER(EFFECTSLOT_NULL                                , AL_EFFECTSLOT_NULL                                )
		CONST_INTEGER(LOWPASS_GAIN                                   , AL_LOWPASS_GAIN                                   )
		CONST_INTEGER(LOWPASS_GAINHF                                 , AL_LOWPASS_GAINHF                                 )
		CONST_INTEGER(HIGHPASS_GAIN                                  , AL_HIGHPASS_GAIN                                  )
		CONST_INTEGER(HIGHPASS_GAINLF                                , AL_HIGHPASS_GAINLF                                )
		CONST_INTEGER(BANDPASS_GAIN                                  , AL_BANDPASS_GAIN                                  )
		CONST_INTEGER(BANDPASS_GAINLF                                , AL_BANDPASS_GAINLF                                )
		CONST_INTEGER(BANDPASS_GAINHF                                , AL_BANDPASS_GAINHF                                )
		CONST_INTEGER(FILTER_FIRST_PARAMETER                         , AL_FILTER_FIRST_PARAMETER                         )
		CONST_INTEGER(FILTER_LAST_PARAMETER                          , AL_FILTER_LAST_PARAMETER                          )
		CONST_INTEGER(FILTER_TYPE                                    , AL_FILTER_TYPE                                    )
		CONST_INTEGER(FILTER_NULL                                    , AL_FILTER_NULL                                    )
		CONST_INTEGER(FILTER_LOWPASS                                 , AL_FILTER_LOWPASS                                 )
		CONST_INTEGER(FILTER_HIGHPASS                                , AL_FILTER_HIGHPASS                                )
		CONST_INTEGER(FILTER_BANDPASS                                , AL_FILTER_BANDPASS                                )

// EFX
		CONST_INTEGER(CHORUS_WAVEFORM_SINUSOID                       , AL_CHORUS_WAVEFORM_SINUSOID                   )
		CONST_INTEGER(CHORUS_WAVEFORM_TRIANGLE                       , AL_CHORUS_WAVEFORM_TRIANGLE                   )

	END_CONST

	BEGIN_STATIC_FUNCTION_SPEC

		FUNCTION_ARGC( open, 1 )
		FUNCTION_ARGC( close, 0 )

		FUNCTION_ARGC( dopplerFactor, 1 )
		FUNCTION_ARGC( dopplerVelocity, 1 )
		FUNCTION_ARGC( speedOfSound, 1 )
		FUNCTION_ARGC( distanceModel, 1 )

		FUNCTION_ARGC( enable, 1 )
		FUNCTION_ARGC( disable, 1 )
		FUNCTION_ARGC( isEnabled, 1 )
		FUNCTION_ARGC( getString, 1 )
		FUNCTION_ARGC( getBoolean, 1 )
		FUNCTION_ARGC( getInteger, 2 )
		FUNCTION_ARGC( getDouble, 2 )

		FUNCTION_ARGC( listener, 2 )
		FUNCTION_ARGC( getListenerReal, 3 )
		FUNCTION_ARGC( genSource, 0 )
		FUNCTION_ARGC( source, 3 )
		FUNCTION_ARGC( getSourceInteger, 3 )
		FUNCTION_ARGC( getSourceReal, 3 )
		FUNCTION_ARGC( deleteSource, 1 )
		FUNCTION_ARGC( sourceQueueBuffers, 2 )
		FUNCTION_ARGC( sourceUnqueueBuffers, 2 )
		FUNCTION_ARGC( buffer, 1 )
		FUNCTION_ARGC( getBufferReal, 3 )
		FUNCTION_ARGC( getBufferInteger, 3 )
		FUNCTION_ARGC( deleteBuffer, 1 )
		FUNCTION_ARGC( playSource, 1 )
		FUNCTION_ARGC( stopSource, 1 )
		FUNCTION_ARGC( pauseSource, 1 )
		FUNCTION_ARGC( rewindSource, 1 )

		FUNCTION_ARGC( playSound, 1 ) // non-openal API

		// OpenAL extensions
//		FUNCTION_ARGC( GenEffect, 0 )


	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER(hasEfx)
		PROPERTY_GETTER(maxAuxiliarySends)
	END_STATIC_PROPERTY_SPEC

END_CLASS

/**doc
=== Examples ===
$H example 1
 A simple ogg player
 {{{
 loadModule('jsio');
 loadModule('jsstd');
 loadModule('jssound');
 loadModule('jsaudio');

 var decoder = new OggVorbisDecoder(new File('41_30secOgg-q0.ogg').open(File.RDONLY));

 Oal.open();
 var sourceId = Oal.genSource();
 var pcm;
 while ( pcm = decoder.read(10000) ) {

  var bufferId = Oal.buffer(pcm);
  Oal.sourceQueueBuffers(sourceId, bufferId);
  if ( Oal.getSourceInteger(sourceId, Oal.SOURCE_STATE) == Oal.INITIAL )
   Oal.playSource(sourceId);
 };

 var totalTime = decoder.frames / decoder.rate;
 var currentTimeOffset = Oal.getSourceReal(sourceId, Oal.SEC_OFFSET);
 sleep( 1000 * (totalTime - currentTimeOffset) );
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
