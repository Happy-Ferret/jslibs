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

#include "queue.h"

#include "error.h"

#include "oalefxapi.h"


struct Private {
	ALuint sid;
	jl::Queue *queue;
	ALuint effectSlot;
	float totalTime;
};


inline float BufferSecTime( ALint bid ) {

	ALint freq, size, bits, channels;
	alGetBufferi(bid, AL_FREQUENCY, &freq);
	alGetBufferi(bid, AL_SIZE, &size);
	alGetBufferi(bid, AL_BITS, &bits);
	alGetBufferi(bid, AL_CHANNELS, &channels);
	return (float)size / (float)( (bits/8) * channels * freq );
}




/*
	jsval *pItem = (jsval*)jl_malloc(sizeof(jsval));
	JL_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}

inline JSBool UnshiftJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)jl_malloc(sizeof(jsval));
	JL_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}
*/


JSBool QueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)JS_malloc(cx, sizeof(jsval));
	JL_CHK( pItem );
	*pItem = value;
	QueuePush(queue, pItem); // no need to JS_AddRoot *pItem, see Tracer callback !
	return JS_TRUE;
	JL_BAD;
}

/*
JSBool UnqueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	for ( jl::QueueCell *it = jl::QueueBegin(queue); it; it = jl::QueueNext(it) ) {

		if ( *(jsval*)QueueGetData(it) == value ) {

			jsval *pItem = (jsval*)QueueRemoveCell(queue, it);
			JS_free(cx, pItem);
			return JS_TRUE;
		}
	}
	return JS_FALSE; // not found
}
*/

