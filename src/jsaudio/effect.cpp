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
#include "oalefxapi.h"
#include "error.h"

struct Private {
	ALuint effect;
	ALuint slot;
};


BEGIN_CLASS( OalEffect )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		if ( alcGetCurrentContext() ) {

			alAuxiliaryEffectSloti(pv->slot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			alDeleteAuxiliaryEffectSlots(1, &pv->slot);
			alDeleteEffects(1, &pv->effect);
		}
		JS_free(cx, pv);
	}
}


DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	alGenEffects(1, &pv->effect);
	J_CHK( CheckThrowCurrentOalError(cx) );

	alGenAuxiliaryEffectSlots(1, &pv->slot);
	J_CHK( CheckThrowCurrentOalError(cx) );

	alAuxiliaryEffectSloti( pv->slot, AL_EFFECTSLOT_EFFECT, pv->effect );
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( UIntToJsval(cx, pv->slot, J_FRVAL) );
	return JS_TRUE;
}



/**doc
=== Properties ===
**/

DEFINE_PROPERTY_SETTER( type ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int effectType;
	J_CHK( JsvalToInt(cx, *vp, &effectType) );

	alEffecti(pv->effect, AL_EFFECT_TYPE, effectType);
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( type ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int effectType;

	alGetEffecti(pv->effect, AL_EFFECT_TYPE, &effectType);
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( IntToJsval(cx, effectType, vp) );
	return JS_TRUE;
}



DEFINE_PROPERTY_SETTER( gain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	J_CHK( JsvalToFloat(cx, *vp, &gain) );

	alAuxiliaryEffectSlotf( pv->slot, AL_EFFECTSLOT_GAIN, gain );
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( gain ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;

	alGetAuxiliaryEffectSlotf( pv->slot, AL_EFFECTSLOT_GAIN, &gain );
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( FloatToJsval(cx, gain, vp) );
	return JS_TRUE;
}



DEFINE_PROPERTY_SETTER( auto ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	bool sendAuto;
	J_CHK( JsvalToBool(cx, *vp, &sendAuto) );

	alAuxiliaryEffectSloti( pv->slot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, sendAuto ? AL_TRUE : AL_FALSE );
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( auto ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	int sendAuto;

	alGetAuxiliaryEffectSloti( pv->slot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, &sendAuto );
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( BoolToJsval(cx, sendAuto == AL_TRUE ? true : false, vp) );
	return JS_TRUE;
}



/**doc
 * $VOID $INAME( buffer )
  $H arguments
  $H OpenAL API
**/
DEFINE_FUNCTION_FAST( Test ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
/*
	ALenum err = alGetError();
	alEffectf( pv->effect, AL_REVERB_DENSITY, 1 );
*/


	#define AL_EAXREVERB_DENSITY                               0x0001
	#define AL_EAXREVERB_DIFFUSION                             0x0002
	#define AL_EAXREVERB_GAIN                                  0x0003
	#define AL_EAXREVERB_GAINHF                                0x0004
	#define AL_EAXREVERB_GAINLF                                0x0005
	#define AL_EAXREVERB_DECAY_TIME                            0x0006
	#define AL_EAXREVERB_DECAY_HFRATIO                         0x0007
	#define AL_EAXREVERB_DECAY_LFRATIO                         0x0008
	#define AL_EAXREVERB_REFLECTIONS_GAIN                      0x0009
	#define AL_EAXREVERB_REFLECTIONS_DELAY                     0x000A
	#define AL_EAXREVERB_REFLECTIONS_PAN                       0x000B
	#define AL_EAXREVERB_LATE_REVERB_GAIN                      0x000C
	#define AL_EAXREVERB_LATE_REVERB_DELAY                     0x000D
	#define AL_EAXREVERB_LATE_REVERB_PAN                       0x000E
	#define AL_EAXREVERB_ECHO_TIME                             0x000F
	#define AL_EAXREVERB_ECHO_DEPTH                            0x0010
	#define AL_EAXREVERB_MODULATION_TIME                       0x0011
	#define AL_EAXREVERB_MODULATION_DEPTH                      0x0012
	#define AL_EAXREVERB_AIR_ABSORPTION_GAINHF                 0x0013 
	#define AL_EAXREVERB_HFREFERENCE                           0x0014 
	#define AL_EAXREVERB_LFREFERENCE                           0x0015 
	#define AL_EAXREVERB_ROOM_ROLLOFF_FACTOR                   0x0016
	#define AL_EAXREVERB_DECAY_HFLIMIT                         0x0017
	#define AL_EFFECT_EAXREVERB                                0x8000

	alEffecti(pv->effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	J_CHK( CheckThrowCurrentOalError(cx) );

	EAXREVERBPROPERTIES eaxReverb = REVERB_PRESET_AUDITORIUM;
	EFXEAXREVERBPROPERTIES efxReverb;

	ConvertReverbParameters(&eaxReverb, &efxReverb);

	EFXEAXREVERBPROPERTIES *pEFXEAXReverb = &efxReverb;
	ALuint uiEffect = pv->effect;

	alEffectf(uiEffect, AL_EAXREVERB_DENSITY, pEFXEAXReverb->flDensity);
	J_CHK( CheckThrowCurrentOalError(cx) );


	alEffectf(uiEffect, AL_EAXREVERB_DIFFUSION, pEFXEAXReverb->flDiffusion);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_GAIN, pEFXEAXReverb->flGain);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_GAINHF, pEFXEAXReverb->flGainHF);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_GAINLF, pEFXEAXReverb->flGainLF);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_DECAY_TIME, pEFXEAXReverb->flDecayTime);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_DECAY_HFRATIO, pEFXEAXReverb->flDecayHFRatio);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_DECAY_LFRATIO, pEFXEAXReverb->flDecayLFRatio);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_GAIN, pEFXEAXReverb->flReflectionsGain);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_REFLECTIONS_DELAY, pEFXEAXReverb->flReflectionsDelay);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectfv(uiEffect, AL_EAXREVERB_REFLECTIONS_PAN, pEFXEAXReverb->flReflectionsPan);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_GAIN, pEFXEAXReverb->flLateReverbGain);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_LATE_REVERB_DELAY, pEFXEAXReverb->flLateReverbDelay);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectfv(uiEffect, AL_EAXREVERB_LATE_REVERB_PAN, pEFXEAXReverb->flLateReverbPan);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_ECHO_TIME, pEFXEAXReverb->flEchoTime);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_ECHO_DEPTH, pEFXEAXReverb->flEchoDepth);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_MODULATION_TIME, pEFXEAXReverb->flModulationTime);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_MODULATION_DEPTH, pEFXEAXReverb->flModulationDepth);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, pEFXEAXReverb->flAirAbsorptionGainHF);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_HFREFERENCE, pEFXEAXReverb->flHFReference);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_LFREFERENCE, pEFXEAXReverb->flLFReference);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffectf(uiEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, pEFXEAXReverb->flRoomRolloffFactor);
	J_CHK( CheckThrowCurrentOalError(cx) );
	alEffecti(uiEffect, AL_EAXREVERB_DECAY_HFLIMIT, pEFXEAXReverb->iDecayHFLimit);
	J_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( effectfloat ) {

	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( effectfloat ) {

	return JS_TRUE;
}


enum { reverbDensity };


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( valueOf, 0 )

		FUNCTION_FAST( Test )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( type )
		PROPERTY( gain )
		PROPERTY( auto )

//		PROPERTY_SWITCH( reverbDensity, effectfloat )
		PROPERTY_CREATE( reverbDensity, AL_REVERB_DENSITY, JSPROP_PERMANENT|JSPROP_SHARED, effectfloatGetter, effectfloatSetter)
	END_PROPERTY_SPEC

END_CLASS
