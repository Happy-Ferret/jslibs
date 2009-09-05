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

videoInput vi;

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 2555 $
**/
BEGIN_CLASS( VideoInput ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, obj, 0, &deviceIdVal) );
	vi.stopDevice(JSVAL_TO_INT(deviceIdVal));
bad:
	return;
}

DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG(1);

	int deviceId = -1;

	int numDevices = vi.listDevices(true);

	if ( !JSVAL_IS_STRING(JL_ARG(1)) ) {
		
		JL_CHK( JsvalToInt(cx, JL_ARG(1), &deviceId) );
		if ( deviceId >= numDevices )
			JL_REPORT_ERROR("Invalid device ID.");
	} else {
	
		const char *requiredDeviceName;
		JL_CHK( JsvalToString(cx, &JL_ARG(1), &requiredDeviceName) );
		for ( int i=0; i < numDevices; i++ ) {

			if ( strstr(vi.getDeviceName(i), requiredDeviceName) != NULL ) {
				
				deviceId = i;
				break;
			}
		}
		if ( deviceId == -1 )
			JL_REPORT_ERROR("Invalid device name.");
	}

	JL_CHK( JS_SetReservedSlot(cx, obj, 0, INT_TO_JSVAL(deviceId)) );
	
	vi.setIdealFramerate(deviceId, 60);
	vi.setupDevice(deviceId);
//	vi.setFormat(deviceId, VI_NTSC_M);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( GetImage ) {

	jsval deviceIdVal;
	JL_CHK( JS_GetReservedSlot(cx, JL_FOBJ, 0, &deviceIdVal) );
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

//	while ( !vi.isFrameNew(deviceId) )
//		Sleep(10);

	bool status = vi.getPixels(deviceId, data, true, true);
	
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

	JS_DefineProperty(cx, blobObj, "channels", INT_TO_JSVAL(3), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, blobObj, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

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



//DEFINE_XDR() {
//
//	if ( xdr->mode == JSXDR_ENCODE ) {
//
//		jsval tmp;
//		JL_CHK( JS_GetReservedSlot(xdr->cx, *objp, 0, &tmp) );
//		JS_XDRValue(xdr, &tmp);
//		return JS_TRUE;
//	}
//
//	if ( xdr->mode == JSXDR_DECODE ) {
//
//		*objp = JS_NewObject(xdr->cx, _class, NULL, NULL);
//		jsval tmp;
//		JS_XDRValue(xdr, &tmp);
//		JL_CHK( JS_SetReservedSlot(xdr->cx, *objp, 0, tmp) );
//		return JS_TRUE;
//	}
//
//	if ( xdr->mode == JSXDR_FREE ) {
//
//		return JS_TRUE;
//	}
//
//	JL_BAD;
//}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

//	HAS_XDR
	REVISION(JL_SvnRevToInt("$Revision: 2555 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // deviceID

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(GetImage)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(list)
	END_STATIC_PROPERTY_SPEC

	BEGIN_STATIC_FUNCTION_SPEC
	END_STATIC_FUNCTION_SPEC

END_CLASS
