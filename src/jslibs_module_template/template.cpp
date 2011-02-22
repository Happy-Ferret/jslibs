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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Template ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

//	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // skip cleanups if possible.
//		return JS_TRUE;

}

DEFINE_CONSTRUCTOR() { // Called when the object is constructed ( a = new Template() ) or activated ( a = Template() ). To distinguish the cases, use JS_IsConstructing() or use the JL_S_ASSERT_CONSTRUCTING() macro.

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	return JS_TRUE;
	JL_BAD;
}

/*
DEFINE_PROPERTY( prop ) {

	return JS_TRUE;
}
*/

/*
DEFINE_FUNCTION( _serialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_COUNT(1);
	JL_S_ASSERT( jl::JsvalIsSerializer(cx, JL_ARG(1)), "Invalid serializer object." );
	jl::Serializer *ser;
	ser = jl::JsvalToSerializer(cx, JL_ARG(1));

	//ser->Write(cx, globalKey);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( _unserialize ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_ARG_COUNT(1);
	JL_S_ASSERT( jl::JsvalIsUnserializer(cx, JL_ARG(1)), "Invalid unserializer object." );
	jl::Unserializer *unser;
	unser = jl::JsvalToUnserializer(cx, JL_ARG(1));

	//uint32_t gKey;
	//unser->Read(cx, gKey);

	return JS_TRUE;
	JL_BAD;
}

*/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	//HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		//PROPERTY(prop)
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		//FUNCTION(Func)
		//FUNCTION_ARGC(_serialize, 1)
		//FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS
