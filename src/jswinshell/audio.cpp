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
#include "error.h"
#include <mmsystem.h>


class AutoCriticalSection {
	CRITICAL_SECTION &_cs;
	bool _in;
public:
	AutoCriticalSection(CRITICAL_SECTION &cs)
	: _cs(cs), _in(true) {

		EnterCriticalSection(&_cs);
	}
	~AutoCriticalSection() {
		
		if ( _in )
			LeaveCriticalSection(&_cs);
	}
	void leave() {

		ASSERT( _in );
		LeaveCriticalSection(&_cs);
		_in = false;
	}
};




NEVER_INLINE bool FASTCALL
ThrowWinAudioError( JSContext *cx, UINT errorCode );



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( AudioIn )

struct Private : public jl::CppAllocators {
	CRITICAL_SECTION cs;
	HWAVEIN hwi;
	HANDLE thread;
	HANDLE audioEvent;
	HANDLE bufferReadyEvent;

	enum {
		RUNNING,
		ENDING,
	} state;

	int32_t frameLatency;
	int32_t audioBits;
	int32_t audioChannels;
	int32_t audioRate;

	jl::Queue1<jl::ArrayBuffer, jl::StaticAllocMedium> bufferList;
};


DWORD WINAPI 
WaveInThreadProc( LPVOID lpParam ) {

	Private *pv = static_cast<Private*>(lpParam);
	MMRESULT res;
	WAVEHDR waveHeader[2];
	jl::ArrayBuffer arrayBuf[2];

	size_t bufferLength = pv->frameLatency * pv->audioChannels * pv->audioBits/8;

	for ( int i = 0; i < 2; ++i ) {

		arrayBuf[i].alloc(bufferLength);
		waveHeader[i].lpData = (LPSTR)arrayBuf[i].data();
		waveHeader[i].dwBufferLength = bufferLength;
		waveHeader[i].dwFlags = 0;

		res = waveInPrepareHeader(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		res = waveInAddBuffer(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
	}

	PulseEvent(pv->audioEvent); // first pulse

	for ( size_t index = 0; ; ++index ) {

		DWORD status = WaitForSingleObject(pv->audioEvent, INFINITE);
		ASSERT( status != WAIT_FAILED );

		if ( pv->state == Private::ENDING )
			break;

		size_t bufferIndex = index % 2;

		res = waveInUnprepareHeader(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		EnterCriticalSection(&pv->cs);
		pv->bufferList.Push(arrayBuf[bufferIndex]);
		LeaveCriticalSection(&pv->cs);
		PulseEvent(pv->bufferReadyEvent);

		arrayBuf[bufferIndex].alloc(bufferLength);
		waveHeader[bufferIndex].lpData = (LPSTR)arrayBuf[bufferIndex].data();
		waveHeader[bufferIndex].dwFlags = 0;

		res = waveInPrepareHeader(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		res = waveInAddBuffer(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
	}

	res = waveInReset(pv->hwi);

	for ( int i = 0; i < 2; ++i ) {

		res = waveInUnprepareHeader(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
		arrayBuf[i].free();
	}

	return 0;
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( !pv )
		return;

	pv->state = Private::ENDING;
	PulseEvent(pv->audioEvent);
	WaitForSingleObject(pv->thread, INFINITE);
	if ( pv->thread )
		CloseHandle(pv->thread);
	waveInClose(pv->hwi);
	CloseHandle(pv->audioEvent);
	DeleteCriticalSection(&pv->cs);

	while ( pv->bufferList )
		pv->bufferList.Pop().free();

	delete pv;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( deviceName | $UNDEF, rate | $UNDEF, bits | $UNDEF, channels | $UNDEF, msLatency )
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;
	JL_ASSERT_ARGC_RANGE(0,5);

	Private *pv = new Private();
	JL_ASSERT_ALLOC(pv);
	JL_SetPrivate(JL_OBJ, pv);

	UINT uDeviceID;

	if ( JL_ARG_ISDEF(1) ) {

		JLData deviceName;
		JL_CHK( jl::getValue(cx, JL_ARG(1), &deviceName) );

		UINT uNumDevs = waveInGetNumDevs();
		for ( uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++ ) {
		
			// Take a look at the driver capabilities.
			WAVEINCAPS wic;
			MMRESULT result = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));
			ASSERT( result == MMSYSERR_NOERROR );
			if ( deviceName.equals(wic.szPname) )
				break;
		}
	} else {
		
		uDeviceID = WAVE_MAPPER;
	}

	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(2), &pv->audioRate) );
	} else {
		
		pv->audioRate = 44100;
	}

	if ( JL_ARG_ISDEF(3) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(3), &pv->audioBits) );
	} else {
		
		pv->audioBits = 16;
	}

	if ( JL_ARG_ISDEF(4) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(4), &pv->audioChannels) );
	} else {
		
		pv->audioChannels = 2;
	}

	int32_t msLatency;
	if ( JL_ARG_ISDEF(5) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(5), &msLatency) );
	} else {
		
		msLatency = 1000;
	}

	pv->frameLatency = pv->audioRate * msLatency/1000;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = pv->audioRate;
	wfx.wBitsPerSample = pv->audioBits;
	wfx.nChannels = pv->audioChannels;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	pv->state = Private::RUNNING;

	InitializeCriticalSection(&pv->cs);
	pv->audioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	pv->bufferReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	pv->thread = NULL;

	MMRESULT res;
	res = waveInOpen(&pv->hwi, uDeviceID, &wfx, (DWORD_PTR)pv->audioEvent, NULL, WAVE_FORMAT_DIRECT | CALLBACK_EVENT);
	if ( res != MMSYSERR_NOERROR )
		return ThrowWinAudioError(cx, res);

	pv->thread = CreateThread(NULL, 0, WaveInThreadProc, pv, CREATE_SUSPENDED, NULL);
	SetThreadPriority(pv->thread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(pv->thread);

	DWORD status = WaitForSingleObject(pv->audioEvent, INFINITE); // first pulse
	ASSERT( status != WAIT_FAILED );
		
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_ARGS;
	Private *pv = (Private*)js::GetObjectPrivate(JL_OBJ);
	{
	AutoCriticalSection acs(pv->cs);
	if ( !pv->bufferList ) {
		
		acs.leave();
		JL_RVAL.setUndefined();
		return true;
	}
	jl::ArrayBuffer buffer = pv->bufferList.Pop();
	acs.leave();
	JL_CHK( JL_NewByteAudioObjectOwner(cx, buffer, pv->audioBits, pv->audioChannels, pv->frameLatency, pv->audioRate, JL_RVAL) );
	}
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
DEFINE_FUNCTION( start ) {

	JL_DEFINE_ARGS;

	MMRESULT res;
	Private *pv = (Private*)js::GetObjectPrivate(JL_OBJ);

	res = waveInStart(pv->hwi);
	ASSERT( res == MMSYSERR_NOERROR );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INAME()
**/
DEFINE_FUNCTION( stop ) {

	JL_DEFINE_ARGS;

	MMRESULT res;
	Private *pv = (Private*)js::GetObjectPrivate(JL_OBJ);
	res = waveInStop(pv->hwi);
	ASSERT( res == MMSYSERR_NOERROR );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME
**/
DEFINE_PROPERTY_GETTER( inputDeviceList ) {

	JL_DEFINE_PROP_ARGS;

	JS::RootedObject list(cx, jl::newArray(cx));
	JL_RVAL.setObject(*list);

	UINT uDeviceID;
	MMRESULT mmResult;
	UINT uNumDevs = waveInGetNumDevs();
	for ( uDeviceID = 0; uDeviceID < uNumDevs; ++uDeviceID ) {
		
		// Take a look at the driver capabilities.
		WAVEINCAPS wic;
		mmResult = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));
		ASSERT( mmResult == MMSYSERR_NOERROR );
		jl::pushElement(cx, list, wic.szPname);
	}

	return true;
}



struct AudioEvent : public ProcessEvent2 {
	HANDLE cancelEvent;
	HANDLE audioEvent;
	Private *_pv;

	AudioEvent(Private *pv)
	: _pv(pv) {
	}

	~AudioEvent() {

		CloseHandle(cancelEvent);
		CloseHandle(audioEvent);
	}

	bool prepareWait(JSContext *cx, JS::HandleObject obj) {
	
		return true;
	}

	void startWait() {

		HANDLE events[] = { audioEvent, cancelEvent };
		DWORD status = WaitForMultipleObjects(COUNTOF(events), events, FALSE, INFINITE);
		ASSERT( status != WAIT_FAILED );
	}

	bool cancelWait() {

		SetEvent(cancelEvent);
		return true;
	}

	bool endWait(bool *hasEvent, JSContext *cx, JS::HandleObject) {

		DWORD status = WaitForSingleObject(audioEvent, 0);
		ASSERT( status != WAIT_FAILED );

		EnterCriticalSection(&_pv->cs);
		*hasEvent = !!_pv->bufferList;
		LeaveCriticalSection(&_pv->cs);

		if ( *hasEvent && slot(1) != JL_ZInitValue() ) {
		
			JS::Value rval; // rval is unused then there is no need to root it
			JL_CHK( jl::call(cx, hslot(0), hslot(1), JS::MutableHandleValue::fromMarkedLocation(&rval)) );
		}
		return true;
		JL_BAD;
	}
};


DEFINE_FUNCTION( events ) {
	
	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	AudioEvent *upe = new AudioEvent(pv);
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	upe->slot(0) = JL_OBJVAL; // Audio object
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);
		upe->slot(1) = JL_ARG(1);
	}

	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
	if ( upe->cancelEvent == NULL )
		JL_CHK( JL_ThrowOSError(cx) );
	
	BOOL st = DuplicateHandle(GetCurrentProcess(), pv->bufferReadyEvent, GetCurrentProcess(), &upe->audioEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
	if ( !st )
		JL_CHK( JL_ThrowOSError(cx) );

	return true;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION(read)
		FUNCTION(start)
		FUNCTION(stop)
		FUNCTION(events)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_GETTER( inputDeviceList )
	END_STATIC_PROPERTY_SPEC

END_CLASS

/**doc
=== Examples ===
{{{
}}}
**/



BEGIN_CLASS( WinAudioError )

DEFINE_PROPERTY_GETTER( code ) {

	JL_DEFINE_PROP_ARGS;

	UINT errorCode;
	JL_CHK( jl::getSlot(cx, JL_OBJ, 0, &errorCode) );
	JL_CHK( jl::setValue(cx, vp, errorCode) );
	return true;
	JL_BAD;
}

const char *ErrorToConstName( UINT errorCode ) {

	switch ( errorCode ) {
		case WAVERR_BADFORMAT     : return "WAVERR_BADFORMAT";
		case WAVERR_STILLPLAYING  : return "WAVERR_STILLPLAYING";
		case WAVERR_UNPREPARED    : return "WAVERR_UNPREPARED";
		case WAVERR_SYNC          : return "WAVERR_SYNC";
		case MMSYSERR_ERROR       : return "MMSYSERR_ERROR";
		case MMSYSERR_BADDEVICEID : return "MMSYSERR_BADDEVICEID";
		case MMSYSERR_NOTENABLED  : return "MMSYSERR_NOTENABLED";
		case MMSYSERR_ALLOCATED   : return "MMSYSERR_ALLOCATED";
		case MMSYSERR_INVALHANDLE : return "MMSYSERR_INVALHANDLE";
		case MMSYSERR_NODRIVER    : return "MMSYSERR_NODRIVER";
		case MMSYSERR_NOMEM       : return "MMSYSERR_NOMEM";
		case MMSYSERR_NOTSUPPORTED: return "MMSYSERR_NOTSUPPORTED";
		case MMSYSERR_BADERRNUM   : return "MMSYSERR_BADERRNUM";
		case MMSYSERR_INVALFLAG   : return "MMSYSERR_INVALFLAG";
		case MMSYSERR_INVALPARAM  : return "MMSYSERR_INVALPARAM";
		case MMSYSERR_HANDLEBUSY  : return "MMSYSERR_HANDLEBUSY";
		case MMSYSERR_INVALIDALIAS: return "MMSYSERR_INVALIDALIAS";
		case MMSYSERR_BADDB       : return "MMSYSERR_BADDB";
		case MMSYSERR_KEYNOTFOUND : return "MMSYSERR_KEYNOTFOUND";
		case MMSYSERR_READERROR   : return "MMSYSERR_READERROR";
		case MMSYSERR_WRITEERROR  : return "MMSYSERR_WRITEERROR";
		case MMSYSERR_DELETEERROR : return "MMSYSERR_DELETEERROR";
		case MMSYSERR_VALNOTFOUND : return "MMSYSERR_VALNOTFOUND";
		case MMSYSERR_NODRIVERCB  : return "MMSYSERR_NODRIVERCB";
		case MMSYSERR_MOREDATA    : return "MMSYSERR_MOREDATA";
		default: return NULL;
	}
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_DEFINE_PROP_ARGS;

	UINT errorCode;
	JL_CHK( jl::getSlot(cx, JL_OBJ, 0, &errorCode) );
	JL_CHK( jl::setValue(cx, vp, ErrorToConstName(errorCode)) );
	return true;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( text ) {

	JL_DEFINE_PROP_ARGS;

	UINT errorCode;
	JL_CHK( jl::getSlot(cx, JL_OBJ, 0, &errorCode) );
	CHAR errorText[MAXERRORLENGTH];
	waveInGetErrorText(errorCode, errorText, COUNTOF(errorText));
	JL_CHK( jl::setValue(cx, vp, errorText) );

	return true;
	JL_BAD;
}


DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;
	JL_DEFINE_FUNCTION_OBJ;

	UINT errorCode;
	JL_CHK( jl::getSlot(cx, JL_OBJ, 0, &errorCode) );
	CHAR errorText[MAXERRORLENGTH];
	waveInGetErrorText(errorCode, errorText, COUNTOF(errorText));
	JL_CHK( jl::setValue(cx, JL_RVAL, errorText) );

	return true;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3524 $"))
	HAS_RESERVED_SLOTS(2)

	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
		PROPERTY_GETTER( text )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE bool FASTCALL
ThrowWinAudioError( JSContext *cx, UINT errorCode ) {

	JS::RootedObject error(cx, jl::newObjectWithGivenProto( cx, JL_CLASS(WinAudioError), JL_CLASS_PROTOTYPE(cx, WinAudioError) ));

	JL_CHK( jl::setException(cx, error) );
	JL_CHK( jl::setSlot(cx, error, 0, errorCode) );
	JL_SAFE( jl::setScriptLocation(cx, &error) );
	return false;
	JL_BAD;
}

