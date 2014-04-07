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

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Audio )

struct Private : public jl::CppAllocators {
	CRITICAL_SECTION cs;
	HWAVEIN hwi;
	HANDLE thread;
	HANDLE ev;

	int frameLatency;

	int32_t audioBits;
	int32_t audioChannels;
	int32_t audioRate;

	jl::Queue1<void*> bufferList;
	bool end;


};

DWORD WINAPI 
WaveInThreadProc( LPVOID lpParam ) {

	Private *pv = (Private*)lpParam;

	size_t bufferLength = pv->frameLatency * pv->audioChannels * pv->audioBits/8;
	
	MMRESULT res;
	WAVEHDR waveHeader[2];

	for ( int i = 0; i < COUNTOF(waveHeader); ++i ) {

		waveHeader[i].lpData = (LPSTR)jl_malloc(bufferLength);
		waveHeader[i].dwBufferLength = bufferLength;
		waveHeader[i].dwFlags = 0;

//		waveHeader[i].dwBytesRecorded = 0;
//		waveHeader[i].dwLoops = 0;

		res = waveInPrepareHeader(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		res = waveInAddBuffer(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
	}

	PulseEvent(pv->ev); // first pulse

	for ( size_t index = 0 ; ; ++index ) {

		DWORD result = WaitForSingleObject(pv->ev, INFINITE);
		if ( pv->end )
			break;

		size_t bufferIndex = index % 2;

		res = waveInUnprepareHeader(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		EnterCriticalSection(&pv->cs);
		pv->bufferList.Push(waveHeader[bufferIndex].lpData);
		LeaveCriticalSection(&pv->cs);

		waveHeader[bufferIndex].lpData = (LPSTR)jl_malloc(bufferLength);
		waveHeader[bufferIndex].dwFlags = 0;

		res = waveInPrepareHeader(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );

		res = waveInAddBuffer(pv->hwi, &waveHeader[bufferIndex], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
	}

	res = waveInReset(pv->hwi);

	for ( int i = 0; i < COUNTOF(waveHeader); ++i ) {

		res = waveInUnprepareHeader(pv->hwi, &waveHeader[i], sizeof(WAVEHDR));
		ASSERT( res == MMSYSERR_NOERROR );
		jl_free(waveHeader[i].lpData);
	}

	return 0;
}


DEFINE_FINALIZE() {

	Private *pv = (Private*)js::GetObjectPrivate(obj);
	if ( !pv )
		return;

//	MMRESULT res;

	pv->end = true;
	PulseEvent(pv->ev);
	WaitForSingleObject(pv->thread, INFINITE);
	CloseHandle(pv->thread);
	waveInClose(pv->hwi);
	CloseHandle(pv->ev);
	DeleteCriticalSection(&pv->cs);

	delete pv;
}



/**doc
$TOC_MEMBER $INAME
 $INAME( deviceName | $UNDEF, msLatency )
**/
DEFINE_CONSTRUCTOR() {

	JL_DEFINE_ARGS;
	JL_DEFINE_CONSTRUCTOR_OBJ;

	Private *pv = new Private();
	JL_ASSERT_ALLOC(pv);
	JL_SetPrivate(JL_OBJ, pv);

	UINT uDeviceID;

	if ( JL_ARG_ISDEF(1) ) {

		JLData deviceName;

		JL_CHK( jl::getValue(cx, JL_ARG(1), &deviceName) );

		MMRESULT mmResult;
		UINT uNumDevs = waveInGetNumDevs();
		for ( uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++ ) {
		
			// Take a look at the driver capabilities.
			WAVEINCAPS wic;
			mmResult = waveInGetDevCaps(uDeviceID, &wic, sizeof(wic));
			
			//if ( mmResult != MMSYSERR_NOERROR )
			//	JL_CHK( WinThrowError(cx, mmResult) );
			ASSERT( mmResult == MMSYSERR_NOERROR );

			if ( deviceName.equals(wic.szPname) )
				break;
		}
	} else {
		
		uDeviceID = WAVE_MAPPER;
	}

	int32_t msLatency;
	if ( JL_ARG_ISDEF(2) ) {

		JL_CHK( jl::getValue(cx, JL_ARG(2), &msLatency) );
	} else {
		
		msLatency = 1000;
	}

	MMRESULT res;

	pv->audioBits = 16;
	pv->audioRate = 44100;
	pv->audioChannels = 2;

	pv->frameLatency = pv->audioRate * msLatency/1000;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = pv->audioRate;
	wfx.wBitsPerSample = pv->audioBits;
	wfx.nChannels = pv->audioChannels;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (wfx.wBitsPerSample * wfx.nChannels) / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	pv->end = false;

	InitializeCriticalSection(&pv->cs);
	pv->ev = CreateEvent(NULL, FALSE, FALSE, NULL);

	res = waveInOpen(&pv->hwi, uDeviceID, &wfx, (DWORD_PTR)pv->ev, NULL, WAVE_FORMAT_DIRECT | CALLBACK_EVENT);
	ASSERT( res == MMSYSERR_NOERROR );

	pv->thread = CreateThread(NULL, 0, WaveInThreadProc, pv, CREATE_SUSPENDED, NULL);
	SetThreadPriority(pv->thread, THREAD_PRIORITY_ABOVE_NORMAL);
	ResumeThread(pv->thread);

	WaitForSingleObject(pv->ev, INFINITE); // first pulse
		
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
	// waveInGetErrorText 

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


class AutoCriticalSection {

	CRITICAL_SECTION &_cs;
public:
	AutoCriticalSection(CRITICAL_SECTION cs)
	: _cs(cs) {

		EnterCriticalSection(&_cs);
	}
	~AutoCriticalSection() {

		LeaveCriticalSection(&_cs);
	}
};


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
	
		JL_RVAL.setUndefined();
		return true;
	}

	void *buffer;
	pv->bufferList.Pop(buffer);
	JL_CHK( JL_NewByteAudioObjectOwner(cx, (uint8_t*)buffer, pv->audioBits, pv->audioChannels, pv->frameLatency, pv->audioRate, JL_RVAL) );

	}

	//

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
	for (uDeviceID = 0; uDeviceID < uNumDevs; uDeviceID++) {
		
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

		JS::RootedObject audioObj(cx, &slot(0).toObject());
		Private *pv = (Private*)JL_GetPrivate(audioObj);

		EnterCriticalSection(&pv->cs);
		*hasEvent = !!pv->bufferList;
		LeaveCriticalSection(&pv->cs);

		if ( !*hasEvent )
			return true;

		if ( slot(1) != JL_ZInitValue() ) {
		
			JS::Value rval; // rval is unused then there is no need to root it
			JL_CHK( JS_CallFunctionValue(cx, audioObj, hslot(1), JS::HandleValueArray::empty(), JS::MutableHandleValue::fromMarkedLocation(&rval)) );
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

	AudioEvent *upe = new AudioEvent();
	JL_CHK( HandleCreate(cx, upe, JL_RVAL) );

	upe->slot(0) = JL_OBJVAL;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_CALLABLE(1);
		upe->slot(1) = JL_ARG(1);
	}

	upe->cancelEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
	if ( upe->cancelEvent == NULL )
		JL_CHK( JL_ThrowOSError(cx) );
	
	HANDLE currentProcess = GetCurrentProcess();
	BOOL st = DuplicateHandle(currentProcess, pv->ev, currentProcess, &upe->audioEvent, 0, FALSE, DUPLICATE_SAME_ACCESS);
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
		FUNCTION(start)
		FUNCTION(read)
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
