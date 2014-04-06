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


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3420 $
**/
BEGIN_CLASS( OalFilter )


DEFINE_FINALIZE() {

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( !pv )
		return;

	if ( alcGetCurrentContext() )
		alDeleteFilters(1, &pv->filter);

	if ( jl::Host::getHost(fop->runtime())->canSkipCleanup )
		return;

	JS_freeop(fop, pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME()
  Creates a new filter object.
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_CONSTRUCTING();

	pv = (Private*)JS_malloc(cx, sizeof(Private));
	JL_CHK( pv );
	pv->filter = 0;
	alGenFilters(1, &pv->filter);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	ASSERT( pv->filter ); // ensure that 0 is not a valid id, else change bad: behavior

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) {

		if ( pv->filter )
			alDeleteFilters(1, &pv->filter);
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
  Returns the internal OpenAL filter id.
**/
DEFINE_FUNCTION( valueOf ) {

	JL_IGNORE( argc );

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( jl::setValue(cx, JL_RVAL, pv->filter) );
	return true;
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

	JL_IGNORE( strict, id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int filterType;
	if ( vp.isUndefined() )
		filterType = AL_FILTER_NULL;
	else
		JL_CHK( jl::getValue(cx, vp, &filterType) );
	alFilteri(pv->filter, AL_FILTER_TYPE, filterType);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( type ) {

	JL_IGNORE( id );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	int filterType;
	alGetFilteri(pv->filter, AL_FILTER_TYPE, &filterType);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	if ( filterType == AL_FILTER_NULL )
		vp.setUndefined();
	else
		JL_CHK( jl::setValue(cx, vp, filterType) );

	return true;
	JL_BAD;
}



DEFINE_PROPERTY_SETTER( filterFloat ) {

	JL_IGNORE( strict );

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	JL_CHK( jl::getValue(cx, vp, &f) );
	alFilterf(pv->filter, param, f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( filterFloat ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ALenum param = JSID_TO_INT(id);
	float f;
	alGetFilterf(pv->filter, param, &f);
	JL_CHK( CheckThrowCurrentOalError(cx) );
	JL_CHK(jl::setValue(cx, vp, f) );
	return true;
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

	REVISION(jl::SvnRevToInt("$Revision: 3420 $"))
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
