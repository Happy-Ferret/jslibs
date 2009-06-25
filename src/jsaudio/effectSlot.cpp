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
	ALuint effectSlot;
};


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2557 $
**/
BEGIN_CLASS( OalEffectSlot )

DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
	if ( alcGetCurrentContext() )
		alDeleteAuxiliaryEffectSlots(1, &pv->effectSlot);
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new effect slot object.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	alGenAuxiliaryEffectSlots(1, &pv->effectSlot);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( UIntToJsval(cx, pv->effectSlot, JL_FRVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY( effect ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );

	ALuint effect;
	if ( !JSVAL_IS_VOID(*vp) )
		JL_CHK( JsvalToUInt(cx, *vp, &effect) );
	else
		effect = AL_EFFECT_NULL;

	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_EFFECT, effect );
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( effectGain ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float gain;
	JL_CHK( JsvalToFloat(cx, *vp, &gain) );
	alAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, gain );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectGain ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	float gain;
	alGetAuxiliaryEffectSlotf( pv->effectSlot, AL_EFFECTSLOT_GAIN, &gain );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( FloatToJsval(cx, gain, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( effectSendAuto ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	bool sendAuto;
	JL_CHK( JsvalToBool(cx, *vp, &sendAuto) );
	alAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, sendAuto ? AL_TRUE : AL_FALSE );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( effectSendAuto ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	int sendAuto;
	alGetAuxiliaryEffectSloti( pv->effectSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, &sendAuto );
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK( BoolToJsval(cx, sendAuto == AL_TRUE ? true : false, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 2557 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST( valueOf )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( effect )
		PROPERTY( effectGain )
		PROPERTY( effectSendAuto )
	END_PROPERTY_SPEC

END_CLASS