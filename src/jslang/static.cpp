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



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( ... )
**/
JLThreadFuncDecl MetaPollThread( void *data ) {

	MetaPollThreadInfo *ti = (MetaPollThreadInfo*)data;
	for (;;) {

		JLAcquireSemaphore(ti->start, -1);
		if ( ti->isEnd )
			break;
		JL_ASSERT( ti->mpSlot != NULL );
		ti->mpSlot->startPoll(ti->mpSlot);
		ti->mpSlot = NULL;
		JLReleaseSemaphore(ti->signalEventSem);
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
		JL_S_ASSERT_RESOURCE( mp );
		
		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		if ( ti->thread == 0 ) { // create the thread stuff, see jl_cmalloc in jslangModuleInit()

			ti->start = JLCreateSemaphore(0);
			ti->thread = JLThreadStart(MetaPollThread, ti);
			JLThreadPriority(ti->thread, JL_THREAD_PRIORITY_HIGHEST);
			ti->signalEventSem = mpv->metaPollSignalEventSem;
			ti->isEnd = false;
		}
		JL_ASSERT( ti->mpSlot == NULL );
		ti->mpSlot = mp;
		JLReleaseSemaphore(ti->start);
		metaPollList[i] = mp;
	}

	JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1); // wait for an event (timeout can also be managed here)
	JLReleaseSemaphore(mpv->metaPollSignalEventSem);

	bool canceled;
	for ( uintN i = 0; i < argc; ++i ) {

		volatile MetaPoll *mpSlot = mpv->metaPollThreadInfo[i].mpSlot; // avoids to mutex ti->mpSlot access.
		if ( mpSlot ) {

			canceled = mpSlot->cancelPoll(mpSlot);
			if ( !canceled ) { // if !canceled, kill the thread

				MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
				ti->mpSlot = NULL;
				JLReleaseSemaphore(ti->signalEventSem);
				JLThreadCancel(ti->thread);
				JLWaitThread(ti->thread); // (TBD) needed ?
				JLFreeSemaphore(&ti->start);
				JLFreeThread(&ti->thread);
				ti->thread = 0; // see thread creation place
			}
		}
	}

	for ( uintN i = 0; i < argc; ++i )
		JLAcquireSemaphore(mpv->metaPollSignalEventSem, -1);

	JL_ASSERT(argc <= JSVAL_INT_BITS); // bits
	unsigned int events;
	events = 0;
	bool hasEvent;
	JSBool ok;
	ok = true;
	for ( uintN i = 0; i < argc; ++i ) {

		MetaPoll *mp = metaPollList[i];
		if ( mp->endPoll(mp, &hasEvent, cx, obj) != JS_TRUE )
			ok = JS_FALSE;
		if ( hasEvent )
			events |= 1 << i;
	}

#ifdef DEBUG
	for ( uintN i = 0; i < argc; ++i ) {

		MetaPollThreadInfo *ti = &mpv->metaPollThreadInfo[i];
		JL_ASSERT( ti->mpSlot == NULL );
	}
	int res2 = JLAcquireSemaphore(mpv->metaPollSignalEventSem, 0);
	JL_ASSERT( res2 == JLTIMEOUT ); // invalid state
#endif // DEBUG

	*JL_RVAL = INT_TO_JSVAL(events);
	return ok;
	JL_BAD;
}




/**doc
$TOC_MEMBER $INAME
 $TYPE id $INAME( msTimeout )
**/
struct MetaPollTimeout {
	
	MetaPoll mp;
	unsigned int timeout;
	JLSemaphoreHandler cancel;
	bool canceled;
};

JL_STATIC_ASSERT( offsetof(MetaPollTimeout, mp) == 0 );

static void StartPoll( volatile MetaPoll *mp ) {

	MetaPollTimeout *mpt = (MetaPollTimeout*)mp;
	mpt->canceled = (JLAcquireSemaphore(mpt->cancel, mpt->timeout) == JLOK);
}

static bool CancelPoll( volatile MetaPoll *mp ) {

	MetaPollTimeout *mpt = (MetaPollTimeout*)mp;
	JLReleaseSemaphore(mpt->cancel);
	return true;
}

static JSBool EndPoll( volatile MetaPoll *mp, bool *hasEvent, JSContext *cx, JSObject *obj ) {

	MetaPollTimeout *mpt = (MetaPollTimeout*)mp;
	JLFreeSemaphore(&mpt->cancel);
	*hasEvent = !mpt->canceled;
	return JS_TRUE;
}

DEFINE_FUNCTION_FAST( MetaPollTimeout ) {

	JL_S_ASSERT_ARG( 1 );

	unsigned int timeout;
	JL_CHK( JsvalToUInt(cx, JL_FARG(1), &timeout) );

	MetaPollTimeout *mpt;
	JL_CHK( CreateHandle(cx, 'poll', sizeof(MetaPollTimeout), (void**)&mpt, NULL, JL_FRVAL) );
	mpt->mp.startPoll = StartPoll;
	mpt->mp.cancelPoll = CancelPoll;
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
