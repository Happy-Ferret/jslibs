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
			break;
		JL_ASSERT( metaPollThreadInfo->mpSlot != NULL );
		metaPollThreadInfo->mpSlot->startPoll(metaPollThreadInfo->mpSlot);
		JLReleaseSemaphore(metaPollThreadInfo->signalEventSem);
		JLReleaseMutex(metaPollThreadInfo->in);
		JLAcquireMutex(metaPollThreadInfo->out);

		#ifdef DEBUG
		metaPollThreadInfo->mpSlot = NULL;
		#endif // DEBUG
	}
	JLThreadExit();
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
		if ( ti->thread == 0 ) { // create the thread stuff, see jl_cmalloc in jslangModuleInit()

			ti->out = JLCreateMutex();
			ti->in = JLCreateMutex();
			JLAcquireMutex(ti->in);
			ti->thread = JLThreadStart(MetaPollThread, ti);
			JLThreadPriority(ti->thread, JL_THREAD_PRIORITY_HIGH);

			ti->signalEventSem = mpv->metaPollSignalEventSem;
			ti->isEnd = false;
		}

		ti->mpSlot = mp;

		JLAcquireMutex(ti->out);
		JLReleaseMutex(ti->in);

		metaPollList[i] = mp;
	}

	JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1); // wait for an event (timeout can also be managed here)
	JLReleaseSemaphore(mpv->metaPollSignalEventSem);

	JSBool ok;
	ok = true;
	for ( uintN i = 0; i < argc; ++i ) {

		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		MetaPoll *mp = metaPollList[i];

		if ( !mp->endPoll(mp, cx) )
			ok = JS_FALSE;

		JLAcquireMutex(ti->in);
		JLReleaseMutex(ti->out);
		JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1);
	}

	#ifdef DEBUG
	int res = JLAcquireSemaphore(mpv->metaPollSignalEventSem, 0);
	JL_S_ASSERT( res == JLTIMEOUT, "Invalid state" );
	#endif // DEBUG

	*JL_RVAL = JSVAL_VOID;
	return ok;
	JL_BAD;
}




struct MetaPollTimeout {
	
	MetaPoll mp;
	unsigned int timeout;
	JLSemaphoreHandler cancel;
};

JL_STATIC_ASSERT( offsetof(MetaPollTimeout, mp) == 0 );


static void StartPoll( MetaPoll *mp ) {

	MetaPollTimeout *mpt = (MetaPollTimeout*)mp;
	JLAcquireSemaphore(mpt->cancel, mpt->timeout);
}

static JSBool EndPoll( MetaPoll *mp, JSContext *cx ) {

	MetaPollTimeout *mpt = (MetaPollTimeout*)mp;
	JLReleaseSemaphore(mpt->cancel);
	JLFreeSemaphore(&mpt->cancel);
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( MetaPollTimeout ) {

	JL_S_ASSERT_ARG( 1 );

	unsigned int timeout;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &timeout) );

	MetaPollTimeout *mpt;
	JL_CHK( CreateHandle(cx, 'poll', sizeof(MetaPollTimeout), (void**)&mpt, NULL, JL_FRVAL) );
	mpt->mp.startPoll = StartPoll;
	mpt->mp.endPoll = EndPoll;
	mpt->timeout = timeout;
	mpt->cancel = JLCreateSemaphore(0);

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
		FUNCTION_FAST( MetaPollTimeout )
	END_STATIC_FUNCTION_SPEC

END_STATIC
