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
#include "pack.h"

#include "../common/jsNativeInterface.h"
#include "../common/queue.h"

BEGIN_CLASS( Pack )

DEFINE_FINALIZE() {
}

DEFINE_CONSTRUCTOR() {

	return JS_TRUE;
}

DEFINE_FUNCTION( ReadInt ) {

	int size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : sizeof(int);

	return JS_TRUE;
}

DEFINE_FUNCTION( ReadUInt ) {

	int size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : sizeof(unsigned int);
	return JS_TRUE;
}

DEFINE_FUNCTION( ReadReal ) {

	int size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : 0;
	return JS_TRUE;
}

DEFINE_FUNCTION( ReadString ) {

	int size = argc >= 1 ? JSVAL_TO_INT(argv[0]) : -1;
	return JS_TRUE;
}


DEFINE_PROPERTY( length ) {

	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(ReadInt)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(length)
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(0)

END_CLASS
