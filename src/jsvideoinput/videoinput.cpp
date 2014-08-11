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
#include "../jslang/imagePub.h"
#include "../jslang/handlePub.h"
#include <jslibsModule.h>

//extern videoInput *vi;

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( VideoInput ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

struct Private : jl::CppAllocators {
	Private( videoInput *&vi ) : vi(vi) {}

	videoInput *&vi;
	int deviceID;
	bool flipImageY;
	bool flipImageRedBlue;
};


DEFINE_FINALIZE() {

//	if ( obj == jl::Host::getJLHost(fop->runtime()).getCachedProto(className) ) // if we are finalizing the proto
//		return;

	Private *pv;
	pv = (Private*)JL_GetPrivateFromFinalize(obj);
	if ( pv != NULL ) {

		pv->vi->stopDevice( pv->deviceID );
		delete pv;
	}
bad:
	return;
}

/**doc
$TOC_MEMBER $INAME
 $Image $INAME( deviceId, [idealWidth], [idealHeight], [idealFPS], [flipImageY = true], [flipImageRedBlue = true] )
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;

	Private *pv = NULL;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_RANGE(1,5);

	pv = new Private( jl::Host::getJLHost( cx ).moduleManager().modulePrivateT<videoInput*>( moduleId() ) );
	JL_ASSERT_ALLOC( pv );
	pv->deviceID = -1; // invalid device

	int numDevices = videoInput::listDevices(true);

	if ( !JL_ARG(1).isString() ) {
		
		JL_CHK( jl::getValue(cx, JL_ARG(1), &pv->deviceID) );
		JL_ASSERT_ARG_VAL_RANGE( pv->deviceID, 0, numDevices-1, 1 );
	} else {
	
		jl::BufString requiredDeviceName;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &requiredDeviceName) );
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
		JL_CHK( jl::getValue(cx, JL_ARG(4), &fps) );
		pv->vi->setIdealFramerate( pv->deviceID, fps ); // vi->VDList[deviceId]->requestedFrameTime;
	}
	
	if ( JL_ARG_ISDEF(2) && JL_ARG_ISDEF(3) ) {
		
		int width, height;
		JL_CHK( jl::getValue(cx, JL_ARG(2), &width) );
		JL_CHK( jl::getValue(cx, JL_ARG(3), &height) );
		pv->vi->setupDevice( pv->deviceID, width, height );
	} else {
	
		pv->vi->setupDevice( pv->deviceID ); // use default size
	}

	if ( JL_ARG_ISDEF(5) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &pv->flipImageY) );
	else
		pv->flipImageY = true;

	if ( JL_ARG_ISDEF(6) )
		JL_CHK( jl::getValue(cx, JL_ARG(1), &pv->flipImageRedBlue) );
	else
		pv->flipImageRedBlue = true;


	//	vi->setVideoSettingCameraPct(deviceId, vi->propBrightness, 100);
	// vi->setFormat(deviceId, VI_NTSC_M);

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	if ( pv ) {
		
		ASSERT( pv->vi );

		if ( pv->deviceID != -1 )
			pv->vi->stopDevice( pv->deviceID );
		delete pv;
	}
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $Image $INAME()
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_ARGC(0);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );
		
	pv->vi->stopDevice( pv->deviceID );
	
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $TYPE HANDLE $INAME()
  Passively waits for a new image through the processEvents function.
**/

struct VIEvent : public ProcessEvent2 {

	Private *pv;
	HANDLE imageEvent;
	HANDLE cancelEvent;

	bool prepareWait( JSContext *cx, JS::HandleObject obj ) {

		ResetEvent( imageEvent ); // (TBD) handle errors ?
		return true;
	}

	void startWait() {

		HANDLE events[] = { imageEvent, cancelEvent };
		DWORD status = WaitForMultipleObjects( COUNTOF( events ), events, FALSE, INFINITE );
		ASSERT( status != WAIT_FAILED );
	}

	bool cancelWait() {

		SetEvent( cancelEvent );
		return true;
	}

	bool endWait( bool *hasEvent, JSContext *cx, JS::HandleObject ) {

		*hasEvent = (WaitForSingleObject( imageEvent, 0 ) == WAIT_OBJECT_0);

		if ( *hasEvent ) {

			imageEvent = pv->vi->ImageEvent( pv->deviceID );
			cancelEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); // auto-reset

			JS::RootedValue viObj(cx, getSlot(0) );
			JS::RootedValue fct( cx );

			JL_CHK( jl::getProperty( cx, viObj, "onFocus", &fct ) );
			if ( jl::isCallable( cx, fct ) ) {

				JL_CHK( jl::callNoRval(cx, viObj, fct) );
			}
		}

		return true;
		JL_BAD;
	}

	~VIEvent() {
		
		CloseHandle( cancelEvent );
	}
};


