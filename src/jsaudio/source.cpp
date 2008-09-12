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

#include "../common/queue.h"

#include "error.h"

#include "oalefxapi.h"


struct Private {
	ALuint sid;
	Queue *queue;
	ALuint effectSlot;

};


/*
	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
}

inline JSBool UnshiftJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)malloc(sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueueUnshift( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback
	return JS_TRUE;
}
*/


JSBool ProtectJsval( JSContext *cx, Queue *queue, jsval value ) {

	jsval *pItem = (jsval*)JS_malloc(cx, sizeof(jsval));
	J_S_ASSERT_ALLOC( pItem );
	*pItem = value;
	QueuePush( queue, pItem ); // no need to JS_AddRoot *pItem, see Tracer callback !
	return JS_TRUE;
}


JSBool UnprotectJsval( JSContext *cx, Queue *queue, jsval value ) {

	for ( QueueCell *it = QueueBegin(queue); it; it = QueueNext(it) ) {

		if ( *(jsval*)QueueGetData(it) == value ) {

			jsval *pItem = (jsval*)QueueRemoveCell(queue, it);
			JS_free(cx, pItem);
			break;
		}
	}
	return JS_TRUE;
}



BEGIN_CLASS( OalSource )

DEFINE_TRACER() {

	Private *pv = (Private*)JS_GetPrivate(trc->context, obj);
	if ( pv )
		for ( QueueCell *it = QueueBegin(pv->queue); it; it = QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		while ( !QueueIsEmpty(pv->queue) ) {

			jsval *pItem = (jsval*)QueuePop(pv->queue);
			JS_free(cx, pItem);
		}
		QueueDestruct(pv->queue);

		if ( alcGetCurrentContext() ) {

			alAuxiliaryEffectSloti(pv->effectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL); //(TBD) check if it is needed before deleting the auxiliary effect slot ?
			alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
			alDeleteSources(1, &pv->sid);
		}
	}
}


DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	pv->queue = QueueConstruct();

	alGenSources(1, &pv->sid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	alGenAuxiliaryEffectSlots(1, &pv->effectSlot);
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( buffer )
  $H arguments
   $ARG integer buffer: a Buffer Object or a buffer Id.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( QueueBuffers ) {

	J_S_ASSERT_ARG_MIN(1);

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	*J_FRVAL = JSVAL_VOID;

	if ( JSVAL_IS_INT(J_FARG(1)) ) {
		
		ALuint bid;
		J_CHK( JsvalToUInt(cx, J_FARG(1), &bid) );

		J_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );
		
		alSourceQueueBuffers( pv->sid, 1, &bid );
		J_CHK( CheckThrowCurrentOalError(cx) );

		if ( JSVAL_IS_OBJECT(J_FARG(1)) )
			J_CHK( ProtectJsval(cx, pv->queue, J_FARG(1)) );
		return JS_TRUE;
	}
	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}


