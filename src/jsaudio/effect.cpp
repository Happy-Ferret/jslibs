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
	ALuint slot;
};


BEGIN_CLASS( OalEffect )


DEFINE_FINALIZE() {

	LOAD_OPENAL_EXTENSION( alDeleteEffects, LPALDELETEEFFECTS );
	LOAD_OPENAL_EXTENSION( alDeleteAuxiliaryEffectSlots, LPALDELETEAUXILIARYEFFECTSLOTS );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {
		if ( alDeleteAuxiliaryEffectSlots )
			alDeleteAuxiliaryEffectSlots(1, &pv->slot);
		if ( alDeleteEffects )
			alDeleteEffects(1, &pv->effect);
		JS_free(cx, pv);
	}
}


DEFINE_CONSTRUCTOR() {

	LOAD_OPENAL_EXTENSION( alGenEffects, LPALGENEFFECTS );
	LOAD_OPENAL_EXTENSION( alGenAuxiliaryEffectSlots, LPALGENAUXILIARYEFFECTSLOTS );
	LOAD_OPENAL_EXTENSION( alAuxiliaryEffectSloti, LPALAUXILIARYEFFECTSLOTI );

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	alGenEffects(1, &pv->effect);
	alGenAuxiliaryEffectSlots(1, &pv->slot);
	alAuxiliaryEffectSloti( pv->slot, AL_EFFECTSLOT_EFFECT, pv->effect );
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
 * $VOID $INAME( buffer )
  $H arguments
  $H OpenAL API
**/
DEFINE_FUNCTION_FAST( Test ) {

//	AL_EFFECT_EAXREVERB
//	return JS_TRUE;
}


/**doc
=== Properties ===
**/


DEFINE_PROPERTY( type ) {

	LOAD_OPENAL_EXTENSION( alEffecti, LPALEFFECTI );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int effectType;
	J_CHK( JsvalToInt(cx, *vp, &effectType) );
	alEffecti(pv->effect, AL_EFFECT_TYPE, effectType);
	return JS_TRUE;
}





CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( valueOf, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE( type )
	END_PROPERTY_SPEC

END_CLASS
