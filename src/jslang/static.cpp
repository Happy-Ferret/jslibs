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

#include "../jslang/handlePub.h"

#include <cstring>

#include "jslang.h"

#include "static.h"

#include "stack.h"
#include "buffer.h"
using namespace jl;

DECLARE_CLASS(Handle)


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

	JL_S_ASSERT_ARG(1);

	if ( !JSVAL_IS_PRIMITIVE(JL_ARG(1)) ) {

		JSObject *sobj;
		sobj = JSVAL_TO_OBJECT( JL_ARG(1) );
		NIStreamRead read = StreamReadInterface(cx, sobj);
		if ( read ) {

			Buffer buf;
			BufferInitialize(&buf, bufferTypeAuto, bufferGrowTypeAuto, NULL, NULL, NULL, NULL);

			size_t length;
			do {
				length = 4096;
				JL_CHKB( read(cx, sobj, BufferNewChunk(&buf, length), &length), bad_freeBuffer );
				BufferConfirm(&buf, length);
			} while ( length != 0 );

			size_t total;
			total = BufferGetLength(&buf);
			char *newBuffer;
			newBuffer = (char*)JS_malloc(cx, total +1);
			JL_CHK( newBuffer );
			newBuffer[total] = '\0';
			BufferCopyData(&buf, newBuffer, total);
			
			JSString *jsstr;
			jsstr = JS_NewString(cx, newBuffer, total);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL( jsstr );

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
	JL_CHK( JsvalToStringAndLength(cx, &JL_ARG(1), &buffer, &length) ); // warning: GC on the returned buffer !

	char *newBuffer;
	newBuffer = (char*)JS_malloc(cx, length +1);
	JL_CHK( newBuffer );
	newBuffer[length] = '\0';
	memcpy(newBuffer, buffer, length);

	JSString *jsstr;
	jsstr = JS_NewString(cx, newBuffer, length);
	*JL_RVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}




JLThreadFuncDecl MetaPollThread( void *data ) {

	MetaPollThreadInfo *metaPollThreadInfo = (MetaPollThreadInfo*)data;
	
	for (;;) {

		JLReleaseMutex(metaPollThreadInfo->out);
		JLAcquireMutex(metaPollThreadInfo->in);
		
		if ( metaPollThreadInfo->isEnd )
			JLThreadExit();
		
		metaPollThreadInfo->mpSlot->startPoll(metaPollThreadInfo->mpSlot);
		JLReleaseSemaphore(metaPollThreadInfo->signalEventSem);
		
		JLReleaseMutex(metaPollThreadInfo->in);
		JLAcquireMutex(metaPollThreadInfo->out);

		#ifdef DEBUG
		metaPollThreadInfo->mpSlot = NULL;
		#endif // DEBUG
	}
	return 0;
}

DEFINE_FUNCTION( MetaPoll ) {

	ModulePrivate *mpv = (ModulePrivate*)GetModulePrivate(cx, 'lang');

	JL_S_ASSERT_ARG_MAX( COUNTOF(mpv->metaPollThreadInfo) );
	MetaPoll *metaPollList[COUNTOF(mpv->metaPollThreadInfo)]; // cache

	for ( uintN i = 0; i < argc; ++i ) {

		JL_S_ASSERT( IsHandleType(cx, JL_ARGV[i], 'poll'), "Invalid MetaPoolable handle." );
		MetaPoll *mp = (MetaPoll*)GetHandlePrivate(cx, JL_ARGV[i]);
		
		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		if ( ti->thread == 0 ) {

			ti->out = JLCreateMutex();
			ti->in = JLCreateMutex();
			
			JLAcquireMutex(ti->in);

			ti->isEnd = false;
			ti->thread = JLThreadStart(MetaPollThread, ti);
			ti->signalEventSem = mpv->metaPollSignalEventSem;
		}

		ti->mpSlot = mp;

		JLAcquireMutex(ti->out);
		JLReleaseMutex(ti->in);

		metaPollList[i] = mp;
	}

	JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1); // wait for an event
	JLReleaseSemaphore(mpv->metaPollSignalEventSem);

	for ( uintN i = 0; i < argc; ++i ) {

		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		MetaPoll *mp = metaPollList[i];

		JL_CHK( mp->endPoll(mp, cx) ); // (TBD) on failure, also release the others.

		JLAcquireMutex(ti->in);
		JLReleaseMutex(ti->out);

		JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1);
	}

	return JS_TRUE;
	JL_BAD;
}



#ifdef DEBUG
DEFINE_FUNCTION( jslang_test ) {
	return JS_TRUE;
}
#endif // DEBUG


CONFIGURE_STATIC

//	REVISION(JL_SvnRevToInt("$Revision$")) // avoid to set a revision to the global context
	BEGIN_STATIC_FUNCTION_SPEC

		#ifdef DEBUG
		FUNCTION( jslang_test )
		#endif // DEBUG

		FUNCTION_ARGC( Stringify, 1 )
		FUNCTION( MetaPoll )
	END_STATIC_FUNCTION_SPEC

END_STATIC