jsval UnqueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval *rval ) {

	jsval *pval = (jsval*)QueueShift(queue);
	*rval = *pval;
	JS_free(cx, pval);
	return JS_TRUE;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( OalSource )

DEFINE_TRACER() {

	Private *pv = (Private*)JL_GetPrivate(trc->context, obj);
	if ( pv )
		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

			jsval *val = (jsval*)QueueGetData(it);
			if ( !JSVAL_IS_PRIMITIVE(*val) )
				JS_CALL_VALUE_TRACER(trc, *val, "jsstd/Buffer:bufferQueueItem");
		}
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( pv ) {

		while ( !QueueIsEmpty(pv->queue) ) {

			jsval *pItem = (jsval*)QueuePop(pv->queue);
			JS_free(cx, pItem);
		}
		QueueDestruct(pv->queue);

		if ( alcGetCurrentContext() ) {

//			alAuxiliaryEffectSloti(pv->effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
			alDeleteSources(1, &pv->sid);
		}
	}
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new source object.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );

	pv->queue = jl::QueueConstruct();

	alGenSources(1, &pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( buffer )
  $H arguments
   $ARG OalBuffer | BufferId: a Buffer Object or a buffer Id.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( QueueBuffers ) {

	JL_S_ASSERT_ARG_MIN(1);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	*JL_FRVAL = JSVAL_VOID;
	ALuint bid;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &bid) );
	JL_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );

	alSourceQueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( QueueBuffersJsval(cx, pv->queue, JL_FARG(1)) );

	ALint queueSize;
	alGetSourcei(pv->sid, AL_BUFFERS_QUEUED, &queueSize);
	if ( queueSize == 1 )
		pv->totalTime = 0;
	pv->totalTime += BufferSecTime(bid);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE OalBuffer $INAME()
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( UnqueueBuffers ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	ALuint bid;
	alSourceUnqueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( UnqueueBuffersJsval(cx, pv->queue, JL_FRVAL ) );
	JL_SAFE(
		ALuint tmp;
		JL_CHK( JsvalToUInt(cx, *JL_FRVAL, &tmp) );
		JL_S_ASSERT( bid == tmp, "Internal error in UnqueueBuffers()." );
	);
	pv->totalTime -= BufferSecTime(bid);
	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION_FAST( Play ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );

	alSourcePlay(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( Pause ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );

	alSourcePause(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( Stop ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );

	alSourceStop(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( Rewind ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );

	alSourceRewind(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION_FAST( Effect ) {

	JL_S_ASSERT_ARG_MIN(1);
	JL_S_ASSERT_INT(JL_FARG(1));

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	ALuint send;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &send) );

	ALuint effectSlot = AL_EFFECTSLOT_NULL;
	ALuint filter = AL_FILTER_NULL;
	if ( JL_FARG_ISDEF(2) )
		JL_CHK( JsvalToUInt(cx, JL_FARG(2), &effectSlot) );
//	if ( JL_FARG_ISDEF(3) )
//		JL_CHK( JsvalToUInt(cx, JL_FARG(3), &filter) );

//	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
//	ALCint numSends;
//	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &numSends);

	JL_CHK( JS_DefineProperty(cx, JL_FOBJ, "effect", JL_FARG(2), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, send, filter);
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( UIntToJsval(cx, pv->sid, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY( effectSlot ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	ALuint effectSlot;
	if ( !JSVAL_IS_VOID(*vp) )
		JL_CHK( JsvalToUInt(cx, *vp, &effectSlot) );
	else
		effectSlot = AL_EFFECTSLOT_NULL;

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY( directFilter ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	ALuint filter;
	if ( !JSVAL_IS_VOID(*vp) )
		JL_CHK( JsvalToUInt(cx, *vp, &filter) );
	else
		filter = AL_FILTER_NULL;

	alSourcei(pv->sid, AL_DIRECT_FILTER, filter);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( buffer ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALint bid;
	if ( JSVAL_IS_VOID( *vp ) || *vp == JSVAL_ZERO )
		bid = AL_NONE;
	else
		JL_CHK( JsvalToInt(cx, *vp, &bid) ); // calls OalBuffer valueOf function
	JL_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );

	alSourcei(pv->sid, AL_BUFFER, bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	pv->totalTime = BufferSecTime(bid);

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( buffer ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALint bid;
	alGetSourcei(pv->sid, AL_BUFFER, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	// look if the current value hold tby the property (_STORE) is the current buffer)
	if ( !JSVAL_IS_VOID( *vp ) ) {

		ALint tmp;
		JL_CHK( JsvalToInt(cx, *vp, &tmp) ); // calls OalBuffer valueOf function
		JL_S_ASSERT( alIsBuffer(tmp), "Invalid buffer." );
		if ( tmp == bid )
			goto out;
	}

	// find the buffer object in the list of jsval
	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		jsval *val = (jsval*)QueueGetData(it);
		ALint tmp;
		JL_CHK( JsvalToInt(cx, *val, &tmp) ); // calls OalBuffer valueOf function
		JL_S_ASSERT( alIsBuffer(tmp), "Invalid buffer." );
		if ( tmp == bid ) {

			*vp = *val;
			goto out;
		}
	}

	JL_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );
	JL_CHK( IntToJsval(cx, bid, vp) );

out:
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/*
DEFINE_PROPERTY_SETTER( position ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];
	size_t len;
	JL_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FUNCTION_FAST( Position ) {

	JL_S_ASSERT_ARG_MIN(3);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &pos[0]) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &pos[1]) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( position ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_POSITION, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_PROPERTY_SETTER( velocity ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];
	size_t len;
	JL_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	return JS_TRUE;
	JL_BAD;
}
*/
DEFINE_FUNCTION_FAST( Velocity ) {

	JL_S_ASSERT_ARG_MIN(3);
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];
	JL_CHK( JsvalToFloat(cx, JL_FARG(1), &pos[0]) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(2), &pos[1]) );
	JL_CHK( JsvalToFloat(cx, JL_FARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( velocity ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_VELOCITY, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}




DEFINE_PROPERTY( remainingTime ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	ALint loop;
	alGetSourcei(pv->sid, AL_LOOPING, &loop);
	if ( loop == AL_TRUE ) {

		*vp = JS_GetPositiveInfinityValue(cx);
		return JS_TRUE;
	}

	ALint state;
	alGetSourcei(pv->sid, AL_SOURCE_STATE, &state);
	if ( state != AL_PLAYING && state != AL_PAUSED ) {

		*vp = JSVAL_VOID;
		return JS_TRUE;
	}

	ALfloat secOffset;
	alGetSourcef(pv->sid, AL_SEC_OFFSET, &secOffset);
	JL_CHK( FloatToJsval(cx, pv->totalTime - secOffset, vp) );
	return JS_TRUE;
	JL_BAD;
}



static const int enumToConst[] = {
	AL_SOURCE_STATE,
	AL_SOURCE_RELATIVE,
	AL_LOOPING,
	AL_SEC_OFFSET,
	AL_GAIN,
	AL_MIN_GAIN,
	AL_MAX_GAIN,
	AL_AIR_ABSORPTION_FACTOR,
	AL_ROOM_ROLLOFF_FACTOR,
	AL_CONE_OUTER_GAINHF,
	AL_DIRECT_FILTER_GAINHF_AUTO,
	AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,
	AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO,
	AL_REFERENCE_DISTANCE,
	AL_MAX_DISTANCE,
	AL_BUFFERS_QUEUED,
	AL_BUFFERS_PROCESSED
};


enum {
	state = 0,
	sourceRelative,
	looping,
	secOffset,
	gain,
	minGain,
	maxGain,
	airAbsorptionFactor,
	roomRolloffFactor,
	coneOuterGainhf,
	directFilterGainhfAuto,
	auxiliarySendFilterGainAuto,
	auxiliarySendFilterGainhfAuto,
	referenceDistance,
	maxDistance,
	buffersQueued,
	buffersProcessed
};



// 'ind' suffix mean that an indirection is needed because tinyid (8bit) cannot store any OpenAL constant.
DEFINE_PROPERTY_SETTER( sourceFloatInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)];
	float f;
	JL_CHK( JsvalToFloat(cx, *vp, &f) );
	alSourcef(pv->sid, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceFloatInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	float f;
	alGetSourcef(pv->sid, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( FloatToJsval(cx, f, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( sourceIntInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	int i;
	JL_CHK( JsvalToInt(cx, *vp, &i) );
	alSourcei(pv->sid, param, i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceIntInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	int i;
	alGetSourcei(pv->sid, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( IntToJsval(cx, i, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( sourceBoolInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	bool b;
	JL_CHK( JsvalToBool(cx, *vp, &b) );
	alSourcei(pv->sid, param, b ? AL_TRUE : AL_FALSE);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceBoolInd ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	int i;
	alGetSourcei(pv->sid, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	*vp = i == AL_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( Position, 3 )
		FUNCTION_FAST_ARGC( Velocity, 3 )

		FUNCTION_FAST( Play )
		FUNCTION_FAST( Pause )
		FUNCTION_FAST( Stop )
		FUNCTION_FAST( Rewind )

		FUNCTION_FAST( QueueBuffers )
		FUNCTION_FAST( UnqueueBuffers )

		FUNCTION_FAST( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC

		PROPERTY_READ( position )
		PROPERTY_READ( velocity )

//		PROPERTY_WRITE_STORE( buffer )
		PROPERTY( buffer )

		PROPERTY_WRITE( effectSlot )
		PROPERTY_WRITE( directFilter )

		PROPERTY_READ( remainingTime )

		PROPERTY_SWITCH_READ( buffersQueued, sourceIntIndGetter )
		PROPERTY_SWITCH_READ( buffersProcessed, sourceIntIndGetter )
		PROPERTY_SWITCH_READ( state, sourceIntIndGetter )
		PROPERTY_SWITCH( looping, sourceBoolInd )
		PROPERTY_SWITCH( sourceRelative, sourceBoolInd )
		PROPERTY_SWITCH( gain, sourceFloatInd )
		PROPERTY_SWITCH( minGain, sourceFloatInd )
		PROPERTY_SWITCH( maxGain, sourceFloatInd )
		PROPERTY_SWITCH( secOffset, sourceFloatInd )
		PROPERTY_SWITCH( airAbsorptionFactor, sourceFloatInd )
		PROPERTY_SWITCH( roomRolloffFactor, sourceFloatInd )
		PROPERTY_SWITCH( coneOuterGainhf, sourceFloatInd )
		PROPERTY_SWITCH( directFilterGainhfAuto, sourceBoolInd )
		PROPERTY_SWITCH( auxiliarySendFilterGainAuto, sourceBoolInd )
		PROPERTY_SWITCH( auxiliarySendFilterGainhfAuto, sourceBoolInd )
		PROPERTY_SWITCH( referenceDistance, sourceFloatInd )
		PROPERTY_SWITCH( maxDistance, sourceFloatInd )

	END_PROPERTY_SPEC

END_CLASS
