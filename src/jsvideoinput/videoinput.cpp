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

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_RANGE(1,4);

	int deviceId;
	int numDevices = videoInput::listDevices(true);

	if ( !JSVAL_IS_STRING(JL_ARG(1)) ) {
		
		JL_CHK( JsvalToInt(cx, JL_ARG(1), &deviceId) );
		if ( deviceId >= numDevices )
			JL_REPORT_ERROR("Invalid device ID.");
	} else {
	
		const char *requiredDeviceName;
		JL_CHK( JsvalToString(cx, &JL_ARG(1), &requiredDeviceName) );
		deviceId = -1;
		for ( int i = 0; i < numDevices; i++ ) {

			if ( strstr(videoInput::getDeviceName(i), requiredDeviceName) != NULL ) {
				
				deviceId = i;
				break;
			}
		}
		if ( deviceId == -1 )
			JL_REPORT_ERROR("Invalid device name.");
	}

	JL_CHK( JS_SetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, INT_TO_JSVAL(deviceId)) );
	
	if ( JL_ARG_ISDEF(4) ) {

		int fps;
		JL_CHK( JsvalToInt(cx, JL_ARG(4), &fps) );
		vi->setIdealFramerate(deviceId, fps); // vi->VDList[deviceId]->requestedFrameTime;
	}

	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		int width, height;
		JL_CHK( JsvalToInt(cx, JL_ARG(2), &width) );
		JL_CHK( JsvalToInt(cx, JL_ARG(3), &height) );
		vi->setupDevice(deviceId, width, height);
	} else {
	
		vi->setupDevice(deviceId); // use default size
	}

//	vi->setVideoSettingCameraPct(deviceId, vi->propBrightness, 100);
//	vi->setFormat(deviceId, VI_NTSC_M);
	return JS_TRUE;
	JL_BAD;
}


struct MetaPollData {
	
	MetaPoll mp;
	HANDLE imageEvent;
	HANDLE cancelEvent;
	JSObject *viObj;
};

JL_STATIC_ASSERT( offsetof(MetaPollData, mp) == 0 );

static void StartPoll( volatile MetaPoll *mp ) {

	MetaPollData *mpd = (MetaPollData*)mp;

	HANDLE events[] = { mpd->imageEvent, mpd->cancelEvent };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	JL_ASSERT( status != WAIT_FAILED );
}

static bool CancelPoll( volatile MetaPoll *mp ) {

	MetaPollData *mpd = (MetaPollData*)mp;
	SetEvent(mpd->cancelEvent);
	return true;
}

static JSBool EndPoll( volatile MetaPoll *mp, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	MetaPollData *mpd = (MetaPollData*)mp;

	CloseHandle(mpd->cancelEvent);
	
	*hasEvent = WaitForSingleObject(mpd->imageEvent, 0) == WAIT_OBJECT_0;

	if ( *hasEvent ) {
	
		jsval fct, rval;
		JS_GetProperty(cx, mpd->viObj, "onImage", &fct);
		if ( JsvalIsFunction(cx, fct) )
			JL_CHK( JS_CallFunctionValue(cx, mpd->viObj, fct, 0, NULL, &rval) );
	}

	JS_RemoveRoot(cx, &mpd->viObj);
	return JS_TRUE;
bad:
	JS_RemoveRoot(cx, &mpd->viObj);
	return JS_FALSE;
}


DEFINE_FUNCTION_FAST( MetaPollable ) {
	
	JL_S_ASSERT_ARG(0);

	MetaPollData *mpd;
	JL_CHK( CreateHandle(cx, 'poll', sizeof(MetaPollData), (void**)&mpd, NULL, JL_FRVAL) );
	mpd->mp.startPoll = StartPoll;
	mpd->mp.cancelPoll = CancelPoll;
	mpd->mp.endPoll = EndPoll;

	mpd->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, JL_FOBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);

	mpd->imageEvent = vi->ImageEvent(deviceId);

	mpd->viObj = JL_FOBJ;

	JS_AddRoot(cx, &mpd->viObj);

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $Image $INAME( [flipImage] )
**/
DEFINE_FUNCTION_FAST( GetImage ) {

	JL_S_ASSERT_ARG_RANGE( 0,1 );
	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, JL_FOBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);

	int width = vi->getWidth(deviceId);
	int height = vi->getHeight(deviceId);
	int dataSize = vi->getSize(deviceId);

	if ( dataSize == 0 ) {

		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	unsigned char *data = (unsigned char *)JS_malloc(cx, dataSize);
	JL_CHK( data );

	bool flipImage;
	if ( JL_FARG_ISDEF(1) )
		JL_CHK( JsvalToBool(cx, JL_FARG(1), &flipImage) );
	else
		flipImage = true;

	bool status = vi->getPixels(deviceId, data, true, flipImage);
	
	if ( !status ) {

		JS_free(cx, (void*)data);
		*JL_FRVAL = JSVAL_FALSE;
		return JS_TRUE;
	}

	jsval blobVal;
	JL_CHK( JL_NewBlob(cx, data, dataSize, &blobVal) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, blobVal, &blobObj) );
	JL_S_ASSERT( blobObj, "Unable to create Blob object." );
	*JL_FRVAL = OBJECT_TO_JSVAL(blobObj);

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
DEFINE_FUNCTION_FAST( Close ) {

	JSObject *obj = JL_FOBJ;

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	if ( deviceIdVal == JSVAL_VOID ) // the device is already closed
		return JS_TRUE;
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	vi->stopDevice(deviceId);
	JL_CHK( JS_SetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, JSVAL_VOID) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasNewFrame ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( BoolToJsval(cx, vi->isFrameNew(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( width ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( IntToJsval(cx, vi->getWidth(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( height ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( IntToJsval(cx, vi->getHeight(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( channels ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	int width = vi->getWidth(deviceId);
	int height = vi->getHeight(deviceId);
	int dataSize = vi->getSize(deviceId);	JL_CHK( IntToJsval(cx, dataSize / (width * height), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY( name ) {

	jsval deviceIdVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( StringToJsval(cx, videoInput::getDeviceName(deviceId), vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasDevice ) {

	int numDevices = videoInput::listDevices(true);
	JL_CHK( BoolToJsval(cx, numDevices > 0, vp) ); 
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARR $INAME $READONLY
  Contains the list of all available video devices for input.
**/
DEFINE_PROPERTY( list ) {

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // (TBD) remove this workaround. cf. bz495422 || bz397177

	int numDevices = videoInput::listDevices(true);
	JSObject *list = JS_NewArrayObject(cx, numDevices, NULL);
	tvr.u.value = OBJECT_TO_JSVAL(list);
	jsval value;
	int i;
	for ( i = 0; i < numDevices; i++ ) {

		JL_CHK( StringToJsval(cx, videoInput::getDeviceName(i), &value) );
		JL_CHK( JS_SetElement(cx, list, i, &value ) );
	}

	*vp = tvr.u.value;
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_TRUE;
bad:
	JS_POP_TEMP_ROOT(cx, &tvr);
	return JS_FALSE;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY( version ) {

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, J__TOSTRING(VI_VERSION)));
	return JL_StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

//	HAS_XDR
	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // JSVIDEOINPUT_SLOT_DEVICEID

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(MetaPollable)
		FUNCTION_FAST(GetImage)
		FUNCTION_FAST(Close)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(hasNewFrame)
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(channels)
		PROPERTY_READ(name)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ( version )
		PROPERTY_READ(list)
		PROPERTY_READ(hasDevice)
	END_STATIC_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
