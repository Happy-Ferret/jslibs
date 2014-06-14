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
#include "error.h"
#include <jsvalserializer.h>


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3524 $
**/
BEGIN_CLASS( WinError )

DEFINE_PROPERTY_GETTER( code ) {

	JL_IGNORE(id);

	JS::RootedValue hi(cx);
	JS::RootedValue lo(cx);
	JL_GetReservedSlot( obj, SLOT_WIN_ERROR_CODE_HI, &hi );
	JL_GetReservedSlot( obj, SLOT_WIN_ERROR_CODE_LO, &lo );
	JL_ASSERT_THIS_OBJECT_STATE(JSVAL_IS_INT(hi) && JSVAL_IS_INT(lo));
	JL_CHK( jl::setValue(cx, vp, (DWORD)MAKELONG(lo.toInt32(), hi.toInt32())) );
	return true;
	JL_BAD;
}


const char *ErrorToConstName( DWORD err ) {

	switch ( err ) {
		#define DEF( NAME ) case NAME: return #NAME;
		#include "errorNamesCase.h"
		#undef DEF
		default:
			return NULL;
	}
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_IGNORE(id);

	JS::RootedValue hi(cx);
	JS::RootedValue lo(cx);
	JL_GetReservedSlot(  obj, SLOT_WIN_ERROR_CODE_HI, &hi );
	JL_GetReservedSlot(  obj, SLOT_WIN_ERROR_CODE_LO, &lo );
	JL_ASSERT_THIS_OBJECT_STATE(JSVAL_IS_INT(hi) && JSVAL_IS_INT(lo));

	vp.setString( JS_NewStringCopyZ(cx, ErrorToConstName( (DWORD)MAKELONG(lo.toInt32(), hi.toInt32()) )) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( text ) {

	JL_IGNORE(id);

	JS::RootedValue hi(cx);
	JS::RootedValue lo(cx);
	JL_GetReservedSlot(  obj, SLOT_WIN_ERROR_CODE_HI, &hi );
	JL_GetReservedSlot(  obj, SLOT_WIN_ERROR_CODE_LO, &lo );
	JL_ASSERT_THIS_OBJECT_STATE(JSVAL_IS_INT(hi) && JSVAL_IS_INT(lo));
	DWORD err = (DWORD)MAKELONG(lo.toInt32(), hi.toInt32());

	LPVOID lpvMessageBuffer;
	DWORD res = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&lpvMessageBuffer, 0, NULL);
	if ( res == 0 )
		return false;

	if ( ((char*)lpvMessageBuffer)[res-2] == '\r' ) { // remove the trailing CRLF

		res -= 2;
		((char*)lpvMessageBuffer)[res] = '\0';
	}

	vp.setString(JS_NewStringCopyN( cx, (char*)lpvMessageBuffer, res+1 )); // doc: If the function succeeds, the return value is the number of TCHARs stored in the output buffer, excluding the terminating null character.
	LocalFree(lpvMessageBuffer);
	return true;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;
	
	{

	JS::RootedObject rtobj(cx, JL_OBJ);
	JS::RootedId rtid(cx, JSID_EMPTY);
	
	if ( !_textGetter(cx, rtobj, rtid, JL_RVAL) )
		return false;
	return true;
	
	}

	JL_BAD;
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	JL_CHK( jl::getProperty(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( jl::getProperty(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( jl::getSlot( cx, JL_OBJ, SLOT_WIN_ERROR_CODE_HI, JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );
	JL_CHK( jl::getSlot( cx, JL_OBJ, SLOT_WIN_ERROR_CODE_LO, JL_RVAL) );
	JL_CHK( ser->Write(cx, JL_RVAL) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_ARGS;
		JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, fileName), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JS_SetPropertyById(cx, JL_OBJ, JLID(cx, lineNumber), JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, SLOT_WIN_ERROR_CODE_HI, JL_RVAL) );
	JL_CHK( unser->Read(cx, JL_RVAL) );
	JL_CHK( JL_SetReservedSlot( JL_OBJ, SLOT_WIN_ERROR_CODE_LO, JL_RVAL) );

	return true;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_RESERVED_SLOTS(2)

	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
		PROPERTY_GETTER( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
		FUNCTION_ARGC(_serialize, 1)
		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE bool FASTCALL
WinNewError( JSContext *cx, DWORD errorCode, JS::MutableHandleValue rval ) {

	// (TBD) understand why it must have a constructor to be throwed in an exception
	JS::RootedObject error(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(WinError), JL_CLASS_PROTOTYPE(cx, WinError) ));
	rval.setObject(*error);
	JL_CHK( jl::setSlot(cx, error, SLOT_WIN_ERROR_CODE_HI, HIWORD(errorCode)) );
	JL_CHK( jl::setSlot(cx, error, SLOT_WIN_ERROR_CODE_LO, LOWORD(errorCode)) );
	return true;
	JL_BAD;
}

NEVER_INLINE bool FASTCALL
WinThrowError( JSContext *cx, DWORD errorCode ) {

//	JL_SAFE(	JS_ReportWarning( cx, "WinError exception" ) );
	JS::RootedObject error(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(WinError), JL_CLASS_PROTOTYPE(cx, WinError) )); // (TBD) understand why it must have a constructor to be throwed in an exception
//	JL_ASSERT( error != NULL, "Unable to create WinError object." );

	JL_CHK( jl::setException(cx, error) );
	JL_CHK( jl::setSlot(cx, error, SLOT_WIN_ERROR_CODE_HI, HIWORD(errorCode)) );
	JL_CHK( jl::setSlot(cx, error, SLOT_WIN_ERROR_CODE_LO, LOWORD(errorCode)) );
	JL_SAFE( jl::addScriptLocation(cx, &error) );
	return false;
	JL_BAD;
}


/*

Sure, you can JS_GetProperty(cx, global, "Error", &v) to get the constructor, then JS_GetProperty(cx, JSVAL_TO_OBJECT(v), "prototype", &v), then JL_GetClass(JSVAL_TO_OBJECT(v)).  Error and type checking elided, as usual.

*/
