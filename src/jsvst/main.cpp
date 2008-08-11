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

#include "../host/host.h"
#include "../common/jsClass.h"

#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "audiomaster.h"

DECLARE_CLASS( AudioMaster );
DECLARE_CLASS( VSTPlugin );

class JsVst : public AudioEffectX {
private:

	JSContext *cx;
	JSObject *vstPlugin;
	jsval _arg, _rval;

	inline jsval GetFunction( const char *functionName ) {
	
		jsval fval;
		JSBool res = JS_GetProperty(cx, vstPlugin, functionName, &fval);
		return res == JS_TRUE && JsvalIsFunction(cx, fval) ? fval : JSVAL_VOID;
	}

	inline JSBool MaybeReportException() {

		if ( JS_IsExceptionPending(cx) ) { // let the script manage its non-fatal errors.

			jsval fval = GetFunction("catch");
			if ( fval != JSVAL_VOID ) {

				jsval rval, ex;
				J_CHK( JS_GetPendingException(cx, &ex) );
				J_CHK( JS_CallFunctionValue(cx, vstPlugin, fval, 1, &ex, &rval) );
				
				// if the catch() function returns something, we report it as an error
				if ( JSVAL_IS_OBJECT(rval) ) {

					JS_SetPendingException(cx, rval);
					JS_ReportPendingException(cx);
					JS_ClearPendingException(cx);
				}
			}
		}
		return JS_TRUE;
	}

public:

	~JsVst() {

		JS_RemoveRoot(cx, &_arg);
		JS_RemoveRoot(cx, &_rval);
		DestroyHost(cx);
	}

	JsVst( audioMasterCallback audioMaster ) 
	: AudioEffectX (audioMaster, 0, 0) {

		cx = CreateHost(-1, -1);
		InitHost(cx, true, NULL, NULL);
//		JSObject *audioMasterObject = CreateAudioMasterObject(cx, audioMaster);

		JS_AddRoot(cx, &_rval);
		JS_AddRoot(cx, &_arg);

		InitializeClassAudioMaster(cx, JS_GetGlobalObject(cx));
		InitializeClassVSTPlugin(cx, JS_GetGlobalObject(cx));

		vstPlugin = JS_DefineObject(cx, JS_GetGlobalObject(cx), "vstPlugin", classVSTPlugin, NULL, NULL);
		JS_SetPrivate(cx, vstPlugin, this);

		ExecuteScript(cx, "vstPlugin.js", false, 0, NULL, &_rval);

//		const char *pdir = (const char*)this->getDirectory(); // FSSpec on MAC, else char* 
//		char scriptFileName[PATH_MAX +1];
//		strcpy( scriptFileName, pdir );
//		strcat( scriptFileName, "vst.js" );
	}

	void SetNumPrograms( VstInt32 numPrograms ) {
	
		cEffect.numPrograms = numPrograms;
	}

	void SetNumParams( VstInt32 numParams ) {
	
		cEffect.numParams = numParams;
	}

	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

