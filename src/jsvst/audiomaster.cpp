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

#include <public.sdk/source/vst2.x/audioeffectx.h>
#include "audiomaster.h"



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3455 $
**/
BEGIN_CLASS( AudioMaster )

/*
DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}
*/

DEFINE_PROPERTY_GETTER( version ) {

	audioMasterCallback audioMaster = (audioMasterCallback)JL_GetPrivate(obj);
	VstIntPtr version = audioMaster(0, audioMasterVersion, 0, 0, 0, 0);
	*vp = INT_TO_JSVAL( version );
	return jl::StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3455 $"))
	HAS_PRIVATE

	IS_UNCONSTRUCTIBLE

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( version )
	END_STATIC_PROPERTY_SPEC

END_CLASS


JSObject * CreateAudioMasterObject( JSContext *cx, audioMasterCallback audioMaster ) {

	JSObject *audioMasterObject = JL_NewObjectWithGivenProto(cx, JL_CLASS(AudioMaster), JL_CLASS_PROTOTYPE(cx, AudioMaster), NULL);
	if ( audioMasterObject == NULL )
		return NULL;
	JL_SetPrivate(cx, audioMasterObject, audioMaster);
	return audioMasterObject;
}
