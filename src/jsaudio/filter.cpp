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
	ALuint filter;
};


BEGIN_CLASS( OalFilter )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		if ( alcGetCurrentContext() ) {

			alDeleteFilters(1, &pv->filter);
		}
		JS_free(cx, pv);
	}
}


DEFINE_CONSTRUCTOR() {

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));

	alGenFilters(1, &pv->filter);
	J_CHK( CheckThrowCurrentOalError(cx) );

	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( valueOf ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( pv );
	J_CHK( UIntToJsval(cx, pv->filter, J_FRVAL) );
	return JS_TRUE;
}


/**doc
=== Properties ===
**/

DEFINE_PROPERTY_SETTER( type ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int filterType;
	if ( JSVAL_IS_VOID(*vp) )
		filterType = AL_FILTER_NULL;
	else
		J_CHK( JsvalToInt(cx, *vp, &filterType) );
	alFilteri(pv->filter, AL_FILTER_TYPE, filterType);
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( type ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int filterType;
	alGetFilteri(pv->filter, AL_FILTER_TYPE, &filterType);
	J_CHK( CheckThrowCurrentOalError(cx) );

	if ( filterType == AL_FILTER_NULL )
		*vp = JSVAL_VOID;
	else
		J_CHK( IntToJsval(cx, filterType, vp) );

	return JS_TRUE;
}


DEFINE_PROPERTY_SETTER( filterFloat ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	float f;
	J_CHK( JsvalToFloat(cx, *vp, &f) );
	alFilterf(pv->filter, param, f);
	J_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( filterFloat ) {

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	ALenum param = JSVAL_TO_INT(id);
	float f;
	alGetFilterf(pv->filter, param, &f);
	J_CHK( CheckThrowCurrentOalError(cx) );
	J_CHK( FloatToJsval(cx, f, vp) );
	return JS_TRUE;
}


enum {
	lowpassGain				= AL_LOWPASS_GAIN    ,
	lowpassGainHF			= AL_LOWPASS_GAINHF	,

	highpassGain			= AL_HIGHPASS_GAIN   ,
	highpassGainLF			= AL_HIGHPASS_GAINLF ,

	bandpassGain			= AL_BANDPASS_GAIN   ,
	bandpassGainLF			= AL_BANDPASS_GAINLF ,
	bandpassGainHF			= AL_BANDPASS_GAINHF ,
};


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( valueOf, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( type )

		PROPERTY_SWITCH( lowpassGain			, filterFloat )
		PROPERTY_SWITCH( lowpassGainHF		, filterFloat )

		PROPERTY_SWITCH( highpassGain			, filterFloat )
		PROPERTY_SWITCH( highpassGainLF		, filterFloat )

		PROPERTY_SWITCH( bandpassGain			, filterFloat )
		PROPERTY_SWITCH( bandpassGainLF		, filterFloat )
		PROPERTY_SWITCH( bandpassGainHF		, filterFloat )
	END_PROPERTY_SPEC

END_CLASS
