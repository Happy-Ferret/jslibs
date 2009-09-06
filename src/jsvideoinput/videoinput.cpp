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

#define JSVIDEOINPUT_SLOT_DEVICEID 0

videoInput vi;

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( VideoInput ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() {

	if ( obj == *_prototype )
		return;
	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	if ( deviceIdVal == JSVAL_VOID ) // the device is already closed
		return;
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	vi.stopDevice(deviceId);
bad:
	return;
}

DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_RANGE(1,4);

	int deviceId;
	int numDevices = vi.listDevices(true);

	if ( !JSVAL_IS_STRING(JL_ARG(1)) ) {
		
		JL_CHK( JsvalToInt(cx, JL_ARG(1), &deviceId) );
		if ( deviceId >= numDevices )
			JL_REPORT_ERROR("Invalid device ID.");
	} else {
	
		const char *requiredDeviceName;
		JL_CHK( JsvalToString(cx, &JL_ARG(1), &requiredDeviceName) );
		deviceId = -1;
		for ( int i = 0; i < numDevices; i++ ) {

			if ( strstr(vi.getDeviceName(i), requiredDeviceName) != NULL ) {
				
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
		vi.setIdealFramerate(deviceId, fps); // vi.VDList[deviceId]->requestedFrameTime;
	}

	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		int width, height;
		JL_CHK( JsvalToInt(cx, JL_ARG(2), &width) );
		JL_CHK( JsvalToInt(cx, JL_ARG(3), &height) );
		vi.setupDevice(deviceId, width, height);
	} else {
	
		vi.setupDevice(deviceId); // use default size
	}

//	vi.setVideoSettingCameraPct(deviceId, vi.propBrightness, 100);
//	vi.setFormat(deviceId, VI_NTSC_M);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $Image $INAME( [flipImage] )
**/
DEFINE_FUNCTION_FAST( GetImage ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, JL_FOBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);

	int width = vi.getWidth(deviceId);
	int height = vi.getHeight(deviceId);
	int dataSize = vi.getSize(deviceId);

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

	bool status = vi.getPixels(deviceId, data, true, flipImage);
	
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

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, JL_FOBJ, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	if ( deviceIdVal == JSVAL_VOID ) // the device is already closed
		return JS_TRUE;
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	vi.stopDevice(deviceId);
	JL_CHK( JS_SetReservedSlot(cx, JL_FOBJ, JSVIDEOINPUT_SLOT_DEVICEID, JSVAL_VOID) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY( hasNewFrame ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( BoolToJsval(cx, vi.isFrameNew(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( width ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( IntToJsval(cx, vi.getWidth(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY( height ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( IntToJsval(cx, vi.getHeight(deviceId), vp) ); 
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY( name ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, JSVIDEOINPUT_SLOT_DEVICEID, &deviceIdVal) );
	JL_S_ASSERT( deviceIdVal != JSVAL_VOID, "Device closed.");
	int deviceId = JSVAL_TO_INT(deviceIdVal);
	JL_CHK( StringToJsval(cx, vi.getDeviceName(deviceId), vp) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $OBJ $INAME $READONLY
  Contains the list of all available video devices for input.
**/
DEFINE_PROPERTY( list ) {

	JSTempValueRooter tvr;
	JS_PUSH_SINGLE_TEMP_ROOT(cx, JSVAL_NULL, &tvr); // (TBD) remove this workaround. cf. bz495422 || bz397177
	JSObject *list = JS_NewObject( cx, NULL, NULL, NULL );
	tvr.u.value = OBJECT_TO_JSVAL(list);

	int numDevices = vi.listDevices(true);

	jsval value;
	int i;
	for ( i = 0; i < numDevices; i++ ) {

		value = INT_TO_JSVAL(i);
		JL_CHK( JS_SetProperty( cx, list, vi.getDeviceName(i), &value ) );
	}

	*vp = tvr.u.value;
	JS_POP_TEMP_ROOT(cx, &tvr);

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY( version ) {

	#define VIVERSIONTOSTRING(s) #s
	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, VIVERSIONTOSTRING(VI_VERSION)));
	#undef VIVERSIONTOSTRING
	return JS_TRUE;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

//	HAS_XDR
	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // JSVIDEOINPUT_SLOT_DEVICEID

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(GetImage)
		FUNCTION_FAST(Close)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(hasNewFrame)
		PROPERTY_READ(width)
		PROPERTY_READ(height)
		PROPERTY_READ(name)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(version)
		PROPERTY_READ(list)
	END_STATIC_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
