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


BEGIN_CLASS( OalListener )


DEFINE_PROPERTY_SETTER( position ) {

	float pos[3];
	size_t len;
	J_CHK( JsvalToFloatVector(cx, *vp, pos, 3, &len) );
	alListener3f(AL_POSITION, pos[0], pos[1], pos[2]);
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( position ) {

	float pos[3];
	alGetListener3f(AL_POSITION, &pos[0], &pos[1], &pos[2]);
	J_CHK( FloatVectorToJsval(cx, pos, 3, vp) );
	return JS_TRUE;
}


CONFIGURE_CLASS

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY(position)
	END_STATIC_PROPERTY_SPEC

END_CLASS
