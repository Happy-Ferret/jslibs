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
	return true;
	JL_BAD;
}

inline bool UnshiftJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)jl_malloc(sizeof(jsval));
	JL_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return true;
	JL_BAD;
}
*/


bool QueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)JS_malloc(cx, sizeof(jsval));
	JL_CHK( pItem );
	*pItem = value;
	QueuePush(queue, pItem); // no need to JS_AddRoot *pItem, see Tracer callback !
	return true;
	JL_BAD;
}

/*
bool UnqueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval value ) {

	for ( jl::QueueCell *it = jl::QueueBegin(queue); it; it = jl::QueueNext(it) ) {

		if ( *(jsval*)QueueGetData(it) == value ) {

			jsval *pItem = (jsval*)QueueRemoveCell(queue, it);
			JS_free(cx, pItem);
			return true;
		}
	}
	return false; // not found
}
*/

bool UnqueueBuffersJsval( JSContext *cx, jl::Queue *queue, jsval *rval ) {

	jsval *pval = (jsval*)QueueShift(queue);
	*rval = *pval;
	JS_free(cx, pval);
	return true;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( OalSource )

DEFINE_TRACER() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( pv )
		for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

			jsval *val = (jsval*)QueueGetData(it);
			if ( !JSVAL_IS_PRIMITIVE(*val) ) {
				
				JS_CallValueTracer(trc, val, "jsstd/Buffer:bufferQueueItem");
			}
		}
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;

	if ( alcGetCurrentContext() ) {

		// alAuxiliaryEffectSloti(pv->effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
		alDeleteSources(1, &pv->sid);
	}

	if ( jl::HostRuntime::getJLRuntime( fop->runtime() ).skipCleanup() )
		return;

	while ( !QueueIsEmpty(pv->queue) ) {

		jsval *pItem = (jsval*)QueuePop(pv->queue);
		JS_freeop(fop, pItem);
	}
	jl::QueueDestruct(pv->queue);
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new source object.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_CONSTRUCTING();

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->queue = jl::QueueConstruct();
	pv->sid = 0;
	alGenSources(1, &pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	ASSERT( pv->sid ); // ensure that 0 is not a valid id, else change bad: behavior

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) {

		if ( pv->sid )
			alDeleteSources(1, &pv->sid);
		if ( pv->queue )
			jl::QueueDestruct(pv->queue);
		JS_free(cx, pv);
	}
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( buffer )
  $H arguments
   $ARG OalBuffer | BufferId: a Buffer Object or a buffer Id.
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( queueBuffers ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(1);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint bid;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &bid) );
	JL_ASSERT( alIsBuffer(bid), E_ARG, E_NUM(1), E_INVALID );

	alSourceQueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( QueueBuffersJsval(cx, pv->queue, JL_ARG(1)) );

	ALint queueSize;
	alGetSourcei(pv->sid, AL_BUFFERS_QUEUED, &queueSize);
	if ( queueSize == 1 )
		pv->totalTime = 0;
	pv->totalTime += BufferSecTime(bid);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE OalBuffer $INAME()
  $H OpenAL API
   alDeleteBuffers
