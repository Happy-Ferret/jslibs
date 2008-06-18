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

#include <cstring>

#include "static.h"

#include "../common/stack.h"

BEGIN_STATIC


DEFINE_FUNCTION( Stringify ) { // todoc

	J_S_ASSERT_ARG_MIN(1);
	J_S_ASSERT_ARG_MAX(1);

	if ( JSVAL_IS_OBJECT( J_ARG(1) ) && !JSVAL_IS_NULL( J_ARG(1) ) ) {

		NIStreamRead read = StreamReadInterface(cx, JSVAL_TO_OBJECT( J_ARG(1) ));
		if ( read ) {
			
			size_t total = 0;
			void *stack;
			StackInit(&stack);
			
			size_t length;
			do {

				length = 8192 - sizeof(size_t); // try to reach a multiple of the system page size

				void *chunk = malloc(length + sizeof(size_t));
				J_S_ASSERT_ALLOC( chunk );
				StackPush(&stack, chunk);

				size_t *chunkDataLength = (size_t*)chunk;
				char *chunkData = (char*)chunk + sizeof(size_t);

				J_CHK( read(cx, JSVAL_TO_OBJECT( J_ARG(1) ), chunkData, &length) );

				*chunkDataLength = length;
				total += length;

			} while ( length != 0 );


			char *newBuffer = (char*)JS_malloc(cx, total +1);
			J_S_ASSERT_ALLOC( newBuffer );
			newBuffer[total] = '\0';

			newBuffer += total;

			while ( !StackIsEnd(&stack) ) {
				
				void *chunk = StackPop(&stack);
				size_t *chunkDataLength = (size_t*)chunk;
				char *chunkData = (char*)chunk + sizeof(size_t);
				newBuffer -= *chunkDataLength;
				memcpy(newBuffer, chunkData, *chunkDataLength);
				free(chunk);
			}
		
			JSString *jsstr = JS_NewString(cx, newBuffer, length);
			J_S_ASSERT_ALLOC( jsstr );
			*J_RVAL = STRING_TO_JSVAL( jsstr );
			return JS_TRUE;
		}
	}

	const char *buffer;
	size_t length;
	J_CHK( JsvalToStringAndLength(cx, J_ARG(1), &buffer, &length) );

	char *newBuffer = (char*)JS_malloc(cx, length +1);
	J_S_ASSERT_ALLOC( newBuffer );
	newBuffer[length] = '\0';
	memcpy(newBuffer, buffer, length);

	JSString *jsstr = JS_NewString(cx, newBuffer, length);
	*J_RVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
}



CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Stringify )
	END_STATIC_FUNCTION_SPEC

END_STATIC
