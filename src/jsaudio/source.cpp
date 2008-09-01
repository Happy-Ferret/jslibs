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

#include <AL/al.h>
#include <AL/alc.h>

#define SLOT_BUFFER 0


struct Private {

	ALuint sid;
	Queue *queue;
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


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		while ( !QueueIsEmpty(pv->queue) ) {

			jsval *pItem = (jsval*)QueuePop(pv->queue);
			JS_free(cx, pItem);
		}
		QueueDestruct(pv->queue);
		alDeleteSources(1, &pv->sid);
	}
}


DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	pv->queue = QueueConstruct();
	alGenSources(1, &pv->sid);
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
		ALenum err = alGetError();
		J_S_ASSERT_1( err == AL_NO_ERROR, "Unable to alSourceQueueBuffers (%x).", err );

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
		ALenum err = alGetError();
		J_S_ASSERT_1( err == AL_NO_ERROR, "Unable to alSourceQueueBuffers (%x).", err );

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
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Pause ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	alSourcePause(pv->sid);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Stop ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	alSourceStop(pv->sid);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( Rewind ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	alSourceRewind(pv->sid);
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}



DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( UIntToJsval(cx, pv->sid, J_FRVAL) );
	return JS_TRUE;
}



/**doc
=== Properties ===
**/


DEFINE_PROPERTY_SETTER( buffer ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );

//	J_CHK( UnprotectJsval(cx, pv->queue, *vp) );
//	J_CHK( ProtectJsval(cx, pv->queue, *vp) );
	JS_SetReservedSlot(cx, obj, SLOT_BUFFER, *vp); // just a GC protection
	
	ALint bid;
	if ( *vp == JSVAL_VOID || *vp == JSVAL_ZERO )
		bid = AL_NONE;
	else
		J_CHK( JsvalToInt(cx, *vp, &bid) ); // calls OalBuffer valueOf function

	J_S_ASSERT( alIsBuffer(bid), "Invalid buffer." );
	alSourcei(pv->sid, AL_BUFFER, bid);
	return JS_TRUE;
}


DEFINE_PROPERTY_GETTER( buffer ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALint bid;
	alGetSourcei(pv->sid, AL_BUFFER, &bid);
	J_CHK( IntToJsval(cx, bid, vp) );
	return JS_TRUE;
}

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
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}


DEFINE_PROPERTY( position ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	alGetSource3f(pv->sid, AL_POSITION, &pos[0], &pos[1], &pos[2]);
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
	*J_FRVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_PROPERTY( velocity ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float pos[3];
	alGetSource3f(pv->sid, AL_VELOCITY, &pos[0], &pos[1], &pos[2]);
	J_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( gain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	J_CHK( JsvalToFloat(cx, *vp, &gain) );
	alSourcef(pv->sid, AL_GAIN, gain);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( gain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	alGetSourcef(pv->sid, AL_GAIN, &gain);
	J_CHK( FloatToJsval(cx, gain, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( secOffset ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float offset;
	J_CHK( JsvalToFloat(cx, *vp, &offset) );
	alSourcef(pv->sid, AL_SEC_OFFSET, offset);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( secOffset ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float offset;
	alGetSourcef(pv->sid, AL_SEC_OFFSET, &offset);
	J_CHK( FloatToJsval(cx, offset, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( looping ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	bool looping;
	J_CHK( JsvalToBool(cx, *vp, &looping) );
	alSourcei(pv->sid, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( looping ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALint looping;
	alGetSourcei(pv->sid, AL_LOOPING, &looping);
	J_CHK( BoolToJsval(cx, looping == AL_TRUE, vp) );
	return JS_TRUE;
}


DEFINE_PROPERTY( state ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALint state;
	alGetSourcei(pv->sid, AL_SOURCE_STATE, &state);
	J_CHK( IntToJsval(cx, state, vp) );
	return JS_TRUE;
}


DEFINE_TRACER() {

	Private *pv = (Private*)JS_GetPrivate(trc->context, obj);
	if ( pv )
		for ( QueueCell *it = QueueBegin(pv->queue); it; it = QueueNext(it) )
			JS_CALL_VALUE_TRACER(trc, *(jsval*)QueueGetData(it), "jsstd/Buffer:bufferQueueItem");
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
		FUNCTION_FAST( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( buffer )
		PROPERTY_READ( position )
		PROPERTY_READ( velocity )
		PROPERTY( gain )
		PROPERTY( secOffset )
		PROPERTY( looping )
		PROPERTY_READ( state )
	END_PROPERTY_SPEC

END_CLASS
