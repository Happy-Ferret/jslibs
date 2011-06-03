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
	JL_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
	JL_BAD;
}

inline JSBool UnshiftJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)jl_malloc(sizeof(jsval));
	JL_ASSERT_ALLOC( pItem );
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

JSBool UnqueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval *rval ) {

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

		if ( alcGetCurrentContext() ) {

//			alAuxiliaryEffectSloti(pv->effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
			alDeleteSources(1, &pv->sid);
		}

		if ( JL_GetHostPrivate(cx)->canSkipCleanup )
			return;

		while ( !QueueIsEmpty(pv->queue) ) {

			jsval *pItem = (jsval*)QueuePop(pv->queue);
			JS_free(cx, pItem);
		}
		QueueDestruct(pv->queue);
	}
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new source object.
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

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
DEFINE_FUNCTION( QueueBuffers ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint bid;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &bid) );
	JL_ASSERT( alIsBuffer(bid), E_ARG, E_NUM(1), E_INVALID );

	alSourceQueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( QueueBuffersJsval(cx, pv->queue, JL_ARG(1)) );

	ALint queueSize;
	alGetSourcei(pv->sid, AL_BUFFERS_QUEUED, &queueSize);
	if ( queueSize == 1 )
		pv->totalTime = 0;
	pv->totalTime += BufferSecTime(bid);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE OalBuffer $INAME()
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( UnqueueBuffers ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALuint bid;
	alSourceUnqueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( UnqueueBuffersJsval(cx, pv->queue, JL_RVAL ) );
	JL_SAFE(
		// ensure the oal buffer matchs the js buffer stored in the pv->queue
		ALuint tmp;
		JL_CHK( JL_JsvalToNative(cx, *JL_RVAL, &tmp) );
		JL_ASSERT( bid == tmp, E_LIB, E_INTERNAL, E_COMMENT("wrong buffer") ); // JL_ASSERT( bid == tmp, "Internal error in UnqueueBuffers()." );
	);
	pv->totalTime -= BufferSecTime(bid);
	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( Play ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourcePlay(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Pause ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourcePause(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Stop ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourceStop(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Rewind ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourceRewind(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_FUNCTION( Effect ) {

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALuint send;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &send) );

	ALuint effectSlot = AL_EFFECTSLOT_NULL;
	ALuint filter = AL_FILTER_NULL;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &effectSlot) );
//	if ( JL_ARG_ISDEF(3) )
//		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &filter) );

//	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
//	ALCint numSends;
//	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &numSends);

	JL_CHK( JS_DefineProperty(cx, JL_OBJ, "effect", JL_ARG(2), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, send, filter);
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FUNCTION( valueOf ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_NativeToJsval(cx, pv->sid, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY_SETTER( effectSlot ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint effectSlot;
	if ( !JSVAL_IS_VOID(*vp) )
		JL_CHK( JL_JsvalToNative(cx, *vp, &effectSlot) );
	else
		effectSlot = AL_EFFECTSLOT_NULL;

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( directFilter ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint filter;
	if ( !JSVAL_IS_VOID(*vp) )
		JL_CHK( JL_JsvalToNative(cx, *vp, &filter) );
	else
		filter = AL_FILTER_NULL;

	alSourcei(pv->sid, AL_DIRECT_FILTER, filter);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( buffer ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALint bid;
	if ( JSVAL_IS_VOID( *vp ) || *vp == JSVAL_ZERO )
		bid = AL_NONE;
	else
		JL_CHK( JL_JsvalToNative(cx, *vp, &bid) ); // calls OalBuffer valueOf function

	JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JL_ASSERT( alIsBuffer(bid), E_VALUE, E_ANINVALID, E_STR("buffer") );

	alSourcei(pv->sid, AL_BUFFER, bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	pv->totalTime = BufferSecTime(bid);

	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( buffer ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALint bid;
	alGetSourcei(pv->sid, AL_BUFFER, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	// look if the current value hold tby the property (_STORE) is the current buffer)
	if ( !JSVAL_IS_VOID( *vp ) ) {

		ALint tmp;
		JL_CHK( JL_JsvalToNative(cx, *vp, &tmp) ); // calls OalBuffer valueOf function

		JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JL_ASSERT( alIsBuffer(bid), E_VALUE, E_ANINVALID, E_STR("buffer") ); // JLSMSG_LOGIC_ERROR, "invalid buffer"

		if ( tmp == bid )
			goto out;
	}

	// find the buffer object in the list of jsval
	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		jsval *val = (jsval*)QueueGetData(it);
		ALint tmp;
		JL_CHK( JL_JsvalToNative(cx, *val, &tmp) ); // calls OalBuffer valueOf function
		JL_ASSERT( alIsBuffer(tmp), E_VALUE, E_INVALID ); // JLSMSG_LOGIC_ERROR, "invalid buffer"
		if ( tmp == bid ) {

			*vp = *val;
			goto out;
		}
	}

	JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JLSMSG_LOGIC_ERROR, "invalid buffer"
	JL_CHK( JL_NativeToJsval(cx, bid, vp) );

out:
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/*
DEFINE_PROPERTY_SETTER( position ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	size_t len;
	JL_CHK( JL_JsvalToNativeVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FUNCTION( Position ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(3);

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pos[0]) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pos[1]) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( position ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_POSITION, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_PROPERTY_SETTER( velocity ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	size_t len;
	JL_CHK( JL_JsvalToNativeVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	return JS_TRUE;
	JL_BAD;
}
*/
DEFINE_FUNCTION( Velocity ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(3);

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pos[0]) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pos[1]) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( velocity ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_VELOCITY, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
	JL_BAD;
}




DEFINE_PROPERTY_GETTER( remainingTime ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

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
	JL_CHK(JL_NativeToJsval(cx, pv->totalTime - secOffset, vp) );
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

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)];
	float f;
	JL_CHK( JL_JsvalToNative(cx, *vp, &f) );
	alSourcef(pv->sid, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceFloatInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
	float f;
	alGetSourcef(pv->sid, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(JL_NativeToJsval(cx, f, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( sourceIntInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	JL_CHK( JL_JsvalToNative(cx, *vp, &i) );
	alSourcei(pv->sid, param, i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceIntInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	alGetSourcei(pv->sid, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( JL_NativeToJsval(cx, i, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( sourceBoolInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
	bool b;
	JL_CHK( JL_JsvalToNative(cx, *vp, &b) );
	alSourcei(pv->sid, param, b ? AL_TRUE : AL_FALSE);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceBoolInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
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
		FUNCTION_ARGC( Position, 3 )
		FUNCTION_ARGC( Velocity, 3 )

		FUNCTION( Play )
		FUNCTION( Pause )
		FUNCTION( Stop )
		FUNCTION( Rewind )

		FUNCTION( QueueBuffers )
		FUNCTION( UnqueueBuffers )

		FUNCTION( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC

		PROPERTY_GET( position )
		PROPERTY_GET( velocity )

//		PROPERTY_WRITE_STORE( buffer )
		PROPERTY( buffer )

		PROPERTY_SET( effectSlot )
		PROPERTY_SET( directFilter )

		PROPERTY_GET( remainingTime )

		PROPERTY_SWITCH_GET( buffersQueued, sourceIntInd )
		PROPERTY_SWITCH_GET( buffersProcessed, sourceIntInd )
		PROPERTY_SWITCH_GET( state, sourceIntInd )
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
