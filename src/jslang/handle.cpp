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
#include "handlePub.h"

// the aim of *globalKey* is to ensure that a pointer in the process's virtual memory space can be serialized and unserializes safely.
static uint32_t globalKey = 0;

BEGIN_CLASS( Handle )

DEFINE_FINALIZE() { // see HandleClose()

	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, obj);
	if ( pv ) {

		if ( pv->finalizeCallback ) // callback function is present
			pv->finalizeCallback((uint8_t*)pv + sizeof(HandlePrivate)); // (TBD) test it !
		jl_free(pv);
	}
}


DEFINE_FUNCTION( toString ) {

	JL_IGNORE( argc );
	JL_DEFINE_FUNCTION_OBJ;
//	JL_ASSERT_THIS_INSTANCE();

	HandlePrivate *pv = JL_HasPrivateSlot(cx, JL_OBJ) ? (HandlePrivate*)JL_GetPrivate(cx, JL_OBJ) : NULL;
	JSString *handleStr;
	char str[] = "[Handle ????]";
	if ( pv != NULL ) { // this manage Print(Handle) issue

		char *ht = (char*)&pv->handleType;
		if ( JLHostEndian == JLLittleEndian ) {

			str[ 8] = ht[3];
			str[ 9] = ht[2];
			str[10] = ht[1];
			str[11] = ht[0];
		} else {

			*((HANDLE_TYPE*)str + 8) = pv->handleType;
		}

		if ( str[ 8] == '\0' )  str[ 8] = ' ';
		if ( str[ 9] == '\0' )  str[ 9] = ' ';
		if ( str[10] == '\0' )  str[10] = ' ';
		if ( str[11] == '\0' )  str[11] = ' ';
	}

	handleStr = JS_NewStringCopyN(cx, str, sizeof(str)-1);
	JL_CHK( handleStr );
	*JL_RVAL = STRING_TO_JSVAL(handleStr);

	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_FUNCTION( valueOf ) {

	*JL_RVAL = INT_TO_JSVAL(213);
	return JS_TRUE;
	JL_BAD;
}
*/


/*
DEFINE_HAS_INSTANCE() { // see issue#52

	JL_IGNORE(obj);
	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}
*/

DEFINE_INIT() {

	JL_IGNORE(cx);
	JL_IGNORE(sc);
	JL_IGNORE(proto);
	JL_IGNORE(obj);

	JL_SAFE( globalKey = JLSessionId() );
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsSerializer(cx, JL_ARG(1)), 1, "Serializer" );

	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	ser->Write(cx, globalKey);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC(1);
	JL_ASSERT_ARG_TYPE( jl::JsvalIsUnserializer(cx, JL_ARG(1)), 1, "Unserializer" );

	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	uint32_t gKey;
	unser->Read(cx, gKey);
	JL_CHKM( gKey == globalKey, E_THISOPERATION, E_INVALID );

	return JS_TRUE;
	JL_BAD;
}
*/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_INIT
	FROZEN_PROTOTYPE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(HANDLE_PUBLIC_SLOT_COUNT);
	HAS_FINALIZE
	
	//HAS_HAS_INSTANCE
	IS_INCONSTRUCTIBLE

	BEGIN_FUNCTION_SPEC

		FUNCTION( toString )
/*
		FUNCTION( _serialize )
		FUNCTION( _unserialize )
*/
	END_FUNCTION_SPEC

END_CLASS
