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

#include "../common/jsvalserializer.h"

// the aim of *globalKey* is to ensure that a pointer in the process's virtual memory space can be serialized and unserializes safely.
static uint32_t globalKey = 0;

BEGIN_CLASS( Handle )

DEFINE_FINALIZE() { // see HandleClose()

	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;
	if ( pv->finalizeCallback ) // callback function is present
		pv->finalizeCallback((char*)pv + sizeof(HandlePrivate)); // (TBD) test it !
	jl_free(pv);
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_FUNCTION_OBJ;
	HandlePrivate *pv = (HandlePrivate*)JL_GetPrivate(cx, JL_OBJ);
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


DEFINE_HAS_INSTANCE() { // see issue#52

	*bp = !JSVAL_IS_PRIMITIVE(*v) && JL_InheritFrom(cx, JSVAL_TO_OBJECT(*v), JL_THIS_CLASS);
	return JS_TRUE;
}

DEFINE_INIT() {

	JL_SAFE( globalKey = JLSessionId() );
	return JS_TRUE;
}


DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	ser->Write(cx, globalKey);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	uint32_t gKey;
	unser->Read(cx, gKey);
	JL_S_ASSERT( gKey == globalKey, "Invalid session." );

	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_INIT
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(HANDLE_PUBLIC_SLOT_COUNT);
	HAS_FINALIZE
	HAS_HAS_INSTANCE

	BEGIN_FUNCTION_SPEC

		FUNCTION( toString )

		FUNCTION( _serialize )
		FUNCTION( _unserialize )

	END_FUNCTION_SPEC

END_CLASS
