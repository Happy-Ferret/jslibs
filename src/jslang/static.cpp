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

#include "../jslang/idPub.h"

#include <cstring>

#include "static.h"

#include "../common/stack.h"
#include "../common/buffer.h"
using namespace jl;

/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC

/**doc
=== Static functions ==
**/

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( value )
  This function converts any value of stream into a string.
**/
DEFINE_FUNCTION( Stringify ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_ARG_MAX(1);

	if ( !JSVAL_IS_PRIMITIVE(J_ARG(1)) ) {

		JSObject *sobj;
		sobj = JSVAL_TO_OBJECT( J_ARG(1) );
		NIStreamRead read = StreamReadInterface(cx, sobj);
		if ( read ) {

			Buffer buf;
			BufferInitialize(&buf, bufferTypeAuto, bufferGrowTypeAuto);

			size_t length;
			do {
				length = 4096;
				J_CHKB( read(cx, sobj, BufferNewChunk(&buf, length), &length), bad_freeBuffer );
				BufferConfirm(&buf, length);
			} while ( length != 0 );

			size_t total;
			total = BufferGetLength(&buf);
			char *newBuffer;
			newBuffer = (char*)JS_malloc(cx, total +1);
			J_CHK( newBuffer );
			newBuffer[total] = '\0';
			BufferCopyData(&buf, newBuffer, total);
			
			JSString *jsstr;
			jsstr = JS_NewString(cx, newBuffer, total);
			J_CHK( jsstr );
			*J_RVAL = STRING_TO_JSVAL( jsstr );

			BufferFinalize(&buf);
			return JS_TRUE;
		bad_freeBuffer:
			BufferFinalize(&buf);
			return JS_FALSE;
		}
	}

	const char *buffer;
	size_t length;
	 // this include NIBufferGet compatible objects
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &buffer, &length) ); // warning: GC on the returned buffer !

	char *newBuffer;
	newBuffer = (char*)JS_malloc(cx, length +1);
	J_CHK( newBuffer );
	newBuffer[length] = '\0';
	memcpy(newBuffer, buffer, length);

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuffer, length);
	*J_RVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

//	REVISION(SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_ARGC( Stringify, 1 )
	END_STATIC_FUNCTION_SPEC

END_STATIC
