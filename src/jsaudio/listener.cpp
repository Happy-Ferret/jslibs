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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( OalListener )

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Gets or sets the position of the listener in the 3D environment.
**/
DEFINE_PROPERTY_SETTER( position ) {

	JL_IGNORE( strict, id, obj );

	float pos[3];
	uint32_t len;
	JL_CHK( jl::getVector(cx, vp, pos, 3, &len) );

	alListener3f(AL_POSITION, pos[0], pos[1], pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( position ) {

	JL_IGNORE( id, obj );

	float pos[3];

	alGetListener3f(AL_POSITION, &pos[0], &pos[1], &pos[2]);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK( jl::setVector(cx, vp, pos, 3) );
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
  Gets or sets the unit size of the 3D environment.
**/
DEFINE_PROPERTY_SETTER( metersPerUnit ) {

	JL_IGNORE( strict, id, obj );

	float metersPerUnit;
	JL_CHK( jl::getValue(cx, vp, &metersPerUnit) );

	alListenerf(AL_METERS_PER_UNIT, metersPerUnit);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( metersPerUnit ) {

	JL_IGNORE( id, obj );

	float metersPerUnit;

	alGetListenerf(AL_METERS_PER_UNIT, &metersPerUnit);
	JL_CHK( CheckThrowCurrentOalError(cx) );

	JL_CHK(jl::setValue(cx, vp, metersPerUnit) );
	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY(position)
		PROPERTY(metersPerUnit)
	END_STATIC_PROPERTY_SPEC

END_CLASS
