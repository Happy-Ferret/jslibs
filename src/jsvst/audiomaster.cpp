#include "stdafx.h"

#include "../common/jsClass.h"

#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "audiomaster.h"



/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( AudioMaster )


DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}

DEFINE_PROPERTY( version ) {

	if ( *vp == JSVAL_VOID ) {

		audioMasterCallback audioMaster = (audioMasterCallback)JS_GetPrivate(cx, obj);
		VstIntPtr version = audioMaster(0, audioMasterVersion, 0, 0, 0, 0);
		*vp = INT_TO_JSVAL( version );
	}
	return JS_TRUE;
}


CONFIGURE_CLASS

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
	JS_SetPrivate(cx, audioMasterObject, audioMaster);
	return audioMasterObject;
}