DEFINE_FUNCTION( events ) {
	
	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	VIEvent *upe = new VIEvent();
	JL_CHK( HandleCreate( cx, upe, JL_RVAL ) );
	upe->pv = pv;

	upe->imageEvent = pv->vi->ImageEvent( pv->deviceID );
	upe->cancelEvent = CreateEvent( NULL, FALSE, FALSE, NULL ); // auto-reset
	upe->setSlot(0, JL_OBJVAL);

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $Image $INAME()
**/
DEFINE_FUNCTION( next ) {

	JL_DEFINE_ARGS;

		JL_ASSERT_ARGC(0);

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	int width = pv->vi->getWidth( pv->deviceID );
	int height = pv->vi->getHeight( pv->deviceID );
	int dataSize = pv->vi->getSize( pv->deviceID );

	if ( dataSize == 0 )
		return JS_ThrowStopIteration(cx);

	uint8_t *data;
	data = JL_NewImageObject( cx, width, height, dataSize / (width * height), TYPE_UINT8, JL_RVAL );
	JL_CHK( data );

	bool status = pv->vi->getPixels( pv->deviceID, data, pv->flipImageRedBlue, pv->flipImageY ); // blocking function
	if ( !status )
		return JS_ThrowStopIteration(cx);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasNewFrame ) {

	JL_DEFINE_PROP_ARGS;
	
	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	JL_CHK( jl::setValue( cx, vp, pv->vi->isFrameNew( pv->deviceID ) ) );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( width ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	JL_CHK( jl::setValue( cx, vp, pv->vi->getWidth( pv->deviceID ) ) );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( height ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	JL_CHK( jl::setValue( cx, vp, pv->vi->getHeight( pv->deviceID ) ) );

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( channels ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	int width = pv->vi->getWidth( pv->deviceID );
	int height = pv->vi->getHeight( pv->deviceID );
	int dataSize = pv->vi->getSize( pv->deviceID );

	JL_CHK( jl::setValue(cx, vp, dataSize / (width * height)) ); 

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( name ) {

	JL_DEFINE_PROP_ARGS;

	Private *pv;
	pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv && pv->vi );

	JL_CHK( jl::setValue(cx, vp, videoInput::getDeviceName(pv->deviceID)) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
**/
DEFINE_PROPERTY_GETTER( hasDevice ) {

	JL_DEFINE_PROP_ARGS;

	int numDevices = videoInput::listDevices(true);
	JL_CHK(jl::setValue(cx, vp, numDevices > 0) ); 
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARR $INAME $READONLY
  Contains the list of all available video devices for input.
**/
DEFINE_PROPERTY_GETTER( list ) {

	JL_DEFINE_PROP_ARGS;

	int numDevices = videoInput::listDevices(true);
	JS::RootedObject list( cx, JS_NewArrayObject( cx, numDevices ) );
	for ( int i = 0; i < numDevices; i++ ) {

		jl::setElement( cx, list, i, videoInput::getDeviceName( i ) );
	}
	vp.setObject(*list);
	return true;
bad:
	return false;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME $READONLY
  Hold the current version of NSPR.
**/
DEFINE_PROPERTY_GETTER( version ) {

	//vp.setString(JS_NewStringCopyZ(cx, JL_TOSTRING(VI_VERSION)));

	JL_CHK( jl::setValue( cx, vp, JL_TOSTRING( VI_VERSION ) ) );
	JL_CHK( jl::StoreProperty( cx, obj, id, vp, true ) );
	return true;
	JL_BAD;
}


DEFINE_ITERATOR_OBJECT() {

	JL_CHKM( !keysonly, E_NAME("for...in"), E_NOTSUPPORTED );
	return obj;
bad:
	return NULL;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_ITERATOR_OBJECT

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
