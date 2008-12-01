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

/**doc fileIndex:topmost **/

BEGIN_STATIC

/**doc
=== Static functions ==
**/

/**doc
 * $STR $INAME( value )
  This function converts any value of stream into a string.
**/
DEFINE_FUNCTION( Stringify ) {

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_ARG_MAX(1);

	if ( JSVAL_IS_OBJECT( J_ARG(1) ) && !JSVAL_IS_NULL( J_ARG(1) ) ) {

		NIStreamRead read = StreamReadInterface(cx, JSVAL_TO_OBJECT( J_ARG(1) ));
		if ( read ) {
			
			size_t total = 0;
			void *stack;
			jl::StackInit(&stack);
			
			size_t defaultChunkSize = 64; // 4096 - sizeof(size_t); // try to reach a multiple of the system page size
//			size_t waste = 0;
//			size_t round = 0;


			size_t length;
			do {
				
				length = defaultChunkSize;

				void *chunk = malloc(length + sizeof(size_t));
				J_S_ASSERT_ALLOC( chunk );
				jl::StackPush(&stack, chunk);

				size_t *chunkDataLength = (size_t*)chunk;
				char *chunkData = (char*)chunk + sizeof(size_t);

				J_CHK( read(cx, JSVAL_TO_OBJECT( J_ARG(1) ), chunkData, &length) );

				*chunkDataLength = length;
				total += length;

//				round++;
//				waste += defaultChunkSize - length;

				if ( length < defaultChunkSize )
					defaultChunkSize = defaultChunkSize / 2 + 8;
				else
					defaultChunkSize = defaultChunkSize * 2 / 3 + 8;

			} while ( length != 0 );


			char *newBuffer = (char*)JS_malloc(cx, total +1);
			J_S_ASSERT_ALLOC( newBuffer );
			newBuffer[total] = '\0';

			newBuffer += total;

			while ( !jl::StackIsEnd(&stack) ) {
				
				void *chunk = jl::StackPop(&stack);
				size_t *chunkDataLength = (size_t*)chunk;
				char *chunkData = (char*)chunk + sizeof(size_t);
				newBuffer -= *chunkDataLength;
				memcpy(newBuffer, chunkData, *chunkDataLength);
				free(chunk);
			}
		
			JSString *jsstr = JS_NewString(cx, newBuffer, total);
			J_S_ASSERT_ALLOC( jsstr );
			*J_RVAL = STRING_TO_JSVAL( jsstr );
			return JS_TRUE;
		}
	}

	const char *buffer;
	size_t length;
	 // this include NIBufferGet compatible objects
	J_CHK( JsvalToStringAndLength(cx, &J_ARG(1), &buffer, &length) ); // warning: GC on the returned buffer !

	char *newBuffer;
	newBuffer = (char*)JS_malloc(cx, length +1);
	J_S_ASSERT_ALLOC( newBuffer );
	newBuffer[length] = '\0';
	memcpy(newBuffer, buffer, length);

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuffer, length);
	*J_RVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}

#ifdef DEBUG

DEFINE_FUNCTION( Test ) {

	return JS_TRUE;
}

#endif // DEBUG


CONFIGURE_STATIC

//	REVISION(SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Stringify )

#ifdef DEBUG
		FUNCTION( Test )
#endif // DEBUG

	END_STATIC_FUNCTION_SPEC

END_STATIC
