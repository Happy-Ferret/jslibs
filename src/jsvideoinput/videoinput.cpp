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

extern videoInput *vi;

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( VideoInput ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

struct Private {
	int deviceID;
	bool flipImageY;
	bool flipImageRedBlue;
};


DEFINE_FINALIZE() {

	if ( obj == JL_GetCachedProto(JL_GetHostPrivate(fop->runtime()), className) )
		return;

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	if ( pv != NULL ) {

		vi->stopDevice(pv->deviceID);
		jl_free(pv);
	}
bad:
	return;
}

/**doc
$TOC_MEMBER $INAME
 $Image $INAME( deviceId, [idealWidth], [idealHeight], [idealFPS], [flipImageY = true], [flipImageRedBlue = true] )
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1,5);

	pv = (Private*)jl_malloc(sizeof(Private));
	JL_ASSERT_ALLOC(pv);

	int numDevices = videoInput::listDevices(true);

	if ( !JSVAL_IS_STRING(JL_ARG(1)) ) {
		
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pv->deviceID) );
		JL_ASSERT_ARG_VAL_RANGE( pv->deviceID, 0, numDevices-1, 1 );
	} else {
	
		JLData requiredDeviceName;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &requiredDeviceName) );
		pv->deviceID = -1;
		for ( int i = 0; i < numDevices; i++ ) {

			if ( strstr(videoInput::getDeviceName(i), requiredDeviceName) != NULL ) {
				
				pv->deviceID = i;
				break;
			}
		}

		JL_CHKM( pv->deviceID != -1, E_ARG, E_NUM(1), E_NOTFOUND );
	}


	if ( JL_ARG_ISDEF(4) ) {

		int fps;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &fps) );
		vi->setIdealFramerate(pv->deviceID, fps); // vi->VDList[deviceId]->requestedFrameTime;
	}

	
	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		int width, height;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &width) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &height) );
		vi->setupDevice(pv->deviceID, width, height);
	} else {
	
		vi->setupDevice(pv->deviceID); // use default size
	}

	if ( JL_ARG_ISDEF(5) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pv->flipImageY) );
	else
		pv->flipImageY = true;

	if ( JL_ARG_ISDEF(6) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &pv->flipImageRedBlue) );
	else
		pv->flipImageRedBlue = true;
	

	JL_SetPrivate(JL_OBJ, pv);

	//	vi->setVideoSettingCameraPct(deviceId, vi->propBrightness, 100);
	//	vi->setFormat(deviceId, VI_NTSC_M);
	return JS_TRUE;

bad:
	jl_free(pv);
	return JS_FALSE;
}


/**doc
$TOC_MEMBER $INAME
 $Image $INAME()
**/
DEFINE_FUNCTION( close ) {

	JL_ASSERT_ARGC(0);

	JL_DEFINE_FUNCTION_OBJ;

	Private *pv;
	pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	
	vi->stopDevice(pv->deviceID);
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a new image through the processEvents function.
**/

struct VIEvent {
	ProcessEvent pe;
	HANDLE imageEvent;
	HANDLE cancelEvent;
	JSObject *obj;
};

S_ASSERT( offsetof(VIEvent, pe) == 0 );

static JSBool VIPrepareWait( volatile ProcessEvent *pe, JSContext *, JSObject *) {
	
	VIEvent *upe = (VIEvent*)pe;

	ResetEvent(upe->imageEvent); // (TBD) handle errors ?

	return JS_TRUE;
}

static void VIStartWait( volatile ProcessEvent *pe ) {

	VIEvent *upe = (VIEvent*)pe;

	HANDLE events[] = { upe->imageEvent, upe->cancelEvent };
	DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
	ASSERT( status != WAIT_FAILED );
}

static bool VICancelWait( volatile ProcessEvent *pe ) {

	VIEvent *upe = (VIEvent*)pe;

	SetEvent(upe->cancelEvent);
	return true;
}

static JSBool VIEndWait( volatile ProcessEvent *pe, bool *hasEvent, JSContext *cx, JSObject * ) {

	VIEvent *upe = (VIEvent*)pe;

	*hasEvent = (WaitForSingleObject(upe->imageEvent, 0) == WAIT_OBJECT_0);

	if ( *hasEvent ) {
	
		jsval fct, rval;
		JL_CHK( JS_GetProperty(cx, upe->obj, "onImage", &fct) );
		if ( JL_ValueIsCallable(cx, fct) ) {
		
			JL_CHK( JL_CallFunctionVA(cx, upe->obj, fct, &rval, OBJECT_TO_JSVAL(upe->obj)) );
		}
	}

	return JS_TRUE;
	JL_BAD;
}

static void VIWaitFinalize( void* data ) {
	
	VIEvent *upe = (VIEvent*)data;

	CloseHandle(upe->cancelEvent);
}


DEFINE_FUNCTION( events ) {
	
	JL_ASSERT_ARGC(0);
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	VIEvent *upe;
	JL_CHK( HandleCreate(cx, JLHID(pev), &upe, VIWaitFinalize, JL_RVAL) );
	upe->pe.prepareWait = VIPrepareWait;
	upe->pe.startWait = VIStartWait;
	upe->pe.cancelWait = VICancelWait;
	upe->pe.endWait = VIEndWait;

	upe->imageEvent = vi->ImageEvent(pv->deviceID);

	upe->obj = JL_OBJ;
	JL_CHK( SetHandleSlot(cx, *JL_RVAL, 0, OBJECT_TO_JSVAL(upe->obj)) ); // GC protection

	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $Image $INAME()
**/
DEFINE_FUNCTION( next ) {

	JL_ASSERT_ARGC(0);
	JL_DEFINE_FUNCTION_OBJ;

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	int width = vi->getWidth(pv->deviceID);
	int height = vi->getHeight(pv->deviceID);
	int dataSize = vi->getSize(pv->deviceID);

	if ( dataSize == 0 )
		return JS_ThrowStopIteration(cx);

	uint8_t *data;
	data = JL_NewByteImageObject(cx, width, height, dataSize / (width * height), JL_RVAL);
	JL_CHK( data );

	bool status = vi->getPixels(pv->deviceID, data, pv->flipImageRedBlue, pv->flipImageY); // blocking function
	if ( !status )
		return JS_ThrowStopIteration(cx);

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasNewFrame ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK(JL_NativeToJsval(cx, vi->isFrameNew(pv->deviceID), vp) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( width ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( JL_NativeToJsval(cx, vi->getWidth(pv->deviceID), vp) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( height ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( JL_NativeToJsval(cx, vi->getHeight(pv->deviceID), vp) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	int width = vi->getWidth(pv->deviceID);
	int height = vi->getHeight(pv->deviceID);
	int dataSize = vi->getSize(pv->deviceID);

	JL_CHK( JL_NativeToJsval(cx, dataSize / (width * height), vp) ); 

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_IGNORE(id);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( JL_NativeToJsval(cx, videoInput::getDeviceName(pv->deviceID), vp) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasDevice ) {

	JL_IGNORE(id, obj);

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

	JL_IGNORE(id, obj);

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

	*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, JL_TOSTRING(VI_VERSION)));
	return jl::StoreProperty(cx, obj, id, vp, true);
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( events )
		FUNCTION( next )
		FUNCTION( close )
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

END_CLASS