/**doc
 * $VOID $INAME( buffer )
  $H arguments
   $ARG integer buffer: a Buffer Object or a buffer Id.
  $H OpenAL API
	alDeleteBuffers
**/
DEFINE_FUNCTION_FAST( UnqueueBuffers ) {

	J_S_ASSERT_ARG_MIN(1);

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	*J_FRVAL = JSVAL_VOID;

	if ( JSVAL_IS_INT(J_FARG(1)) ) {
		
		ALuint bid;
		J_CHK( JsvalToUInt(cx, J_FARG(1), &bid) );

		J_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );

		alSourceUnqueueBuffers( pv->sid, 1, &bid );
		J_CHK( CheckThrowCurrentOalError(cx) );

		if ( JSVAL_IS_OBJECT(J_FARG(1)) )
			J_CHK( UnprotectJsval(cx, pv->queue, J_FARG(1)) );
		return JS_TRUE;
	}
	J_REPORT_ERROR("Invalid argument.");
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( Play ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	alSourcePlay(pv->sid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Pause ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	alSourcePause(pv->sid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Stop ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	alSourceStop(pv->sid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Rewind ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );

	alSourceRewind(pv->sid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


/*
DEFINE_FUNCTION_FAST( Effect ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_INT(J_FARG(1));

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	ALuint send;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &send) );

	ALuint effectSlot = AL_EFFECTSLOT_NULL;
	ALuint filter = AL_FILTER_NULL;
	if ( J_FARG_ISDEF(2) )
		J_CHK( JsvalToUInt(cx, J_FARG(2), &effectSlot) );
//	if ( J_FARG_ISDEF(3) )
//		J_CHK( JsvalToUInt(cx, J_FARG(3), &filter) );

//	ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
//	ALCint numSends;
//	alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &numSends);

	J_CHK( JS_DefineProperty(cx, J_FOBJ, "effect", J_FARG(2), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT) );

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, effectSlot, send, filter);
	return JS_TRUE;
}
*/


DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( UIntToJsval(cx, pv->sid, J_FRVAL) );
	return JS_TRUE;
}



/**doc
=== Properties ===
**/

DEFINE_PROPERTY( effect ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	ALuint effect;
	if ( !JSVAL_IS_VOID(*vp) )
		J_CHK( JsvalToUInt(cx, *vp, &effect) );	
	else
		effect = AL_EFFECT_NULL;
	
	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_EFFECT, effect );
	J_CHK( CheckThrowCurrentOalError(cx) );
//		effectSlot = AL_EFFECTSLOT_NULL;

//	int tmp[10];
//	alGetSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, tmp);
//	J_CHK( CheckThrowCurrentOalError(cx) );

	alSource3i(pv->sid, AL_AUXILIARY_SEND_FILTER, pv->effectSlot, 0, AL_FILTER_NULL);
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}



DEFINE_PROPERTY_SETTER( effectGain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	J_CHK( JsvalToFloat(cx, *vp, &gain) );
	alAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, gain );
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( effectGain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	alGetAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, &gain );
	J_CHK( CheckThrowCurrentOalError(cx) );
	J_CHK( FloatToJsval(cx, gain, vp) );
	return JS_TRUE;
}



DEFINE_PROPERTY_SETTER( effectSendAuto ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	bool sendAuto;
	J_CHK( JsvalToBool(cx, *vp, &sendAuto) );
	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, sendAuto ? AL_TRUE : AL_FALSE );
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( effectSendAuto ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	int sendAuto;
	alGetAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, &sendAuto );
	J_CHK( CheckThrowCurrentOalError(cx) );
	J_CHK( BoolToJsval(cx, sendAuto == AL_TRUE ? true : false, vp) );
	return JS_TRUE;
}




DEFINE_PROPERTY( directFilter ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

	ALuint filter;
	if ( !JSVAL_IS_VOID(*vp) )
		J_CHK( JsvalToUInt(cx, *vp, &filter) );	
	else
		filter = AL_FILTER_NULL;

	alSourcei(pv->sid, AL_DIRECT_FILTER, filter);
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}


DEFINE_PROPERTY( buffer ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALint bid;
	if ( *vp == JSVAL_VOID || *vp == JSVAL_ZERO )
		bid = AL_NONE;
	else
		J_CHK( JsvalToInt(cx, *vp, &bid) ); // calls OalBuffer valueOf function
	J_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );

	alSourcei(pv->sid, AL_BUFFER, bid);
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}

/*
DEFINE_PROPERTY_GETTER( buffer ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALint bid;
	alGetSourcei(pv->sid, AL_BUFFER, &bid);
	J_CHK( IntToJsval(cx, bid, vp) );
	return JS_TRUE;
}
*/

/*
DEFINE_PROPERTY_SETTER( position ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	size_t len;
	J_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	return JS_TRUE;
}
*/


