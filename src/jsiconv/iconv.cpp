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

#include <iconv.h>

/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Iconv ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	iconv_t cd = JS_GetPrivate(cx, obj);
	if ( cd != NULL ) {
		
		iconv_close(cd);
	}
}


DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(2);

	const char *tocode;
	const char *fromcode;

	J_CHK( JsvalToString(cx, &J_ARG(1), &tocode) );
	J_CHK( JsvalToString(cx, &J_ARG(2), &fromcode) );

	iconv_t cd = iconv_open(tocode, fromcode);

	J_S_ASSERT_2( cd != NULL, "Unable to open %s to %s conversion.", fromcode, tocode );

	J_CHK( JS_SetPrivate(cx, obj, cd) );

	return JS_TRUE;
}


DEFINE_CALL() {

	J_S_ASSERT_ARG_MIN(1);
		
	JSObject *thisObj = JSVAL_TO_OBJECT(argv[-2]); // get 'this' object of the current object ...
	J_S_ASSERT_CLASS(thisObj, classIconv);

	iconv_t cd = JS_GetPrivate(cx, thisObj);
	J_S_ASSERT_RESOURCE( cd );

	const char *inBuf;
	size_t inLen;
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &inBuf, &inLen) );

	char *outBuf;
	size_t outLen;

	outBuf = (char*)JS_malloc(cx, inLen*4);
	J_S_ASSERT_ALLOC( outBuf );

	size_t status = iconv(cd, &inBuf, &inLen, &outBuf, &outLen);

	return JS_TRUE;
}

CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
	HAS_CALL

	HAS_CONSTRUCTOR
	HAS_FINALIZE

END_CLASS
