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

#include "../common/jsClass.h"

#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "audiomaster.h"



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( AudioMaster )


DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

DEFINE_PROPERTY( version ) {

	if ( JSVAL_IS_VOID( *vp ) ) {

		audioMasterCallback audioMaster = (audioMasterCallback)JL_GetPrivate(cx, obj);
		VstIntPtr version = audioMaster(0, audioMasterVersion, 0, 0, 0, 0);
		*vp = INT_TO_JSVAL( version );
	}
	return JS_TRUE;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_HAS_INSTANCE

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ_STORE( version )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

END_CLASS


JSObject * CreateAudioMasterObject( JSContext *cx, audioMasterCallback audioMaster ) {

	JSObject *audioMasterObject = JS_NewObject(cx, classAudioMaster, NULL, NULL);
	if ( audioMasterObject == NULL )
		return NULL;
	JL_SetPrivate(cx, audioMasterObject, audioMaster);
	return audioMasterObject;
}