DEFINE_FUNCTION_FAST( Position ) {

	J_S_ASSERT_ARG_MIN(3);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	J_CHK( JsvalToFloat(cx, J_FARG(1), &pos[0]) );
	J_CHK( JsvalToFloat(cx, J_FARG(2), &pos[1]) );
	J_CHK( JsvalToFloat(cx, J_FARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_POSITION, pos[0], pos[1], pos[2]);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_PROPERTY( position ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_POSITION, &pos[0], &pos[1], &pos[2]);
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
}


/*
DEFINE_PROPERTY_SETTER( velocity ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	size_t len;
	J_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	return JS_TRUE;
}
*/
DEFINE_FUNCTION_FAST( Velocity ) {

	J_S_ASSERT_ARG_MIN(3);
	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	J_CHK( JsvalToFloat(cx, J_FARG(1), &pos[0]) );
	J_CHK( JsvalToFloat(cx, J_FARG(2), &pos[1]) );
	J_CHK( JsvalToFloat(cx, J_FARG(3), &pos[2]) );

	alSource3f(pv->sid, AL_VELOCITY, pos[0], pos[1], pos[2]);
	J_CHK( CheckThrowCurrentOalError(cx) );

	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_PROPERTY( velocity ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];

	alGetSource3f(pv->sid, AL_VELOCITY, &pos[0], &pos[1], &pos[2]);
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
}


static const int enumToConst[] = {
	AL_SOURCE_STATE,
	AL_LOOPING,
	AL_SEC_OFFSET,
	AL_GAIN,
	AL_AIR_ABSORPTION_FACTOR,
	AL_ROOM_ROLLOFF_FACTOR,
	AL_CONE_OUTER_GAINHF,
	AL_DIRECT_FILTER_GAINHF_AUTO,
	AL_AUXILIARY_SEND_FILTER_GAIN_AUTO,
	AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO,
};

enum {
	state								= 0,
	looping								,
	secOffset							,
	gain									,
	airAbsorptionFactor				,
	roomRolloffFactor					,
	coneOuterGainhf					,
	directFilterGainhfAuto			,
	auxiliarySendFilterGainAuto	,
	auxiliarySendFilterGainhfAuto	,
};



// 'ind' suffix mean that an indirection is needed because tinyid (8bit) cannot store any OpenAL constant.
DEFINE_PROPERTY_SETTER( sourceFloatInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)];
	float f;
	J_CHK( JsvalToFloat(cx, *vp, &f) );
	alSourcef(pv->sid, param, f);
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( sourceFloatInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	float f;
	alGetSourcef(pv->sid, param, &f);
	J_CHK( CheckThrowCurrentOalError(cx) );
	J_CHK( FloatToJsval(cx, f, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( sourceIntInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	int i;
	J_CHK( JsvalToInt(cx, *vp, &i) );
	alSourcei(pv->sid, param, i);
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( sourceIntInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	int i;
	alGetSourcei(pv->sid, param, &i);
	J_CHK( CheckThrowCurrentOalError(cx) );
	J_CHK( IntToJsval(cx, i, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( sourceBoolInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	bool b;
	J_CHK( JsvalToBool(cx, *vp, &b) );
	alSourcei(pv->sid, param, b ? AL_TRUE : AL_FALSE);
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( sourceBoolInd ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = enumToConst[JSVAL_TO_INT(id)]; // see sourceFloatInd comment.
	int i;
	alGetSourcei(pv->sid, param, &i);
	J_CHK( CheckThrowCurrentOalError(cx) );
	*vp = i == AL_TRUE ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
}



CONFIGURE_CLASS

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

//		FUNCTION_FAST_ARGC( Effect, 3 )

		FUNCTION_FAST( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( effect )
		PROPERTY( effectGain )
		PROPERTY( effectSendAuto )

		PROPERTY_WRITE_STORE( directFilter )
		PROPERTY_WRITE_STORE( buffer )

		PROPERTY_READ( position )
		PROPERTY_READ( velocity )


		PROPERTY_SWITCH_READ( state, sourceIntIndGetter )
		PROPERTY_SWITCH( looping, sourceBoolInd )
		PROPERTY_SWITCH( gain, sourceFloatInd )
		PROPERTY_SWITCH( secOffset, sourceFloatInd )
		PROPERTY_SWITCH( airAbsorptionFactor, sourceFloatInd )
		PROPERTY_SWITCH( roomRolloffFactor, sourceFloatInd )
		PROPERTY_SWITCH( coneOuterGainhf, sourceFloatInd )
		PROPERTY_SWITCH( directFilterGainhfAuto, sourceBoolInd )
		PROPERTY_SWITCH( auxiliarySendFilterGainAuto, sourceBoolInd )
		PROPERTY_SWITCH( auxiliarySendFilterGainhfAuto, sourceBoolInd )

	END_PROPERTY_SPEC

END_CLASS
