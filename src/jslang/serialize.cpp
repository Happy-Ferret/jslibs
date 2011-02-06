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

#include "../common/jsvalserializer.h"

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3321 $
**/
BEGIN_CLASS( Serializer )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	jl::Serializer *ser;
	ser = static_cast<jl::Serializer*>(JL_GetPrivate(cx, obj));
	if ( !ser )
		return;
	delete ser;
}

DEFINE_CONSTRUCTOR() {

	JL_DEFINE_CONSTRUCTOR_OBJ;
	jl::Serializer *ser;
	ser = new jl::Serializer(OBJECT_TO_JSVAL(obj));
	JL_S_ASSERT_ALLOC(ser);
	JL_SetPrivate(cx, obj, ser);
	ser->Write(cx, JL_THIS_REVISION);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
**/
DEFINE_FUNCTION( Write ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG(1);
	*JL_RVAL = JSVAL_VOID;
	jl::Serializer *ser;
	ser = static_cast<jl::Serializer*>(JL_GetPrivate(cx, obj));
	JL_S_ASSERT_RESOURCE(ser);
	JL_CHKM( ser->Write(cx, JL_ARG(1)), "Serializer write error.");
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION( Done ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG(0);
	jl::Serializer* ser;
	ser = (jl::Serializer*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE(ser);
	void *data;
	size_t length;
	JL_CHKM( ser->GetBufferOwnership(&data, &length), "Serializer buffer error.");
	delete ser;
	JL_SetPrivate(cx, obj, NULL);
	JL_updateMallocCounter(cx, length);
	return JL_NewBlob(cx, data, length, vp);
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3321 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(Write, 1)
		FUNCTION_ARGC(Done, 0)
	END_FUNCTION_SPEC

END_CLASS





/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3321 $
**/
BEGIN_CLASS( Unserializer )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	jl::Unserializer *unser;
	unser = static_cast<jl::Unserializer*>(JL_GetPrivate(cx, obj));
	if ( !unser )
		return;
	delete unser;
}

DEFINE_CONSTRUCTOR() {

	JLStr str;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_S_ASSERT_ARG(1);
	JL_S_ASSERT_STRING(JL_ARG(1));
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	jl::Unserializer *unser;
	unser = new jl::Unserializer(OBJECT_TO_JSVAL(obj), str.GetStrZOwnership(), str.Length());
	JL_S_ASSERT_ALLOC(unser);
	JL_SetPrivate(cx, obj, unser);
	JLRevisionType rev;
	JL_CHK( unser->Read(cx, rev) );
	JL_S_ASSERT( rev == JL_THIS_REVISION, "Invalid serialized data version." );
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( value )
**/
DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG(0);

	jl::Unserializer *unser;
	unser = static_cast<jl::Unserializer*>(JL_GetPrivate(cx, obj));
	JL_S_ASSERT_RESOURCE(unser);
	JL_CHKM( unser->Read(cx, *JL_RVAL), "Unserializer read error.");
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3321 $"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(Read, 0)
	END_FUNCTION_SPEC

END_CLASS