**/
DEFINE_FUNCTION( unqueueBuffers ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALuint bid;
	alSourceUnqueueBuffers(pv->sid, 1, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( UnqueueBuffersJsval(cx, pv->queue, JL_RVAL ) );
	JL_SAFE(
		// ensure the oal buffer matchs the js buffer stored in the pv->queue
		ALuint tmp;
		JL_CHK( jl::getValue(cx, *JL_RVAL, &tmp) );
		JL_ASSERT( bid == tmp, E_LIB, E_INTERNAL, E_COMMENT("wrong buffer") ); // JL_ASSERT( bid == tmp, "Internal error in UnqueueBuffers()." );
	);
	pv->totalTime -= BufferSecTime(bid);
	return true;
	JL_BAD;
}



DEFINE_FUNCTION( play ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourcePlay(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

DEFINE_FUNCTION( pause ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourcePause(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

DEFINE_FUNCTION( stop ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourceStop(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

DEFINE_FUNCTION( rewind ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	alSourceRewind(pv->sid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/*
DEFINE_FUNCTION( effect ) {

	JL_ASSERT_ARGC_MIN(1);
	JL_ASSERT_ARG_IS_INTEGER(1);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALuint send;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &send) );

	ALuint effectSlot = AL_EFFECTSLOT_NULL;
	ALuint filter = AL_FILTER_NULL;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( jl::getValue(cx, JL_ARG(2), &effectSlot) );
//	if ( JL_ARG_ISDEF(3) )
//		JL_CHK( jl::getValue(cx, JL_ARG(3), &filter) );

//	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
//	ALCint numSends;
//	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &numSends);

	JL_CHK( JS_DefineProperty(cx, JL_OBJ, "effect", JL_ARG(2), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, send, filter);
	return true;
	JL_BAD;
}
*/


DEFINE_FUNCTION( valueOf ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->sid) );
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY_SETTER( effectSlot ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint effectSlot;
	if ( !vp.isUndefined() )
		JL_CHK( jl::getValue(cx, vp, &effectSlot) );
	else
		effectSlot = AL_EFFECTSLOT_NULL;

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( directFilter ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint filter;
	if ( !vp.isUndefined() )
		JL_CHK( jl::getValue(cx, vp, &filter) );
	else
		filter = AL_FILTER_NULL;

	alSourcei(pv->sid, AL_DIRECT_FILTER, filter);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( buffer ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALint bid;
	if ( vp.isUndefined() || vp.get() == JSVAL_ZERO )
		bid = AL_NONE;
	else
		JL_CHK( jl::getValue(cx, vp, &bid) ); // calls OalBuffer valueOf function

	JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JL_ASSERT( alIsBuffer(bid), E_VALUE, E_ANINVALID, E_STR("buffer") );

	alSourcei(pv->sid, AL_BUFFER, bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	pv->totalTime = BufferSecTime(bid);

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( buffer ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALint bid;
	alGetSourcei(pv->sid, AL_BUFFER, &bid);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	// look if the current value hold tby the property (_STORE) is the current buffer)
	if ( !vp.isUndefined() ) {

		ALint tmp;
		JL_CHK( jl::getValue(cx, vp, &tmp) ); // calls OalBuffer valueOf function

		JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JL_ASSERT( alIsBuffer(bid), E_VALUE, E_ANINVALID, E_STR("buffer") ); // JLSMSG_LOGIC_ERROR, "invalid buffer"

		if ( tmp == bid )
			goto out;
	}

	// find the buffer object in the list of jsval
	for ( jl::QueueCell *it = jl::QueueBegin(pv->queue); it; it = jl::QueueNext(it) ) {

		jsval *val = (jsval*)QueueGetData(it);
		ALint tmp;
		JL_CHK( jl::getValue(cx, *val, &tmp) ); // calls OalBuffer valueOf function
		JL_ASSERT( alIsBuffer(tmp), E_VALUE, E_INVALID ); // JLSMSG_LOGIC_ERROR, "invalid buffer"
		if ( tmp == bid ) {

			vp.set(*val);
			goto out;
		}
	}

	JL_ASSERT( alIsBuffer(bid), E_VALUE, E_INVALID ); // JLSMSG_LOGIC_ERROR, "invalid buffer"
	JL_CHK( jl::setValue(cx, vp, bid) );

out:
	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}



/*
DEFINE_PROPERTY_SETTER( position ) {

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	size_t len;
	JL_CHK( jl::getVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	return true;
	JL_BAD;
}
*/


DEFINE_FUNCTION( position ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(3);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	JL_CHK( jl::getValue(cx, JL_ARG(1), &pos[0]) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &pos[1]) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( position ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_POSITION, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( jl::setVector(cx, vp, pos, 3) );
	return true;
	JL_BAD;
}


/*
DEFINE_PROPERTY_SETTER( velocity ) {

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	size_t len;
	JL_CHK( jl::getVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	return true;
	JL_BAD;
}
*/
DEFINE_FUNCTION( velocity ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_MIN(3);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];
	JL_CHK( jl::getValue(cx, JL_ARG(1), &pos[0]) );
	JL_CHK( jl::getValue(cx, JL_ARG(2), &pos[1]) );
	JL_CHK( jl::getValue(cx, JL_ARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( velocity ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_VELOCITY, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( jl::setVector(cx, vp, pos, 3) );
	return true;
	JL_BAD;
}




DEFINE_PROPERTY_GETTER( remainingTime ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALint loop;
	alGetSourcei(pv->sid, AL_LOOPING, &loop);
	if ( loop == AL_TRUE ) {

		vp.set(JS_GetPositiveInfinityValue(cx));
		return true;
	}

	ALint state;
	alGetSourcei(pv->sid, AL_SOURCE_STATE, &state);
	if ( state != AL_PLAYING && state != AL_PAUSED ) {

		vp.setUndefined();
		return true;
	}

	ALfloat secOffset;
	alGetSourcef(pv->sid, AL_SEC_OFFSET, &secOffset);
	JL_CHK(jl::setValue(cx, vp, pv->totalTime - secOffset) );
	return true;
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

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)];
	float f;
	JL_CHK( jl::getValue(cx, vp, &f) );
	alSourcef(pv->sid, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceFloatInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
	float f;
	alGetSourcef(pv->sid, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(jl::setValue(cx, vp, f) );
	return true;
	JL_BAD;
}

/* (TBD) no setter ?
DEFINE_PROPERTY_SETTER( sourceIntInd ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	JL_CHK( jl::getValue(cx, *vp, &i) );
	alSourcei(pv->sid, param, i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}
*/

DEFINE_PROPERTY_GETTER( sourceIntInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	alGetSourcei(pv->sid, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( jl::setValue(cx, vp, i) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( sourceBoolInd ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
	bool b;
	JL_CHK( jl::getValue(cx, vp, &b) );
	alSourcei(pv->sid, param, b ? AL_TRUE : AL_FALSE);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( sourceBoolInd ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = enumToConst[JSID_TO_INT(id)]; // see sourceFloatInd comment.
	int i;
	alGetSourcei(pv->sid, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	vp.setBoolean(i == AL_TRUE);
	return true;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_TRACER

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( position, 3 )
		FUNCTION_ARGC( velocity, 3 )

		FUNCTION( play )
		FUNCTION( pause )
		FUNCTION( stop )
		FUNCTION( rewind )

		FUNCTION( queueBuffers )
		FUNCTION( unqueueBuffers )

		FUNCTION( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC

		PROPERTY_GETTER( position )
		PROPERTY_GETTER( velocity )

//		PROPERTY_WRITE_STORE( buffer )
		PROPERTY( buffer )

		PROPERTY_SETTER( effectSlot )
		PROPERTY_SETTER( directFilter )

		PROPERTY_GETTER( remainingTime )

		PROPERTY_SWITCH_GETTER( buffersQueued, sourceIntInd )
		PROPERTY_SWITCH_GETTER( buffersProcessed, sourceIntInd )
		PROPERTY_SWITCH_GETTER( state, sourceIntInd )
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
