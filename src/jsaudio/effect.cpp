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
	ALuint effect;
};

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( OalEffect )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;

	if ( alcGetCurrentContext() )
		alDeleteEffects(1, &pv->effect);

	if ( JL_GetHostPrivate(fop->runtime())->canSkipCleanup )
		return;

	JS_freeop(fop, pv);
}



/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new effect object.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_CONSTRUCTING();

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->effect = 0;
	alGenEffects(1, &pv->effect);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	ASSERT( pv->effect ); // ensure that 0 is not a valid id, else change bad: behavior

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) {

		if ( pv->effect )
			alDeleteEffects(1, &pv->effect);
		JS_free(cx, pv);
	}
	return false;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns the internal OpenAL buffer id.
**/
DEFINE_FUNCTION( valueOf ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( JL_NativeToJsval(cx, pv->effect, *JL_RVAL) );
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  set the type of effect represented by the Effect object.
  * undefined
  * Oal.EFFECT_EAXREVERB
  * Oal.EFFECT_REVERB
  * Oal.EFFECT_CHORUS
  * Oal.EFFECT_DISTORTION
  * Oal.EFFECT_ECHO
  * Oal.EFFECT_FLANGER
  * Oal.EFFECT_FREQUENCY_SHIFTER
  * Oal.EFFECT_VOCAL_MORPHER
  * Oal.EFFECT_PITCH_SHIFTER
  * Oal.EFFECT_RING_MODULATOR
  * Oal.EFFECT_AUTOWAH
  * Oal.EFFECT_COMPRESSOR
  * Oal.EFFECT_EQUALIZER
**/
DEFINE_PROPERTY_SETTER( type ) {

	JL_IGNORE( strict, id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int effectType;
	JL_CHK( JL_JsvalToNative(cx, vp, &effectType) );

	alEffecti(pv->effect, AL_EFFECT_TYPE, effectType);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( type ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int effectType;

	alGetEffecti(pv->effect, AL_EFFECT_TYPE, &effectType);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( JL_NativeToJsval(cx, effectType, vp) );
	return true;
	JL_BAD;
}


/*
DEFINE_FUNCTION( test ) {

	JL_DEFINE_FUNCTION_OBJ;
	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );


	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}
*/

DEFINE_PROPERTY_SETTER( effectFloat ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	JL_CHK( JL_JsvalToNative(cx, vp, &f) );
	alEffectf(pv->effect, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectFloat ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	alGetEffectf(pv->effect, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(JL_NativeToJsval(cx, f, vp) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( effectInt ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	JL_CHK( JL_JsvalToNative(cx, vp, &i) );
	alEffecti(pv->effect, param, i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectInt ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	alGetEffecti(pv->effect, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( JL_NativeToJsval(cx, i, vp) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( effectBool ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	bool b;
	JL_CHK( JL_JsvalToNative(cx, vp, &b) );
	alEffecti(pv->effect, param, b ? AL_TRUE : AL_FALSE);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectBool ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	int i;
	alGetEffecti(pv->effect, param, &i);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	vp.setBoolean(i == AL_TRUE);
	return true;
	JL_BAD;
}


enum {
	reverbDensity             				= AL_REVERB_DENSITY					 ,
	reverbDiffusion           				= AL_REVERB_DIFFUSION				 ,
	reverbGain                				= AL_REVERB_GAIN						 ,
	reverbGainHF              				= AL_REVERB_GAINHF					 ,
	reverbDecayTime           				= AL_REVERB_DECAY_TIME				 ,
	reverbDecayHFRatio        				= AL_REVERB_DECAY_HFRATIO			 ,
	reverbReflectionsGain     				= AL_REVERB_REFLECTIONS_GAIN		 ,
	reverbReflectionsDelay    				= AL_REVERB_REFLECTIONS_DELAY		 ,
	reverbLateReverbGain      				= AL_REVERB_LATE_REVERB_GAIN		 ,
	reverbLateReverbDelay     				= AL_REVERB_LATE_REVERB_DELAY		 ,
	reverbAirAbsorptionGainHF 				= AL_REVERB_AIR_ABSORPTION_GAINHF ,
	reverbRoomRolloffFactor   				= AL_REVERB_ROOM_ROLLOFF_FACTOR	 ,
	reverbDecayHFLimit        				= AL_REVERB_DECAY_HFLIMIT			 ,

	chorusWaveform 							= AL_CHORUS_WAVEFORM ,
	chorusPhase    							= AL_CHORUS_PHASE    ,
	chorusRate     							= AL_CHORUS_RATE     ,
	chorusDepth    							= AL_CHORUS_DEPTH    ,
	chorusFeedback 							= AL_CHORUS_FEEDBACK ,
	chorusDelay    							= AL_CHORUS_DELAY    ,

	distortionEdge           				= AL_DISTORTION_EDGE           ,
	distortionGain           				= AL_DISTORTION_GAIN           ,
	distortionLowpassCutoff  				= AL_DISTORTION_LOWPASS_CUTOFF ,
	distortionEqcenter       				= AL_DISTORTION_EQCENTER       ,
	distortionEqbandwidth    				= AL_DISTORTION_EQBANDWIDTH    ,

	echoDelay    								= AL_ECHO_DELAY    ,
	echoLrdelay  								= AL_ECHO_LRDELAY  ,
	echoDamping  								= AL_ECHO_DAMPING  ,
	echoFeedback 								= AL_ECHO_FEEDBACK ,
	echoSpread   								= AL_ECHO_SPREAD   ,

	flangerWaveform 							= AL_FLANGER_WAVEFORM ,
	flangerPhase    							= AL_FLANGER_PHASE    ,
	flangerRate     							= AL_FLANGER_RATE     ,
	flangerDepth    							= AL_FLANGER_DEPTH    ,
	flangerFeedback 							= AL_FLANGER_FEEDBACK ,
	flangerDelay    							= AL_FLANGER_DELAY    ,

	frequencyShifterFrequency      		= AL_FREQUENCY_SHIFTER_FREQUENCY       ,
	frequencyShifterLeftDirection  		= AL_FREQUENCY_SHIFTER_LEFT_DIRECTION  ,
	frequencyShifterRightDirection 		= AL_FREQUENCY_SHIFTER_RIGHT_DIRECTION ,

	vocalMorpherPhonemea               	= AL_VOCAL_MORPHER_PHONEMEA               ,
	vocalMorpherPhonemeaCoarseTuning   	= AL_VOCAL_MORPHER_PHONEMEA_COARSE_TUNING ,
	vocalMorpherPhonemeb               	= AL_VOCAL_MORPHER_PHONEMEB               ,
	vocalMorpherPhonemebCoarseTuning   	= AL_VOCAL_MORPHER_PHONEMEB_COARSE_TUNING ,
	vocalMorpherWaveform               	= AL_VOCAL_MORPHER_WAVEFORM               ,
	vocalMorpherRate                   	= AL_VOCAL_MORPHER_RATE                   ,

	pitchShifterCoarseTune 					= AL_PITCH_SHIFTER_COARSE_TUNE ,
	pitchShifterFineTune   					= AL_PITCH_SHIFTER_FINE_TUNE   ,

	ringModulatorFrequency      			= AL_RING_MODULATOR_FREQUENCY       ,
	ringModulatorHighpassCutoff 			= AL_RING_MODULATOR_HIGHPASS_CUTOFF ,
	ringModulatorWaveform       			= AL_RING_MODULATOR_WAVEFORM        ,

	autowahAttackTime  						= AL_AUTOWAH_ATTACK_TIME  ,
	autowahReleaseTime 						= AL_AUTOWAH_RELEASE_TIME ,
	autowahResonance   						= AL_AUTOWAH_RESONANCE    ,
	autowahPeakGain    						= AL_AUTOWAH_PEAK_GAIN    ,

	compressorOnoff     						= AL_COMPRESSOR_ONOFF ,

	equalizerLowGain    						= AL_EQUALIZER_LOW_GAIN	   ,
	equalizerLowCutoff  						= AL_EQUALIZER_LOW_CUTOFF  ,
	equalizerMid1Gain   						= AL_EQUALIZER_MID1_GAIN   ,
	equalizerMid1Center 						= AL_EQUALIZER_MID1_CENTER ,
	equalizerMid1Width  						= AL_EQUALIZER_MID1_WIDTH  ,
	equalizerMid2Gain   						= AL_EQUALIZER_MID2_GAIN   ,
	equalizerMid2Center 						= AL_EQUALIZER_MID2_CENTER ,
	equalizerMid2Width  						= AL_EQUALIZER_MID2_WIDTH  ,
	equalizerHighGain   						= AL_EQUALIZER_HIGH_GAIN   ,
	equalizerHighCutoff 						= AL_EQUALIZER_HIGH_CUTOFF ,
};


/**doc
$TOC_MEMBER (many)
 * $REAL *reverbDensity*
 * $REAL *reverbDiffusion*
 * $REAL *reverbGain*
 * $REAL *reverbGainHF*
 * $REAL *reverbDecayTime*
 * $REAL *reverbDecayHFRatio*
 * $REAL *reverbReflectionsGain*
 * $REAL *reverbReflectionsDelay*
 * $REAL *reverbLateReverbGain*
 * $REAL *reverbLateReverbDelay*
 * $REAL *reverbAirAbsorptionGainHF*
 * $REAL *reverbRoomRolloffFactor*
 * $BOOL *reverbDecayHFLimit*

 * $INT *chorusWaveform*
 * $INT *chorusPhase*
 * $REAL *chorusRate*
 * $REAL *chorusDepth*
 * $REAL *chorusFeedback*
 * $REAL *chorusDelay*

 * $REAL *distortionEdge*
 * $REAL *distortionGain*
 * $REAL *distortionLowpassCutoff*
 * $REAL *distortionEqcenter*
 * $REAL *distortionEqbandwidth*

 * $REAL *echoDelay*
 * $REAL *echoLrdelay*
 * $REAL *echoDamping*
 * $REAL *echoFeedback*
 * $REAL *echoSpread*

 * $INT *flangerWaveform*
 * $REAL *flangerPhase*
 * $REAL *flangerRate*
 * $REAL *flangerDepth*
 * $REAL *flangerFeedback*
 * $REAL *flangerDelay*

 * $REAL *frequencyShifterFrequency*
 * $INT *frequencyShifterLeftDirection*
 * $INT *frequencyShifterRightDirection*

 * $INT *vocalMorpherPhonemea*
 * $INT *vocalMorpherPhonemeaCoarseTuning*
 * $INT *vocalMorpherPhonemeb*
 * $INT *vocalMorpherPhonemebCoarseTuning*
 * $INT *vocalMorpherWaveform*
 * $REAL *vocalMorpherRate*

 * $INT *pitchShifterCoarseTune*
 * $INT *pitchShifterFineTune*

 * $REAL *ringModulatorFrequency*
 * $REAL *ringModulatorHighpassCutoff*
 * $INT *ringModulatorWaveform*

 * $REAL *autowahAttackTime*
 * $REAL *autowahReleaseTime*
 * $REAL *autowahResonance*
 * $REAL *autowahPeakGain*

 * $BOOL *compressorOnoff*

 * $REAL *equalizerLowGain*
 * $REAL *equalizerLowCutoff*
 * $REAL *equalizerMid1Gain*
 * $REAL *equalizerMid1Center*
 * $REAL *equalizerMid1Width*
 * $REAL *equalizerMid2Gain*
 * $REAL *equalizerMid2Center*
 * $REAL *equalizerMid2Width*
 * $REAL *equalizerHighGain*
 * $REAL *equalizerHighCutoff*
**/

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( valueOf, 0 )
//		FUNCTION( test )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( type )

		PROPERTY_SWITCH( reverbDensity							, effectFloat )
		PROPERTY_SWITCH( reverbDiffusion							, effectFloat )
		PROPERTY_SWITCH( reverbGain								, effectFloat )
		PROPERTY_SWITCH( reverbGainHF								, effectFloat )
		PROPERTY_SWITCH( reverbDecayTime							, effectFloat )
		PROPERTY_SWITCH( reverbDecayHFRatio						, effectFloat )
		PROPERTY_SWITCH( reverbReflectionsGain					, effectFloat )
		PROPERTY_SWITCH( reverbReflectionsDelay				, effectFloat )
		PROPERTY_SWITCH( reverbLateReverbGain					, effectFloat )
		PROPERTY_SWITCH( reverbLateReverbDelay					, effectFloat )
		PROPERTY_SWITCH( reverbAirAbsorptionGainHF			, effectFloat )
		PROPERTY_SWITCH( reverbRoomRolloffFactor				, effectFloat )
		PROPERTY_SWITCH( reverbDecayHFLimit						, effectBool  )

		PROPERTY_SWITCH( chorusWaveform 							, effectInt   ) // AL_CHORUS_WAVEFORM_SINUSOID, AL_CHORUS_WAVEFORM_TRIANGLE
		PROPERTY_SWITCH( chorusPhase    							, effectInt   ) // -180, +180
		PROPERTY_SWITCH( chorusRate     							, effectFloat )
		PROPERTY_SWITCH( chorusDepth    							, effectFloat )
		PROPERTY_SWITCH( chorusFeedback 							, effectFloat )
		PROPERTY_SWITCH( chorusDelay    							, effectFloat )

		PROPERTY_SWITCH( distortionEdge          				, effectFloat )
		PROPERTY_SWITCH( distortionGain          				, effectFloat )
		PROPERTY_SWITCH( distortionLowpassCutoff 				, effectFloat )
		PROPERTY_SWITCH( distortionEqcenter      				, effectFloat )
		PROPERTY_SWITCH( distortionEqbandwidth   				, effectFloat )

		PROPERTY_SWITCH( echoDelay    							, effectFloat )
		PROPERTY_SWITCH( echoLrdelay  							, effectFloat )
		PROPERTY_SWITCH( echoDamping  							, effectFloat )
		PROPERTY_SWITCH( echoFeedback 							, effectFloat )
		PROPERTY_SWITCH( echoSpread   							, effectFloat )

		PROPERTY_SWITCH( flangerWaveform 						, effectInt ) // AL_CHORUS_WAVEFORM_SINUSOID, AL_CHORUS_WAVEFORM_TRIANGLE
		PROPERTY_SWITCH( flangerPhase    						, effectFloat )
		PROPERTY_SWITCH( flangerRate     						, effectFloat )
		PROPERTY_SWITCH( flangerDepth    						, effectFloat )
		PROPERTY_SWITCH( flangerFeedback 						, effectFloat )
		PROPERTY_SWITCH( flangerDelay    						, effectFloat )

		PROPERTY_SWITCH( frequencyShifterFrequency      	, effectFloat )
		PROPERTY_SWITCH( frequencyShifterLeftDirection  	, effectInt ) // Down, Up, Off
		PROPERTY_SWITCH( frequencyShifterRightDirection 	, effectInt ) // Down, Up, Off

		PROPERTY_SWITCH( vocalMorpherPhonemea              , effectInt )
		PROPERTY_SWITCH( vocalMorpherPhonemeaCoarseTuning  , effectInt )
		PROPERTY_SWITCH( vocalMorpherPhonemeb              , effectInt )
		PROPERTY_SWITCH( vocalMorpherPhonemebCoarseTuning  , effectInt )
		PROPERTY_SWITCH( vocalMorpherWaveform              , effectInt ) // Sin, Triangle, Saw
		PROPERTY_SWITCH( vocalMorpherRate                  , effectFloat )

		PROPERTY_SWITCH( pitchShifterCoarseTune 				, effectInt )
		PROPERTY_SWITCH( pitchShifterFineTune   				, effectInt )

		PROPERTY_SWITCH( ringModulatorFrequency      		, effectFloat )
		PROPERTY_SWITCH( ringModulatorHighpassCutoff 		, effectFloat )
		PROPERTY_SWITCH( ringModulatorWaveform       		, effectInt ) // Sin, Saw, Square

		PROPERTY_SWITCH( autowahAttackTime  					, effectFloat )
		PROPERTY_SWITCH( autowahReleaseTime 					, effectFloat )
		PROPERTY_SWITCH( autowahResonance   					, effectFloat )
		PROPERTY_SWITCH( autowahPeakGain    					, effectFloat )

		PROPERTY_SWITCH( compressorOnoff     					, effectBool )

		PROPERTY_SWITCH( equalizerLowGain    					, effectFloat )
		PROPERTY_SWITCH( equalizerLowCutoff  					, effectFloat )
		PROPERTY_SWITCH( equalizerMid1Gain   					, effectFloat )
		PROPERTY_SWITCH( equalizerMid1Center 					, effectFloat )
		PROPERTY_SWITCH( equalizerMid1Width  					, effectFloat )
		PROPERTY_SWITCH( equalizerMid2Gain   					, effectFloat )
		PROPERTY_SWITCH( equalizerMid2Center 					, effectFloat )
		PROPERTY_SWITCH( equalizerMid2Width  					, effectFloat )
		PROPERTY_SWITCH( equalizerHighGain   					, effectFloat )
		PROPERTY_SWITCH( equalizerHighCutoff 					, effectFloat )

	END_PROPERTY_SPEC

END_CLASS
