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


BEGIN_CLASS( Sound ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

//DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.
//}

DEFINE_CONSTRUCTOR() { // Called when the object is constructed ( a = new Template() ) or activated ( a = Template() ). To distinguish the cases, use JS_IsConstructing() or use the JL_ASSERT_CONSTRUCTING() macro.

	JL_IGNORE(argc);

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_CONSTRUCTING();

	return true;
	JL_BAD;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
//	HAS_FINALIZE

//DEFINE_FUNCTION( call ) {
//	return true;
//}

//DEFINE_PROPERTY( prop ) {
//	return true;
//}

//DEFINE_FUNCTION( func ) {
//	return true;
//}

	BEGIN_FUNCTION_SPEC
//		FUNCTION(func)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

END_CLASS
