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


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( OalFilter )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( pv ) {

		if ( alcGetCurrentContext() ) {

			alDeleteFilters(1, &pv->filter);
		}
		JS_free(cx, pv);
	}
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new filter object.
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );

	alGenFilters(1, &pv->filter);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME()
  Returns the internal OpenAL filter id.
**/
DEFINE_FUNCTION( valueOf ) {

	JL_DEFINE_FUNCTION_OBJ;
	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( JL_CValToJsval(cx, pv->filter, JL_RVAL) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  set the type of filter represented by the Filter object.
  * undefined
  * Oal.FILTER_NULL
  * Oal.FILTER_LOWPASS
  * Oal.FILTER_HIGHPASS
  * Oal.FILTER_BANDPASS
**/
DEFINE_PROPERTY_SETTER( type ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	int filterType;
	if ( JSVAL_IS_VOID(*vp) )
		filterType = AL_FILTER_NULL;
	else
		JL_CHK( 	JL_JsvalToCVal(cx, *vp, &filterType) );
	alFilteri(pv->filter, AL_FILTER_TYPE, filterType);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( type ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(pv);
	int filterType;
	alGetFilteri(pv->filter, AL_FILTER_TYPE, &filterType);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	if ( filterType == AL_FILTER_NULL )
		*vp = JSVAL_VOID;
	else
		JL_CHK( JL_CValToJsval(cx, filterType, vp) );

	return JS_TRUE;
	JL_BAD;
}



DEFINE_PROPERTY_SETTER( filterFloat ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	JL_CHK( JL_JsvalToCVal(cx, *vp, &f) );
	alFilterf(pv->filter, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( filterFloat ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	alGetFilterf(pv->filter, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(JL_CValToJsval(cx, f, vp) );
	return JS_TRUE;
	JL_BAD;
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


/**doc
$TOC_MEMBER (many)
 * $REAL *lowpassGain*
 * $REAL *lowpassGainHF*

 * $REAL *highpassGain*
 * $REAL *highpassGainLF*

 * $REAL *bandpassGain*
 * $REAL *bandpassGainLF*
 * $REAL *bandpassGainHF*
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC( valueOf, 0 )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( type )

		PROPERTY_SWITCH( lowpassGain		, filterFloat )
		PROPERTY_SWITCH( lowpassGainHF	, filterFloat )

		PROPERTY_SWITCH( highpassGain		, filterFloat )
		PROPERTY_SWITCH( highpassGainLF	, filterFloat )

		PROPERTY_SWITCH( bandpassGain		, filterFloat )
		PROPERTY_SWITCH( bandpassGainLF	, filterFloat )
		PROPERTY_SWITCH( bandpassGainHF	, filterFloat )
	END_PROPERTY_SPEC

END_CLASS
