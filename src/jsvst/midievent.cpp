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

#include <public.sdk/source/vst2.x/audioeffectx.h>
#include "audiomaster.h"


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3487 $

  midi spec: http://www.gweep.net/~prefect/eng/reference/protocol/midispec.html
**/
BEGIN_CLASS( MidiEvent )

DEFINE_FINALIZE() {

	void *pv = JL_GetPrivate(obj);
	if ( !pv )
		return;

	jsval constructed;
	JL_GetReservedSlot( obj, 0, &constructed);
	if ( !constructed.isUndefined() )
		JL_freeop(fop, pv);
}


DEFINE_CONSTRUCTOR() {

	VstMidiEvent *pv = NULL;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_CHK( JL_SetReservedSlot( obj, 0, JSVAL_TRUE) );
	pv = (VstMidiEvent*)JS_malloc(cx, sizeof(VstMidiEvent));
	JL_CHK( pv );

	pv->byteSize = sizeof(VstMidiEvent);
	pv->type = kVstMidiType;

	JL_SetPrivate(obj, pv);
	return true;

bad:
	JS_free(cx, pv);
	return false;
}


DEFINE_PROPERTY_GETTER( deltaFrames ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->deltaFrames);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( deltaFrames ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	pv->deltaFrames = vp.toInt32();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( realtime ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = BOOLEAN_TO_JSVAL( pv->flags & kVstMidiEventIsRealtime );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( realtime ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_BOOLEAN(*vp, "");
	pv->flags = vp.toBoolean() ? pv->flags & kVstMidiEventIsRealtime : pv->flags | ~kVstMidiEventIsRealtime;
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteLength ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->noteLength);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteLength ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	pv->noteLength = vp.toInt32();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteOffset ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->noteOffset);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOffset ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	pv->noteOffset = vp.toInt32();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( detune ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->detune);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( detune ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	int value = vp.toInt32();
	if ( value > 63 )
		value = 63;
	else
	if ( value < -64 )
		value = -64;
	pv->detune = vp.toInt32();
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( noteOffVelocity ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->noteOffVelocity);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOffVelocity ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	int value = vp.toInt32();
	if ( value > 127 )
		value = 127;
	else
	if ( value < 0 )
		value = 0;
	pv->noteOffVelocity = vp.toInt32();
	return true;
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

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(((unsigned char) pv->midiData[0]) >> 4);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( status ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	unsigned int value = vp.toInt32();
	JL_ASSERT( value >= 0x8 && value <= 0xF, E_VALUE, E_RANGE, E_INTERVAL_NUM(0x8, 0xF) ); // "Invalid status value."
	pv->midiData[0] = (pv->midiData[0] & 0x0F) | ((value & 0x0F) << 4);
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( channel ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[0] & 0x0F); // 0..15
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( channel ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	unsigned int value = vp.toInt32();
	JL_ASSERT( value >= 0 && value <= 15, E_VALUE, E_RANGE, E_INTERVAL_NUM(0, 15) ); // "Invalid channel value."
	pv->midiData[0] = (pv->midiData[0] & 0xF0) | (value & 0x0F);
	return true;
	JL_BAD;
}


/*
DEFINE_PROPERTY_GETTER( noteOn ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);


	*vp = INT_TO_JSVAL(pv->midiData[0] & 0x0F); // 0..15
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( noteOn ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	unsigned int value = vp.toInt32();
	JL_ASSERT( value >= 0 && value <= 15, "Invalid channel value.");
	pv->midiData[0] = (pv->midiData[0] & 0xF0) | (value & 0x0F);
	return true;
	JL_BAD;
}
*/




DEFINE_PROPERTY_GETTER( value1 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[1]);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( value1 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	int value = vp.toInt32();
	pv->midiData[1] = value;
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( value2 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = INT_TO_JSVAL(pv->midiData[2]);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( value2 ) {

	VstMidiEvent *pv = (VstMidiEvent*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_IS_INTEGER(*vp, "");
	int value = vp.toInt32();
	pv->midiData[2] = value;
	return true;
	JL_BAD;
}





CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3487 $"))
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

END_CLASS



JSObject* CreateMidiEventObject( JSContext *cx, VstMidiEvent *midiEvent ) {

	JSObject *midiEventObject = jl::newObjectWithGivenProto(cx, JL_CLASS(MidiEvent), JL_CLASS_PROTOTYPE(cx, MidiEvent));
	if ( midiEventObject == NULL )
		return NULL;
	JL_SetPrivate( midiEventObject, midiEvent);
	return midiEventObject;
}
