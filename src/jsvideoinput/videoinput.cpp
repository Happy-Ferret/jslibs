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
#include <videoinput.h>

#include "../jslang/handlePub.h"

#define JSVIDEOINPUT_SLOT_DEVICEID 0

extern videoInput *vi;

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( VideoInput ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() {

	if ( obj == JL_THIS_PROTOTYPE )
		return;

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	if ( deviceIdVal == JSVAL_VOID ) // the device is already closed
		return;
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	vi->stopDevice(deviceId);
bad:
	return;
}

/**doc
$TOC_MEMBER $INAME
 $Image $INAME( deviceId, [idealWidth], [idealWidth], [idealFPS] )
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1,4);

	int deviceId;
	int numDevices = videoInput::listDevices(true);

	if ( !JSVAL_IS_STRING(JL_ARG(1)) ) {
		
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &deviceId) );
		JL_ASSERT_ARG_VAL_RANGE( deviceId, 0, numDevices-1, 1 );
	} else {
	
		JLStr requiredDeviceName;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &requiredDeviceName) );
		deviceId = -1;
		for ( int i = 0; i < numDevices; i++ ) {

			if ( strstr(videoInput::getDeviceName(i), requiredDeviceName) != NULL ) {
				
				deviceId = i;
				break;
			}
		}

		JL_CHKM( deviceId != -1, E_ARG, E_NUM(1), E_NOTFOUND );
	}

	JL_CHK( JL_SetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, INT_TO_JSVAL(deviceId)) );
	
	if ( JL_ARG_ISDEF(4) ) {

		int fps;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &fps) );
		vi->setIdealFramerate(deviceId, fps); // vi->VDList[deviceId]->requestedFrameTime;
	}

	
	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		int width, height;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &width) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &height) );
		vi->setupDevice(deviceId, width, height);
	} else {
	
		vi->setupDevice(deviceId); // use default size
	}

//	vi->setVideoSettingCameraPct(deviceId, vi->propBrightness, 100);
//	vi->setFormat(deviceId, VI_NTSC_M);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a new image through the ProcessEvents function.
**/

struct UserProcessEvent {
	
	ProcessEvent pe;
	HANDLE imageEvent;
	HANDLE cancelEvent;
	JSObject *obj;
};

S_ASSERT( offsetof(UserProcessEvent, pe) == 0 );

void VIStartWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	HANDLE events[] = { upe->imageEvent, upe->cancelEvent };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	ASSERT( status != WAIT_FAILED );
}

bool VICancelWait( volatile ProcessEvent *pe ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	SetEvent(upe->cancelEvent);
	return true;
}

JSBool VIEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	UserProcessEvent *upe = (UserProcessEvent*)pe;

	CloseHandle(upe->cancelEvent);
	
	*hasEvent = WaitForSingleObject(upe->imageEvent, 0) == WAIT_OBJECT_0;

	if ( *hasEvent ) {
	
		jsval fct, argv[2];
		argv[1] = OBJECT_TO_JSVAL(upe->obj);

		JL_CHK( JS_GetProperty(cx, upe->obj, "onImage", &fct) );
		if ( JL_ValueIsCallable(cx, fct) )
			JL_CHK( JS_CallFunctionValue(cx, upe->obj, fct, COUNTOF(argv)-1, argv+1, argv) );
	}

	return JS_TRUE;
bad:
	return JS_FALSE;
}

DEFINE_FUNCTION( Events ) {
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(0);

	UserProcessEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), sizeof(UserProcessEvent), (void**)&upe, NULL, JL_RVAL) );
	upe->pe.startWait = VIStartWait;
	upe->pe.cancelWait = VICancelWait;
	upe->pe.endWait = VIEndWait;

	upe->cancelEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );

	int deviceId = JSVAL_TO_INT(deviceIdVal);

	upe->imageEvent = vi->ImageEvent(deviceId);

	upe->obj = JL_OBJ;

	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, OBJECT_TO_JSVAL(upe->obj)) ); // GC protection

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $Image $INAME( [flipImage] )
**/
DEFINE_FUNCTION( GetImage ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_RANGE( 0,1 );
	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, JL_OBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);

	int width = vi->getWidth(deviceId);
	int height = vi->getHeight(deviceId);
	int dataSize = vi->getSize(deviceId);

	if ( dataSize == 0 ) {

		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	unsigned char *data = (unsigned char *)JS_malloc(cx, dataSize +1);
	JL_CHK( data );

	bool flipImage;
	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &flipImage) );
	else
		flipImage = true;

	bool status = vi->getPixels(deviceId, data, true, flipImage);
	
	if ( !status ) {

		JS_free(cx, (void*)data);
		*JL_RVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	data[dataSize] = 0;
	JL_CHK( JL_NewBlob(cx, data, dataSize, JL_RVAL) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	JL_ASSERT( blobObj, E_STR("Blob"), E_CREATE );
	*JL_RVAL = OBJECT_TO_JSVAL(blobObj);

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(dataSize / (width * height)), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $Image $INAME()
**/
DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ;
	*JL_RVAL = JSVAL_VOID;

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	if ( deviceIdVal == JSVAL_VOID ) { // the device is already closed
		
		JL_WARN( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED);
		return JS_TRUE;
	}
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	vi->stopDevice(deviceId);
	JL_CHK( JL_SetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, JSVAL_VOID) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasNewFrame ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK(JL_NativeToJsval(cx, vi->isFrameNew(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( width ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( JL_NativeToJsval(cx, vi->getWidth(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( height ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( JL_NativeToJsval(cx, vi->getHeight(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( channels ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	int width = vi->getWidth(deviceId);
	int height = vi->getHeight(deviceId);
	int dataSize = vi->getSize(deviceId);
	JL_CHK( JL_NativeToJsval(cx, dataSize / (width * height), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( name ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_ASSERT( deviceIdVal != JSVAL_VOID, E_THISOBJ, E_CLOSED );
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( JL_NativeToJsval(cx, videoInput::getDeviceName(deviceId), vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasDevice ) {

	int numDevices = videoInput::listDevices(true);
	JL_CHK(JL_NativeToJsval(cx, numDevices > 0, vp) ); 
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARR $INAME $READONLY
  Contains the list of all available video devices for input.
**/
DEFINE_PROPERTY_GETTER( list ) {

	int numDevices = videoInput::listDevices(true);
	JSObject *list = JS_NewArrayObject(cx, numDevices, NULL);

	jsval value;
	int i;
	for ( i = 0; i < numDevices; i++ ) {

		JL_CHK( JL_NativeToJsval(cx, videoInput::getDeviceName(i), &value) );
		JL_CHK( JL_SetElement(cx, list, i, &value ) );
	}

	*vp = OBJECT_TO_JSVAL( list );
	return JS_TRUE;
bad:
	return JS_FALSE;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY_GETTER( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, J__TOSTRING(VI_VERSION)));
	return JL_StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(JL_SvnRevToInt("$Revision$"))
//	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // JSVIDEOINPUT_SLOT_DEVICEID

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Events )
		FUNCTION( GetImage )
		FUNCTION( Close )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( hasNewFrame )
		PROPERTY_GETTER( width )
		PROPERTY_GETTER( height )
		PROPERTY_GETTER( channels )
		PROPERTY_GETTER( name )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( version )
		PROPERTY_GETTER( list )
		PROPERTY_GETTER( hasDevice )
	END_STATIC_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
