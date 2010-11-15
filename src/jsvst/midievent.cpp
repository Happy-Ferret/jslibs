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

#include "jlclass.h"

#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "audiomaster.h"



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$

  midi spec: http://www.gweep.net/~prefect/eng/reference/protocol/midispec.html
**/
BEGIN_CLASS( MidiEvent )

DEFINE_FINALIZE() {

	void *pv = JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	jsval constructed;
	JL_GetReservedSlot(cx, obj, 0, &constructed);
	if ( !JSVAL_IS_VOID( constructed ) )
		JS_free(cx, pv);
}


DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_CHK( JL_SetReservedSlot(cx, obj, 0, JSVAL_TRUE) );
	VstMidiEvent *pv = (VstMidiEvent*)JS_malloc(cx, sizeof(VstMidiEvent));
	JL_CHK( pv );

	pv->byteSize = sizeof(VstMidiEvent);
	pv->type = kVstMidiType;
	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( deltaFrames ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->deltaFrames);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( deltaFrames ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	pv->deltaFrames = JSVAL_TO_INT( *vp );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( realtime ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = pv->flags & kVstMidiEventIsRealtime ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( realtime ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_BOOLEAN( *vp );
	pv->flags = JSVAL_TO_BOOLEAN( *vp ) == JS_TRUE ? pv->flags & kVstMidiEventIsRealtime : pv->flags | ~kVstMidiEventIsRealtime;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteLength ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->noteLength);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteLength ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	pv->noteLength = JSVAL_TO_INT( *vp );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteOffset ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->noteOffset);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOffset ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	pv->noteOffset = JSVAL_TO_INT( *vp );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( detune ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->detune);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( detune ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	int value = JSVAL_TO_INT( *vp );
	if ( value > 63 )
		value = 63;
	else
	if ( value < -64 )
		value = -64;
	pv->detune = JSVAL_TO_INT( *vp );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteOffVelocity ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->noteOffVelocity);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOffVelocity ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	int value = JSVAL_TO_INT( *vp );
	if ( value > 127 )
		value = 127;
	else
	if ( value < 0 )
		value = 0;
	pv->noteOffVelocity = JSVAL_TO_INT( *vp );
	return JS_TRUE;
	JL_BAD;
}


// Status byte in the range 0x80 to 0xFF. The remaining bytes of the message (ie, the data bytes, if any) will be in the range 0x00 to 0x7F.
//8 = Note Off
//9 = Note On
//A = AfterTouch (ie, key pressure)
//B = Control Change
//C = Program (patch) change
//D = Channel Pressure
//E = Pitch Wheel


DEFINE_PROPERTY_GETTER( status ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(((unsigned char) pv->midiData[0]) >> 4);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( status ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	unsigned int value = JSVAL_TO_INT( *vp );
	JL_S_ASSERT( value >= 0x8 && value <= 0xF, "Invalid status value.");
	pv->midiData[0] = (pv->midiData[0] & 0x0F) | ((value & 0x0F) << 4);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( channel ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[0] & 0x0F); // 0..15
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( channel ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	unsigned int value = JSVAL_TO_INT( *vp );
	JL_S_ASSERT( value >= 0 && value <= 15, "Invalid channel value.");
	pv->midiData[0] = (pv->midiData[0] & 0xF0) | (value & 0x0F);
	return JS_TRUE;
	JL_BAD;
}


/*
DEFINE_PROPERTY_GETTER( noteOn ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);


	*vp = INT_TO_JSVAL(pv->midiData[0] & 0x0F); // 0..15
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOn ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	unsigned int value = JSVAL_TO_INT( *vp );
	JL_S_ASSERT( value >= 0 && value <= 15, "Invalid channel value.");
	pv->midiData[0] = (pv->midiData[0] & 0xF0) | (value & 0x0F);
	return JS_TRUE;
	JL_BAD;
}
*/




DEFINE_PROPERTY_GETTER( value1 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[1]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( value1 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	int value = JSVAL_TO_INT( *vp );
	pv->midiData[1] = value;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( value2 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[2]);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( value2 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	JL_S_ASSERT_INT( *vp );
	int value = JSVAL_TO_INT( *vp );
	pv->midiData[2] = value;
	return JS_TRUE;
	JL_BAD;
}





CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)
	HAS_FINALIZE
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
		PROPERTY( deltaFrames )
		PROPERTY( realtime )
		PROPERTY( noteLength )
		PROPERTY( noteOffset )
		PROPERTY( detune )
		PROPERTY( noteOffVelocity )

		PROPERTY( status )
		PROPERTY( channel )
		PROPERTY( value1 )
		PROPERTY( value2 )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

END_CLASS



JSObject* CreateMidiEventObject( JSContext *cx, VstMidiEvent *midiEvent ) {

	JSObject *midiEventObject = JS_NewObjectWithGivenProto(cx, JL_CLASS(MidiEvent), JL_PROTOTYPE(cx, MidiEvent), NULL);
	if ( midiEventObject == NULL )
		return NULL;
	JL_SetPrivate(cx, midiEventObject, midiEvent);
	return midiEventObject;
}
