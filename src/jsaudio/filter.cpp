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

	ALuint filter;
};


BEGIN_CLASS( OalFilter )


DEFINE_FINALIZE() {

	LOAD_OPENAL_EXTENSION( alDeleteFilters, LPALDELETEFILTERS );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	if ( pv ) {

		if ( alDeleteFilters )
			alDeleteFilters(1, &pv->filter);

		JS_free(cx, pv);
	}
}


DEFINE_CONSTRUCTOR() {

	LOAD_OPENAL_EXTENSION( alGenFilters, LPALGENFILTERS );

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	alGenFilters(1, &pv->filter);

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


DEFINE_PROPERTY( type ) {

	LOAD_OPENAL_EXTENSION( alFilteri, LPALFILTERI );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE(pv);
	int filterType;
	J_CHK( JsvalToInt(cx, *vp, &filterType) );
	alFilteri(pv->filter, AL_FILTER_TYPE, filterType);
	return JS_TRUE;
}


DEFINE_PROPERTY( lowpassGain ) {

	LOAD_OPENAL_EXTENSION( alFilterf, LPALFILTERF );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	J_CHK( JsvalToFloat(cx, *vp, &gain) );
	ALenum err = alGetError();
	alFilterf(pv->filter, AL_LOWPASS_GAIN, gain);
	err = alGetError();

	return JS_TRUE;
}


DEFINE_PROPERTY( lowpassGainHF ) {

	LOAD_OPENAL_EXTENSION( alFilterf, LPALFILTERF );

	Private *pv = (Private*)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( pv );
	float gain;
	J_CHK( JsvalToFloat(cx, *vp, &gain) );
	alFilterf(pv->filter, AL_LOWPASS_GAINHF, gain);
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
		PROPERTY_WRITE( lowpassGain )
		PROPERTY_WRITE( lowpassGainHF )
	END_PROPERTY_SPEC

END_CLASS