		jsval fval = GetFunction( "dispatcher" );
		if ( fval != JSVAL_VOID ) {

			JS_NewNumberValue(cx, opt, &_arg);
			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 4, INT_TO_JSVAL(opcode), INT_TO_JSVAL(index), INT_TO_JSVAL(value), _arg ); // , STRING_TO_JSVAL(JS_NewStringCopyZ(cx, ptr))
		}
		return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
	}


	void open() {
		
		jsval fval = GetFunction( "open" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0 );
			if ( res == JS_TRUE )
				return;
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::open();
	}


	void close() {

		jsval fval = GetFunction( "close" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0 );
			if ( res == JS_TRUE )
				return;
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::close();
	}


	// H->P 
	void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {

		float *in1 = inputs[0];
		float *in2 = inputs[1];
		float *out1 = outputs[0]; 
		float *out2 = outputs[1]; 
		while (--sampleFrames >= 0) { 

			(*out1++) = (*in1++); 
			(*out2++) = (*in2++); 
		} 
	}

	// H->P Called when a parameter changed.
	void setParameter(VstInt32 index, float value) {
		
		jsval fval = GetFunction( "setParameter" );
		if ( fval != JSVAL_VOID ) {

			JS_NewNumberValue(cx, value, &_arg);
			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 2, INT_TO_JSVAL(index), _arg );
			if ( res == JS_TRUE )
				return;
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::setParameter(index, value);
	}

	// H->P 
	float getParameter(VstInt32 index) {

		jsval fval = GetFunction( "getParameter" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(index) );
			if ( res == JS_TRUE && JSVAL_IS_NUMBER(_rval) ) {

				jsdouble d;
				JS_ValueToNumber(cx, _rval, &d);
				return d;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getParameter(index);
	}

	// H->P Return the index to the current program. 
	VstInt32 getProgram() {
		
		jsval fval = GetFunction( "getProgram" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0);
			if ( res == JS_TRUE && JSVAL_IS_INT(_rval) )
				return JSVAL_TO_INT(_rval);
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getProgram();
	}

	// H->P Set the current program.
	void setProgram(VstInt32 program) {

		jsval fval = GetFunction( "setProgram" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(program));
			if ( res == JS_TRUE )
				return;
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::setProgram(program);
	}

	// H->P Stuff the name field of the current program with name. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void setProgramName(char *name) {
	
		jsval fval = GetFunction( "setProgramName" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, name)));
			if ( res == JS_TRUE )
				return;
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::setProgramName(name);
	}

	// H->P Stuff name with the name of the current program. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void getProgramName(char *name) {

		jsval fval = GetFunction( "getProgramName" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0);
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(name, JS_GetStringBytes(jsstr), kVstMaxProgNameLen); // truncate
				name[kVstMaxProgNameLen-1] = '\0';
				return;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getProgramName(name);
	}

	// H->P Stuff label with the units in which parameter index is displayed.
	void getParameterLabel(VstInt32 index, char* label) {

		jsval fval = GetFunction( "getParameterLabel" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(index));
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(label, JS_GetStringBytes(jsstr), kVstMaxParamStrLen); // truncate
				label[kVstMaxParamStrLen-1] = '\0';
				return;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getParameterLabel(index, label);
	}

	// H->P Stuff text with a string representation ("0.5", "-3", "PLATE", etc...) of the value of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterDisplay(VstInt32 index, char *text) {

		jsval fval = GetFunction( "getParameterDisplay" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(index));
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(text, JS_GetStringBytes(jsstr), kVstMaxParamStrLen); // truncate
				text[kVstMaxParamStrLen-1] = '\0';
				return;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getParameterDisplay(index, text);
	}

	// H->P Stuff text with the name ("Time", "Gain", "RoomType", etc...) of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterName(VstInt32 index, char* text) {

		jsval fval = GetFunction( "getParameterName" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(index));
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(text, JS_GetStringBytes(jsstr), kVstMaxParamStrLen); // truncate
				text[kVstMaxParamStrLen-1] = '\0';
				return;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getParameterName(index, text);
	}


	// H->P
	//Reports what the plug-in is able to do (plugCanDos in audioeffectx.cpp).
	//Report what the plug-in is able to do. In general you can but don't have to report whatever you support or not support via canDo. Some application functionality may require some specific reply, but in that case you will probably know. Best is to report whatever you know for sure. A Host application cannot make assumptions about the presence of the new 2.x features of a plug-in. Ignoring this inquiry methods and trying to access a 2.x feature from a 1.0 plug, or vice versa, will mean the plug-in or Host application will break. It is not the end-users job to pick and choose which plug-ins can be supported by which Host.
	//Parameters: text 	A string from plugCanDos
	//Returns:
	//        * 0: don't know (default)
	//        * 1: yes
	//        * -1: no
	//Note:
	//    This should be supported. 
	VstInt32 canDo(char* text) {

		jsval fval = GetFunction( "canDo" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, text)));
			if ( res == JS_TRUE ) {
				
				if ( JSVAL_IS_VOID(_rval) )
					return 0; // 0: don't know (default)
				JSBool b;
				JS_ValueToBoolean(cx, _rval, &b);
				return b == JS_TRUE ? 1 : -1; // 1: yes, -1: no
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::canDo(text);
	}

	// H->P Fill text with name of program index (category deprecated in VST 2.4). Allows a Host application to list the plug-in's programs (presets).
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {

		jsval fval = GetFunction( "getProgramNameIndexed" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, INT_TO_JSVAL(index));
			if ( res == JS_TRUE ) {

				if ( !JSVAL_IS_STRING(_rval) ) // end of list
					return false;
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(text, JS_GetStringBytes(jsstr), kVstMaxProgNameLen); // truncate
				text[kVstMaxProgNameLen-1] = '\0';
				return true;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getProgramNameIndexed(category, index, text);
	}

	// H->P Specify a category that fits the plug (VstPlugCategory). 
	VstPlugCategory getPlugCategory () {

		jsval fval = GetFunction( "getPlugCategory" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0);
			if ( res == JS_TRUE && JSVAL_IS_INT(_rval) ) {
				
				int cat = JSVAL_TO_INT(_rval);
				if ( cat < 0 || cat >= kPlugCategMaxCount )
					return kPlugCategUnknown;
				return (VstPlugCategory)cat;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getPlugCategory();
	}

	// H->P Fill text with a string identifying the product name. text 	A string up to 64 chars 
	bool getProductString(char* text) {

		jsval fval = GetFunction( "getProductString" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0);
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(text, JS_GetStringBytes(jsstr), kVstMaxProductStrLen); // truncate
				text[kVstMaxProductStrLen-1] = '\0';
				return true;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getProductString(text);
	}

	// H->P Fill text with a string identifying the vendor.  text 	A string up to 64 chars 
	bool getVendorString(char* text) {
		
		jsval fval = GetFunction( "getVendorString" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 0);
			if ( res == JS_TRUE ) {
				
				JSString *jsstr = JS_ValueToString(cx, _rval);
				_rval = STRING_TO_JSVAL(jsstr);
				strncpy(text, JS_GetStringBytes(jsstr), kVstMaxVendorStrLen); // truncate
				text[kVstMaxVendorStrLen-1] = '\0';
				return true;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::getVendorString(text);
	}


	// H->P For 'soft-bypass' (this could be automated (in Audio Thread) that why you could NOT call iochanged (if needed) in this function, do it in fxidle).
   //      process still called (if Supported) although the plug-in was bypassed. Some plugs need to stay 'alive' even when bypassed.
	//      An example is a surround decoder which has more inputs than outputs and must maintain some reasonable signal distribution even when being bypassed.
	//      A CanDo 'bypass' allows to ask the plug-in if it supports soft bypass or not.
	//      Returns:
	//			  true: supports SoftBypass, process will be called, the plug-in should compensate its latency, and copy inputs to outputs
	//			  false: doesn't support SoftBypass, process will not be called, the Host should bypass the process call
	bool setBypass(bool onOff) {

		jsval fval = GetFunction( "setBypass" );
		if ( fval != JSVAL_VOID ) {

			JSBool res = JL_CallFunction(cx, vstPlugin, fval, &_rval, 1, onOff ? JS_TRUE : JS_FALSE);
			if ( res == JS_TRUE ) {
				
				JSBool b;
				JS_ValueToBoolean(cx, _rval, &b);
				return b == JS_TRUE ? true : false;
			}
		}
		MaybeReportException(); // let the script manage its errors.
		return AudioEffectX::setBypass(onOff);
	}

	bool string2parameter(long index, char *text) {

		// ???
		return AudioEffectX::string2parameter(index, text);
	}
};




/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( VSTPlugin )


DEFINE_PROPERTY( canProcessReplacing ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_BOOLEAN( *vp );
		vstPlugin->canProcessReplacing( JSVAL_TO_BOOLEAN(*vp) == JS_TRUE ? true : false );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numPrograms ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->SetNumPrograms( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numParams ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->SetNumParams( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numInputs ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->setNumInputs( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numOutputs ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->setNumOutputs( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( uniqueID ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_STRING( *vp );
		JSString *jsstr = JS_ValueToString(cx, *vp);
		J_S_ASSERT( JS_GetStringLength(jsstr) == 4, "Invalid ID length" );
		char *str = JS_GetStringBytes(jsstr);
		VstInt32 id = CCONST( str[0], str[1], str[2], str[3] );
		vstPlugin->setUniqueID( id );
	}
	return JS_TRUE;
}


DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_HAS_INSTANCE

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE( numPrograms )
		PROPERTY_WRITE_STORE( numParams )
		PROPERTY_WRITE_STORE( numInputs )
		PROPERTY_WRITE_STORE( numOutputs )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE( kPlugCategUnknown	     ) //< Unknown, category not implemented
		CONST_INTEGER_SINGLE( kPlugCategEffect		     )	//< Simple Effect
		CONST_INTEGER_SINGLE( kPlugCategSynth		     )	//< VST Instrument (Synths, samplers,...)
		CONST_INTEGER_SINGLE( kPlugCategAnalysis		  )	//< Scope, Tuner, ...
		CONST_INTEGER_SINGLE( kPlugCategMastering		  )	//< Dynamics, ...
		CONST_INTEGER_SINGLE( kPlugCategSpacializer	  )	//< Panners, ...
		CONST_INTEGER_SINGLE( kPlugCategRoomFx			  )	//< Delays and Reverbs
		CONST_INTEGER_SINGLE( kPlugSurroundFx			  )	//< Dedicated surround processor
		CONST_INTEGER_SINGLE( kPlugCategRestoration	  )	//< Denoiser, ...
		CONST_INTEGER_SINGLE( kPlugCategOfflineProcess )	//< Offline Process
		CONST_INTEGER_SINGLE( kPlugCategShell			  )	//< Plug-in is container of other plug-ins  @see effShellGetNextPlugin
		CONST_INTEGER_SINGLE( kPlugCategGenerator		  )	//< ToneGenerator, ...
		
		// opcode names
		// vst 1.x
		CONST_INTEGER_SINGLE( effOpen )
		CONST_INTEGER_SINGLE( effClose )
		CONST_INTEGER_SINGLE( effSetProgram )
		CONST_INTEGER_SINGLE( effGetProgram )
		CONST_INTEGER_SINGLE( effSetProgramName )
		CONST_INTEGER_SINGLE( effGetProgramName )
		CONST_INTEGER_SINGLE( effGetParamLabel )
		CONST_INTEGER_SINGLE( effGetParamDisplay )
		CONST_INTEGER_SINGLE( effGetParamName )
//		CONST_INTEGER_SINGLE( effGetVu )
		CONST_INTEGER_SINGLE( effSetSampleRate )
		CONST_INTEGER_SINGLE( effSetBlockSize )
		CONST_INTEGER_SINGLE( effMainsChanged )
		CONST_INTEGER_SINGLE( effEditGetRect )
		CONST_INTEGER_SINGLE( effEditOpen )
		CONST_INTEGER_SINGLE( effEditClose )
//		CONST_INTEGER_SINGLE( effEditDraw )
//		CONST_INTEGER_SINGLE( effEditMouse )
//		CONST_INTEGER_SINGLE( effEditKey )
//		CONST_INTEGER_SINGLE( effEditIdle )
//		CONST_INTEGER_SINGLE( effEditTop )
//		CONST_INTEGER_SINGLE( effEditSleep )
//		CONST_INTEGER_SINGLE( effIdentify )
		CONST_INTEGER_SINGLE( effGetChunk )
		CONST_INTEGER_SINGLE( effSetChunk )

		// vst 2.4
		CONST_INTEGER_SINGLE(effProcessEvents)
		CONST_INTEGER_SINGLE(effCanBeAutomated)
		CONST_INTEGER_SINGLE(effString2Parameter)
//		CONST_INTEGER_SINGLE(effGetNumProgramCategories)
		CONST_INTEGER_SINGLE(effGetProgramNameIndexed)
//		CONST_INTEGER_SINGLE(effCopyProgram)
//		CONST_INTEGER_SINGLE(effConnectInput)
//		CONST_INTEGER_SINGLE(effConnectOutput)
		CONST_INTEGER_SINGLE(effGetInputProperties)
		CONST_INTEGER_SINGLE(effGetOutputProperties)
		CONST_INTEGER_SINGLE(effGetPlugCategory)
//		CONST_INTEGER_SINGLE(effGetCurrentPosition)
//		CONST_INTEGER_SINGLE(effGetDestinationBuffer)
		CONST_INTEGER_SINGLE(effOfflineNotify)
		CONST_INTEGER_SINGLE(effOfflinePrepare)
		CONST_INTEGER_SINGLE(effOfflineRun)
		CONST_INTEGER_SINGLE(effProcessVarIo)
		CONST_INTEGER_SINGLE(effSetSpeakerArrangement)
//		CONST_INTEGER_SINGLE(effSetBlockSizeAndSampleRate)
		CONST_INTEGER_SINGLE(effSetBypass)
		CONST_INTEGER_SINGLE(effGetEffectName)
//		CONST_INTEGER_SINGLE(effGetErrorText)
		CONST_INTEGER_SINGLE(effGetVendorString)
		CONST_INTEGER_SINGLE(effGetProductString)
		CONST_INTEGER_SINGLE(effGetVendorVersion)
		CONST_INTEGER_SINGLE(effVendorSpecific)
		CONST_INTEGER_SINGLE(effCanDo)
		CONST_INTEGER_SINGLE(effGetTailSize)
//		CONST_INTEGER_SINGLE(effIdle)
//		CONST_INTEGER_SINGLE(effGetIcon)
//		CONST_INTEGER_SINGLE(effSetViewPosition)
		CONST_INTEGER_SINGLE(effGetParameterProperties)
//		CONST_INTEGER_SINGLE(effKeysRequired)
		CONST_INTEGER_SINGLE(effGetVstVersion)
		CONST_INTEGER_SINGLE(effEditKeyDown)
		CONST_INTEGER_SINGLE(effEditKeyUp)
		CONST_INTEGER_SINGLE(effSetEditKnobMode)
		CONST_INTEGER_SINGLE(effGetMidiProgramName)
		CONST_INTEGER_SINGLE(effGetCurrentMidiProgram)
		CONST_INTEGER_SINGLE(effGetMidiProgramCategory)
		CONST_INTEGER_SINGLE(effHasMidiProgramsChanged)
		CONST_INTEGER_SINGLE(effGetMidiKeyName)
		CONST_INTEGER_SINGLE(effBeginSetProgram)
		CONST_INTEGER_SINGLE(effEndSetProgram)
		CONST_INTEGER_SINGLE(effGetSpeakerArrangement)
		CONST_INTEGER_SINGLE(effShellGetNextPlugin)
		CONST_INTEGER_SINGLE(effStartProcess)
		CONST_INTEGER_SINGLE(effStopProcess)
		CONST_INTEGER_SINGLE(effSetTotalSampleToProcess)
		CONST_INTEGER_SINGLE(effSetPanLaw)
		CONST_INTEGER_SINGLE(effBeginLoadBank)
		CONST_INTEGER_SINGLE(effBeginLoadProgram)
		CONST_INTEGER_SINGLE(effSetProcessPrecision)
		CONST_INTEGER_SINGLE(effGetNumMidiInputChannels)
		CONST_INTEGER_SINGLE(effGetNumMidiOutputChannels)
	END_CONST_INTEGER_SPEC

END_CLASS



extern "C" {

#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
	#define VST_EXPORT	__attribute__ ((visibility ("default")))
#else
	#define VST_EXPORT
#endif


VST_EXPORT AEffect* VSTPluginMain(audioMasterCallback audioMaster) {

	// Get VST Version of the Host
	if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version
	// Create the AudioEffect
	AudioEffect* effect = new JsVst(audioMaster);
	// Return the VST AEffect structur
	return effect->getAeffect ();
}

// support for old hosts not looking for VSTPluginMain
#if (T_argET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif WIN32
VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif BEOS
VST_EXPORT AEffect* main_plugin (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#endif

} // extern "C"

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>

void* hInstance;

extern "C" {
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hInst;
	return 1;
}
} // extern "C"
#endif


/* functions called by the host:

// vst 1.x
		open ();
		close ();
		setProgram ((VstInt32)value);
		v = getProgram ();
		setProgramName ((char*)ptr);
		getProgramName ((char*)ptr);
		getParameterLabel (index, (char*)ptr);
		getParameterDisplay (index, (char*)ptr);
		getParameterName (index, (char*)ptr);
		setSampleRate (opt);
		setBlockSize ((VstInt32)value);
		suspend ();
		resume ();
		editor->getRect ((ERect**)ptr) ? 1 : 0;
		editor->open (ptr) ? 1 : 0;
		editor->close ();
		editor->idle ();
		editor->draw ((ERect*)ptr);				break;
		v = editor->mouse (index, value);		break;
		v = editor->key (value);				break;
		editor->top ();							break;
		editor->sleep ();						break;
		getChunk ((void**)ptr, index ? true : false);	break;
		setChunk (ptr, (VstInt32)value, index ? true : false);	break;


vst 2.x
			v = processEvents ((VstEvents*)ptr);
			v = canParameterBeAutomated (index) ? 1 : 0;
			v = string2parameter (index, (char*)ptr) ? 1 : 0;
			v = getProgramNameIndexed ((VstInt32)value, index, (char*)ptr) ? 1 : 0;
			v = getNumCategories ();
			v = copyProgram (index) ? 1 : 0;
			inputConnected (index, value ? true : false);
			outputConnected (index, value ? true : false);
			v = getInputProperties (index, (VstPinProperties*)ptr) ? 1 : 0;
			v = getOutputProperties (index, (VstPinProperties*)ptr) ? 1 : 0;
			v = (VstIntPtr)getPlugCategory ();
			v = reportCurrentPosition ();
			v = ToVstPtr<float> (reportDestinationBuffer ());
			v = offlineNotify ((VstAudioFile*)ptr, (VstInt32)value, index != 0);
			v = offlinePrepare ((VstOfflineTask*)ptr, (VstInt32)value);
			v = offlineRun ((VstOfflineTask*)ptr, (VstInt32)value);
			v = setSpeakerArrangement (FromVstPtr<VstSpeakerArrangement> (value), (VstSpeakerArrangement*)ptr) ? 1 : 0;
			v = processVariableIo ((VstVariableIo*)ptr) ? 1 : 0;
			setBlockSizeAndSampleRate ((VstInt32)value, opt);
			v = setBypass (value ? true : false) ? 1 : 0;
			v = getEffectName ((char*)ptr) ? 1 : 0;
			v = getVendorString ((char*)ptr) ? 1 : 0;
			v = getProductString ((char*)ptr) ? 1 : 0;
			v = getVendorVersion ();
			v = vendorSpecific (index, value, ptr, opt);
			v = canDo ((char*)ptr);
			v = getGetTailSize ();
			v = getErrorText ((char*)ptr) ? 1 : 0;
			v = ToVstPtr<void> (getIcon ());
			v = setViewPosition (index, (VstInt32)value) ? 1 : 0;
			v = fxIdle ();
			v = (keysRequired () ? 0 : 1);	// reversed to keep v1 compatibility
			v = getParameterProperties (index, (VstParameterProperties*)ptr) ? 1 : 0;
			v = getVstVersion ();
				v = editor->onKeyDown (keyCode) ? 1 : 0;
				v = editor->onKeyUp (keyCode) ? 1 : 0;
				v = editor->setKnobMode ((VstInt32)value) ? 1 : 0;
			v = getMidiProgramName (index, (MidiProgramName*)ptr);
			v = getCurrentMidiProgram (index, (MidiProgramName*)ptr);
			v = getMidiProgramCategory (index, (MidiProgramCategory*)ptr);
			v = hasMidiProgramsChanged (index) ? 1 : 0;
			v = getMidiKeyName (index, (MidiKeyName*)ptr) ? 1 : 0;
			v = beginSetProgram () ? 1 : 0;
			v = endSetProgram () ? 1 : 0;
			v = getSpeakerArrangement (FromVstPtr<VstSpeakerArrangement*> (value), (VstSpeakerArrangement**)ptr) ? 1 : 0;
			v = setTotalSampleToProcess ((VstInt32)value);
			v = getNextShellPlugin ((char*)ptr);
			v = startProcess ();
			v = stopProcess ();
			v = setPanLaw ((VstInt32)value, opt) ? 1 : 0;
			v = beginLoadBank ((VstPatchChunkInfo*)ptr);
			v = beginLoadProgram ((VstPatchChunkInfo*)ptr);
			v = setProcessPrecision ((VstInt32)value) ? 1 : 0;
			v = getNumMidiInputChannels ();
			v = getNumMidiOutputChannels ();

*/

