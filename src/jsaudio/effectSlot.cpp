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
	ALuint effectSlot;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3455 $
**/
BEGIN_CLASS( OalEffectSlot )

DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;
	if ( alcGetCurrentContext() )
		alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
	JS_freeop(fop, pv);
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new effect slot object.
**/
DEFINE_CONSTRUCTOR() {
	
	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_CONSTRUCTING();
	
	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->effectSlot = 0;
	alGenAuxiliaryEffectSlots(1, &pv->effectSlot);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	ASSERT( pv->effectSlot ); // ensure that 0 is not a valid id, else change bad: behavior

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) {

		if ( pv->effectSlot )
			alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
		JS_free(cx, pv);
	}
	return false;
}

DEFINE_FUNCTION( valueOf ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_NativeToJsval(cx, pv->effectSlot, *JL_RVAL) );
	return true;
	JL_BAD;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY_SETTER( effect ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ALuint effect;
	if ( !vp.isUndefined() )
		JL_CHK( JL_JsvalToNative(cx, vp, &effect) );
	else
		effect = AL_EFFECT_NULL;

	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_EFFECT, effect );
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( effectGain ) {

	JL_IGNORE( strict, id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float gain;
	JL_CHK( JL_JsvalToNative(cx, vp, &gain) );
	alAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, gain );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectGain ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	float gain;
	alGetAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, &gain );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(JL_NativeToJsval(cx, gain, vp) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( effectSendAuto ) {

	JL_IGNORE( strict, id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	bool sendAuto;
	JL_CHK( JL_JsvalToNative(cx, vp, &sendAuto) );
	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, sendAuto ? AL_TRUE : AL_FALSE );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectSendAuto ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int sendAuto;
	alGetAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, &sendAuto );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(JL_NativeToJsval(cx, sendAuto == AL_TRUE ? true : false, vp) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3455 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_SETTER( effect )
		PROPERTY( effectGain )
		PROPERTY( effectSendAuto )
	END_PROPERTY_SPEC

END_CLASS
