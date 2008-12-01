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

#include "../host/host.h"
#include "../common/jsClass.h"
#include "jsapihelper.h"

#include <public.sdk/source/vst2.x/audioeffectx.h>

#include "audiomaster.h"
#include "midievent.h"

DECLARE_CLASS( MidiEvent );
DECLARE_CLASS( AudioMaster );
DECLARE_CLASS( VSTPlugin );

class JsVst : public AudioEffectX, private JSApiHelper {
private:

	JSContext *_cx;
	JSObject *vstPlugin;
	jsval _arg, _rval;

	void ManageException() {
	
		if ( JS_IsExceptionPending(_cx) ) {

			jsval fval, rval, ex;
			fval = GetProperty(vstPlugin, "catch");
			if ( JsvalIsFunction(fval) ) {

				JS_GetPendingException(_cx, &ex);
				JS_CallFunctionValue(_cx, vstPlugin, fval, 1, &ex, &rval);
				if ( JSVAL_IS_VOID(rval) )
					JS_ClearPendingException(_cx);
				else
					JS_SetPendingException(_cx, rval);
			}

			if ( JS_IsExceptionPending(_cx) ) {
				
				JS_ReportPendingException(_cx);
				JS_ClearPendingException(_cx);
			}
		}
	}

public:

	~JsVst() {

		JS_RemoveRoot(_cx, &_arg);
		JS_RemoveRoot(_cx, &_rval);
		DestroyHost(_cx);
	}

	JsVst( audioMasterCallback audioMaster ) : AudioEffectX(audioMaster, 0, 0), JSApiHelper(_cx) {

		_cx = CreateHost(-1, -1, 0);
		InitHost(_cx, true, NULL, NULL, NULL);
		JS_SetOptions(_cx, JSOPTION_DONT_REPORT_UNCAUGHT);

		JS_AddRoot(_cx, &_rval);
		JS_AddRoot(_cx, &_arg);

		InitializeClassMidiEvent(_cx, JS_GetGlobalObject(_cx));
		InitializeClassAudioMaster(_cx, JS_GetGlobalObject(_cx));
		InitializeClassVSTPlugin(_cx, JS_GetGlobalObject(_cx));

		JSObject *audioMasterObject = CreateAudioMasterObject(_cx, audioMaster);
		_arg = OBJECT_TO_JSVAL(audioMasterObject);
		JS_SetProperty(_cx, JS_GetGlobalObject(_cx), "audioMaster", &_arg);

		vstPlugin = JS_DefineObject(_cx, JS_GetGlobalObject(_cx), "vstPlugin", classVSTPlugin, NULL, NULL);
		JS_SetPrivate(_cx, vstPlugin, this);

		JSBool status = ExecuteScriptFileName(_cx, "vstPlugin.js", false, 0, NULL, &_rval);
//		if ( !status )
//			MessageBox(NULL, "script compilation error", "Error", 0);



//		const char *pdir = (const char*)this->getDirectory(); // FSSpec on MAC, else char* 
//		char scriptFileName[PATH_MAX +1];
//		strcpy( scriptFileName, pdir );
//		strcat( scriptFileName, "vst.js" );
	}

	// P->H
	void SetNumPrograms( VstInt32 numPrograms ) {
	
		cEffect.numPrograms = numPrograms;
	}

	// P->H
	void SetNumParams( VstInt32 numParams ) {
	
		cEffect.numParams = numParams;
	}

private:

	// H->P
	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

		try {

			jsval fval = GetProperty(vstPlugin, "dispatcher");
			
			// for DEBUG only ?
			if ( JsvalIsFunction(fval) ) {

				char *tmp = (char*)ptr;
/*
				switch (opcode) {
					case effCanDo:
					case effGetParamDisplay:
						break;
					default:
						tmp = NULL;
				}
*/
				FunctionCall5(vstPlugin, fval, IntToJsval(opcode), IntToJsval(index), IntToJsval(value), StringToJsval(""), RealToJsval(opt) ); // , STRING_TO_JSVAL(JS_NewStringCopyZ(cx, ptr))
			}
		} catch( JsException ) { ManageException(); }

		return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
	}

	void open() {

		try {
			jsval fval = GetProperty(vstPlugin, "open");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall0(vstPlugin, fval);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::open();
	}

	void close() {

		try {
			jsval fval = GetProperty(vstPlugin, "close");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall0(vstPlugin, fval);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::close();
	}

	void suspend() {

		try {
			jsval fval = GetProperty(vstPlugin, "suspend");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall0(vstPlugin, fval);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::suspend();
	}

	void resume() {

		try {
			jsval fval = GetProperty(vstPlugin, "resume");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall0(vstPlugin, fval);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::resume();
	}

	VstInt32 startProcess() {

		try {
			jsval fval = GetProperty(vstPlugin, "startProcess");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return 1; // (TBD) ???
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::startProcess();
	}

	VstInt32 stopProcess() {

		try {
			jsval fval = GetProperty(vstPlugin, "stopProcess");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return 1; // (TBD) ???
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::stopProcess();
	}


	// H->P Specify a category that fits the plug (VstPlugCategory).
	// (TBD) move to a class variable
	VstPlugCategory getPlugCategory () {

		try {
			jsval fval = GetProperty(vstPlugin, "getPlugCategory");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall0(vstPlugin, fval);
				if ( JSVAL_IS_INT(_rval) ) {
					
					int cat = JSVAL_TO_INT(_rval);
					if ( cat < 0 || cat >= kPlugCategMaxCount )
						return kPlugCategUnknown;
					return (VstPlugCategory)cat;
				}
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getPlugCategory();
	}

	// H->P Fill text with a string identifying the product name. text 	A string up to 64 chars
	// (TBD) move to a class variable
	bool getProductString(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProductString");
			if ( JsvalIsFunction(fval) ) {

				CopyJsvalToString(FunctionCall0(vstPlugin, fval), text, kVstMaxProductStrLen);
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProductString(text);
	}

	// H->P Fill text with a string identifying the vendor.  text 	A string up to 64 chars 
	// (TBD) move to a class variable
	bool getVendorString(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getVendorString");
			if ( JsvalIsFunction(fval) ) {

				CopyJsvalToString(FunctionCall0(vstPlugin, fval), text, kVstMaxVendorStrLen);
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getVendorString(text);
	}

	// H->P Return vendor-specific version.
	// (TBD) move to a class variable
	VstInt32 getVendorVersion() {

		try {
			jsval fval = GetProperty(vstPlugin, "getVendorVersion");
			if ( JsvalIsFunction(fval) )
				return JsvalToInt(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getVendorVersion();
	}


	// H->P Fill text with a string identifying the effect. 
	// (TBD) move to a class variable
	bool getEffectName(char* name) {

		try {
			jsval fval = GetProperty(vstPlugin, "getEffectName");
			if ( JsvalIsFunction(fval) )
				return CopyJsvalToString(FunctionCall0(vstPlugin, fval), name, kVstMaxEffectNameLen), true;
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getEffectName(name);
	}


	// H->P 
	void processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames) {
/*
		float *in1 = inputs[0];
		float *in2 = inputs[1];
		float *out1 = outputs[0]; 
		float *out2 = outputs[1]; 
		while (--sampleFrames >= 0) { 

			(*out1++) = (*in1++); 
			(*out2++) = (*in2++); 
		} 
*/
	}


	// H->P 
	VstInt32 processEvents(VstEvents* events) {

		try {
			jsval jsProcessMidiEvent = GetProperty(vstPlugin, "procesMidiEvent");
			if ( !JsvalIsFunction(jsProcessMidiEvent) ) // optimization
				jsProcessMidiEvent = JSVAL_VOID;
			jsval jsSysExEvent = GetProperty(vstPlugin, "procesSysExEvent");
			if ( !JsvalIsFunction(jsSysExEvent) ) // optimization
				jsSysExEvent = JSVAL_VOID;

			for ( int i = 0; i < events->numEvents; i++ ) {

				if ( events->events[i]->type == kVstMidiType && !JSVAL_IS_VOID(jsProcessMidiEvent) ) {

					JSObject *jsMidiEvent = CreateMidiEventObject(_cx, (VstMidiEvent*)events->events[i]);
					FunctionCall1(vstPlugin, jsProcessMidiEvent, OBJECT_TO_JSVAL(jsMidiEvent));
				} else
				if ( events->events[i]->type == kVstSysExType && !JSVAL_IS_VOID(jsSysExEvent) ) {

					// (TBD)
				}
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::processEvents(events);
	}


	bool canParameterBeAutomated(VstInt32 index) {

		try {
			jsval fval = GetProperty(vstPlugin, "canParameterBeAutomated");
			if ( JsvalIsFunction(fval) )
				return JsvalToBool(FunctionCall1(vstPlugin, fval, IntToJsval(index)));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::canParameterBeAutomated(index);
	}


	void setParameterAutomated(VstInt32 index, float value) {

		try {
			jsval fval = GetProperty(vstPlugin, "setParameterAutomated");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall2(vstPlugin, fval, IntToJsval(index), RealToJsval(value));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setParameterAutomated(index, value);
	}


	// H->P Called when a parameter changed.
	void setParameter(VstInt32 index, float value) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "setParameter");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall2(vstPlugin, fval, IntToJsval(index), RealToJsval(value));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setParameter(index, value);
	}


	// first call: return 1 = plug CAN convert string2Parameter, 0 = plug CANNOT convert string2Parameter
	// next calls: return 1 = conversion successful, 0 = fail
	bool string2parameter(long index, char *text) {

		try {
			jsval fval = GetProperty(vstPlugin, "string2parameter");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(index), StringToJsval(text) );
				if ( text == NULL )
					return JsvalToBool(_rval);

				if ( JSVAL_IS_NUMBER(_rval) ) {
					setParameter(index, JsvalToReal(_rval));
					return true;
				} else
					return false;
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::string2parameter(index, text);
	}

	// H->P 
	float getParameter(VstInt32 index) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "getParameter");
			if ( JsvalIsFunction(fval) )
				return JsvalToReal(FunctionCall1(vstPlugin, fval, IntToJsval(index)));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameter(index);
	}


	// return: 1 = successful
	bool getParameterProperties(VstInt32 index, VstParameterProperties* p) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterProperties");
			if ( JsvalIsFunction(fval) ) {

				JSObject *obj = JS_NewObject(_cx, NULL, NULL, NULL);
				FunctionCall1(vstPlugin, fval, OBJECT_TO_JSVAL(obj));
				
				p->flags = 0;

				CopyJsvalToString(AssertDefined(GetProperty(obj, "label")), p->label, kVstMaxLabelLen);
				CopyJsvalToString(AssertDefined(GetProperty(obj, "shortLabel")), p->shortLabel, kVstMaxShortLabelLen);
// (TBD) finish !			
			}

		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterProperties(index, p);
	}

	// H->P Stuff label with the units in which parameter index is displayed.
	void getParameterLabel(VstInt32 index, char* label) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterLabel");
			if ( JsvalIsFunction(fval) )
				return CopyJsvalToString(FunctionCall1(vstPlugin, fval, IntToJsval(index)), label, kVstMaxParamStrLen);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterLabel(index, label);
	}

	// H->P Stuff text with a string representation ("0.5", "-3", "PLATE", etc...) of the value of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterDisplay(VstInt32 index, char *text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterDisplay");
			if ( JsvalIsFunction(fval) )
				return CopyJsvalToString(FunctionCall1(vstPlugin, fval, IntToJsval(index)), text, kVstMaxParamStrLen);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterDisplay(index, text);
	}

	// H->P Stuff text with the name ("Time", "Gain", "RoomType", etc...) of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterName(VstInt32 index, char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterName");
			if ( JsvalIsFunction(fval) )
				return CopyJsvalToString(FunctionCall1(vstPlugin, fval, IntToJsval(index)), text, kVstMaxParamStrLen);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterName(index, text);
	}


	// H->P Return the index to the current program. 
	VstInt32 getProgram() {
		
		try {
			jsval fval = GetProperty(vstPlugin, "getProgram");
			if ( JsvalIsFunction(fval) )
				return JsvalToInt(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProgram();
	}

	// H->P Set the current program.
	void setProgram(VstInt32 program) {

		try {
			jsval fval = GetProperty(vstPlugin, "setProgram");
			if ( JsvalIsFunction(fval) )
				FunctionCall1(vstPlugin, fval, IntToJsval(program));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setProgram(program);
	}

	bool beginSetProgram() {
	
		try {
			jsval fval = GetProperty(vstPlugin, "beginSetProgram");
			if ( JsvalIsFunction(fval) )
				return JsvalToBool(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::beginSetProgram();
	}

	bool endSetProgram() {
	
		try {
			jsval fval = GetProperty(vstPlugin, "endSetProgram");
			if ( JsvalIsFunction(fval) )
				return JsvalToBool(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::endSetProgram();
	}


	// H->P Stuff the name field of the current program with name. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void setProgramName(char *name) {
	
		try {
			jsval fval = GetProperty(vstPlugin, "setProgramName");
			if ( JsvalIsFunction(fval) )
				return (void) FunctionCall1(vstPlugin, fval, StringToJsval(name));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setProgramName(name);
	}

	// H->P Stuff name with the name of the current program. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void getProgramName(char *name) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProgramName");
			if ( JsvalIsFunction(fval) )
				return CopyJsvalToString(FunctionCall0(vstPlugin, fval), name, kVstMaxProgNameLen);
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProgramName(name);
	}


	// H->P
	VstInt32 canDo(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "canDo");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, StringToJsval(text));
				if ( JSVAL_IS_VOID(_rval) || _rval == JSVAL_ZERO )
						return 0; // 0: don't know (default)
				return JsvalToBool(_rval) ? 1 : -1; // 1: yes, -1: no
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::canDo(text);
	}

	// H->P Fill text with name of program index (category deprecated in VST 2.4). Allows a Host application to list the plug-in's programs (presets).
	// return: 1 = OK, 0 = fail
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProgramNameIndexed");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(category), IntToJsval(index));
				if ( JSVAL_IS_VOID(_rval) ) // end of list
					return false;
				CopyJsvalToString(_rval, text, kVstMaxProgNameLen);
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProgramNameIndexed(category, index, text);
	}


	// H->P For 'soft-bypass' (this could be automated (in Audio Thread) that why you could NOT call iochanged (if needed) in this function, do it in fxidle).
	bool setBypass(bool onOff) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "setBypass");
			if ( JsvalIsFunction(fval) )
				return JsvalToBool(FunctionCall1(vstPlugin, fval, BoolToJsval(onOff)));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setBypass(onOff);
	}


// <MIDI>

	VstInt32 getNumMidiInputChannels() {

		try {
			jsval fval = GetProperty(vstPlugin, "getNumMidiInputChannels");
			if ( JsvalIsFunction(fval) )
				return JsvalToInt(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getNumMidiInputChannels();
	}

	VstInt32 getNumMidiOutputChannels() {

		try {
			jsval fval = GetProperty(vstPlugin, "getNumMidiOutputChannels");
			if ( JsvalIsFunction(fval) )
				return JsvalToInt(FunctionCall0(vstPlugin, fval));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getNumMidiOutputChannels();
	}


	// Plugin Return true if the #MidiProgramNames, #MidiKeyNames or #MidiControllerNames had changed on this MIDI channel.
	bool hasMidiProgramsChanged(VstInt32 channel) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "hasMidiProgramsChanged");
			if ( JsvalIsFunction(fval) )
				return JsvalToBool(FunctionCall1(vstPlugin, fval, IntToJsval(channel)));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::hasMidiProgramsChanged(channel);
	}

	// return: number of used programIndexes. if 0 is returned, no MidiProgramNames supported.
	VstInt32 getMidiProgramName(VstInt32 channel, MidiProgramName* mpn) {

		try {
			jsval fval = GetProperty(vstPlugin, "getMidiProgramName");
			if ( JsvalIsFunction(fval) ) {

				JSObject *jsMpn = JS_NewObject(_cx, NULL, NULL, NULL);
				_rval = FunctionCall3(vstPlugin, fval, IntToJsval(channel), IntToJsval(mpn->thisProgramIndex), OBJECT_TO_JSVAL(jsMpn));
				if ( !JSVAL_IS_INT(_rval) )
					throw JsException(_cx, "invalid return value (need integer)");
				VstInt32 ret = JsvalToInt(_rval);
				if ( ret == 0 )
					return 0;
				CopyJsvalToString(GetProperty(jsMpn, "name"), mpn->name, kVstMaxNameLen);
				_arg = GetProperty(jsMpn, "midiProgram");
				mpn->midiProgram = JSVAL_IS_VOID(_arg) ? -1 : AssertRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "midiBankMsb");
				mpn->midiBankMsb = JSVAL_IS_VOID(_arg) ? -1 : AssertRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "midiBankLsb");
				mpn->midiBankLsb = JSVAL_IS_VOID(_arg) ? -1 : AssertRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "parentCategoryIndex");
				mpn->parentCategoryIndex = JSVAL_IS_VOID(_arg) ? -1 : JsvalToInt(_arg);
				mpn->flags = JsvalToBool(GetProperty(jsMpn, "isOmny")) ? kMidiIsOmni : 0;
				return ret;
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getMidiProgramName(channel, mpn);
	}


	VstInt32 getCurrentMidiProgram(VstInt32 channel, MidiProgramName* currentProgram)  {
		
		try {
			jsval fval = GetProperty(vstPlugin, "getCurrentMidiProgram");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, IntToJsval(channel));
				if ( !JSVAL_IS_INT(_rval) )
					throw JsException(_cx, "invalid return value (need integer)");
				VstInt32 prog = JsvalToInt(_rval);
				currentProgram->thisProgramIndex = prog;
				getMidiProgramName(channel, currentProgram); // (TBD) is this ok ?
				return prog;
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getCurrentMidiProgram(channel, currentProgram);
	}


/*
	// return: number of used categoryIndexes. if 0 is returned, no MidiProgramCategories supported.
	VstInt32 getMidiProgramCategory(VstInt32 channel, MidiProgramCategory* category) {

// [ ['name'], 

		try {
			jsval fval = GetProperty(vstPlugin, "getMidiProgramCategory");
			if ( JsvalIsFunction(fval) ) {

//				JSObject *jsMpn = JS_NewObject(_cx, NULL, NULL, NULL);

				_rval = FunctionCall2(vstPlugin, fval, JsvalToInt(channel), JsvalToInt(category->thisCategoryIndex) );
				
				category->flags = 0;

				if ( JSVAL_IS_VOID(_rval) ) {

					category->name[0] = '\0';
					category->parentCategoryIndex = -1; // -1:no parent category
					return 1;
				}
				CopyJsvalToString(_rval, category->name, kVstMaxNameLen);
???

// http://asseca.com/vst-24-specs/efGetMidiProgramCategory.html
				return 1; 
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getMidiProgramCategory(channel, category);
	}
*/

	bool getMidiKeyName(VstInt32 channel, MidiKeyName* keyName) {

		try {
			jsval fval = GetProperty(vstPlugin, "getMidiKeyName");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall3(vstPlugin, fval, JsvalToInt(channel), JsvalToInt(keyName->thisProgramIndex), JsvalToInt(keyName->thisKeyNumber) );
				if ( JSVAL_IS_VOID(_rval) )
					return false; // If 0 is returned, no MidiKeyNames are defined for 'thisProgramIndex'.
				CopyJsvalToString(_rval, keyName->keyName, kVstMaxNameLen);
				keyName->flags = 0;
				keyName->reserved = 0;
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getMidiKeyName(channel, keyName);
	}

// </MIDI>

	void setSampleRate(float sampleRate) {

		try {
			jsval fval = GetProperty(vstPlugin, "setSampleRate");
			if ( JsvalIsFunction(fval) )
				FunctionCall1(vstPlugin, fval, RealToJsval(sampleRate));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setSampleRate(sampleRate);
	}


	void setBlockSize(VstInt32 blockSize) {
	
		try {
			jsval fval = GetProperty(vstPlugin, "setBlockSize");
			if ( JsvalIsFunction(fval) )
				FunctionCall1(vstPlugin, fval, IntToJsval(sampleRate));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setBlockSize(sampleRate);
	}


	bool getInputProperties(VstInt32 index, VstPinProperties* properties) {

		try {
			jsval fval = GetProperty(vstPlugin, "getInputProperties");
			if ( JsvalIsFunction(fval) ) {

				JSObject *tmpObj = JS_NewObject(_cx, NULL, NULL, NULL);
				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(index), OBJECT_TO_JSVAL(tmpObj));
				if ( !JSVAL_IS_BOOLEAN(_rval) )
					throw JsException(_cx, "invalid return value (need boolean)");
				CopyJsvalToString(GetProperty(tmpObj, "label"), properties->label, kVstMaxLabelLen);
				CopyJsvalToString(GetProperty(tmpObj, "shortLabel"), properties->shortLabel, kVstMaxShortLabelLen);
				properties->flags = JsvalToInt(GetProperty(tmpObj, "flags"));
				properties->arrangementType = JsvalToInt(GetProperty(tmpObj, "arrangementType"));
				return JsvalToBool(_rval);
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getInputProperties(index, properties);
	}

	bool getOutputProperties(VstInt32 index, VstPinProperties* properties) {

		try {
			jsval fval = GetProperty(vstPlugin, "getOutputProperties");
			if ( JsvalIsFunction(fval) ) {

				JSObject *tmpObj = JS_NewObject(_cx, NULL, NULL, NULL);
				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(index), OBJECT_TO_JSVAL(tmpObj));
				if ( !JSVAL_IS_BOOLEAN(_rval) )
					throw JsException(_cx, "invalid return value (need boolean)");
				CopyJsvalToString(GetProperty(tmpObj, "label"), properties->label, kVstMaxLabelLen);
				CopyJsvalToString(GetProperty(tmpObj, "shortLabel"), properties->shortLabel, kVstMaxShortLabelLen);
				properties->flags = JsvalToInt(GetProperty(tmpObj, "flags"));
				properties->arrangementType = JsvalToInt(GetProperty(tmpObj, "arrangementType"));
				return JsvalToBool(_rval);
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getOutputProperties(index, properties);
	}


	bool getSpeakerArrangement(VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput) {

		return AudioEffectX::getSpeakerArrangement(pluginInput, pluginOutput);	
	}
	
	bool setSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) {

		return AudioEffectX::setSpeakerArrangement(pluginInput, pluginOutput);	
	}
};



/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( VSTPlugin )


DEFINE_PROPERTY( hostLanguage ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstInt32 lang = vstPlugin->getHostLanguage();
	*vp = INT_TO_JSVAL(lang);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( directory ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	void *dirName = vstPlugin->getDirectory();
	if ( dirName != NULL ) {
		
		*vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char*)dirName));
	} else {

		*vp = JSVAL_VOID;
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( canProcessReplacing ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_BOOLEAN( *vp );
		vstPlugin->canProcessReplacing( JSVAL_TO_BOOLEAN(*vp) == JS_TRUE ? true : false );
	}
	return JS_TRUE;
	JL_BAD;
}

/* perhaps later
DEFINE_PROPERTY( canDoubleReplacing ) { }
*/

DEFINE_PROPERTY( numPrograms ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	J_S_ASSERT_INT( *vp );
	vstPlugin->SetNumPrograms( JSVAL_TO_INT(*vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( numParams ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	J_S_ASSERT_INT( *vp );
	vstPlugin->SetNumParams( JSVAL_TO_INT(*vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( numInputs ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	J_S_ASSERT_INT( *vp );
	vstPlugin->setNumInputs( JSVAL_TO_INT(*vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( numOutputs ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->setNumOutputs( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( samplePos ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(0); // samplePos always valid
	if ( info != NULL )
		J_CHK( JS_NewNumberValue(cx, info->samplePos, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( sampleRate ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(0); // sampleRate always valid
	if ( info != NULL )
		J_CHK( JS_NewNumberValue(cx, info->sampleRate, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( nanoSeconds ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstNanosValid);
	if ( info != NULL || (info->flags & kVstNanosValid) )
		J_CHK( JS_NewNumberValue(cx, info->nanoSeconds, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( ppqPos ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstPpqPosValid);
	if ( info != NULL || (info->flags & kVstPpqPosValid) )
		J_CHK( JS_NewNumberValue(cx, info->ppqPos, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( tempo ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstTempoValid);
	if ( info != NULL || (info->flags & kVstTempoValid) )
		J_CHK( JS_NewNumberValue(cx, info->tempo, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( barStartPos ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstBarsValid);
	if ( info != NULL || (info->flags & kVstBarsValid) )
		J_CHK( JS_NewNumberValue(cx, info->barStartPos, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( cycleStartPos ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstCyclePosValid);
	if ( info != NULL || (info->flags & kVstCyclePosValid) )
		J_CHK( JS_NewNumberValue(cx, info->cycleStartPos, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( cycleEndPos ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstCyclePosValid);
	if ( info != NULL || (info->flags & kVstCyclePosValid) )
		J_CHK( JS_NewNumberValue(cx, info->cycleEndPos, vp ) );
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( timeSigNumerator ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstTimeSigValid);
	if ( info != NULL || (info->flags & kVstTimeSigValid) )
		*vp = INT_TO_JSVAL(info->timeSigNumerator);
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( timeSigDenominator ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstTimeSigValid);
	if ( info != NULL || (info->flags & kVstTimeSigValid) )
		*vp = INT_TO_JSVAL(info->timeSigDenominator);
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( smpteOffset ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstSmpteValid);
	if ( info != NULL || (info->flags & kVstSmpteValid) )
		*vp = INT_TO_JSVAL(info->smpteOffset);
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( smpteFrameRate ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstSmpteValid);
	if ( info != NULL || (info->flags & kVstSmpteValid) )
		*vp = INT_TO_JSVAL(info->smpteFrameRate);
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( samplesToNextClock ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	VstTimeInfo* info = vstPlugin->getTimeInfo(kVstClockValid);
	if ( info != NULL || (info->flags & kVstClockValid) )
		*vp = INT_TO_JSVAL(info->samplesToNextClock);
	else
		*vp = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



DEFINE_PROPERTY( inputLatency ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	*vp = JSVAL_TO_INT( vstPlugin->getInputLatency() );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY( outputLatency ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	*vp = JSVAL_TO_INT( vstPlugin->getOutputLatency() );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( initialDelay ) {

	J_S_ASSERT_INT( *vp );
	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	vstPlugin->setInitialDelay( JSVAL_TO_INT( *vp ) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( uniqueID ) {

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
	J_S_ASSERT_RESOURCE( vstPlugin );
	J_S_ASSERT_STRING( *vp );
	JSString *jsstr = JS_ValueToString(cx, *vp);
	J_S_ASSERT( JS_GetStringLength(jsstr) == 4, "Invalid ID length" );
	char *str = JS_GetStringBytes(jsstr);
	VstInt32 vstid = CCONST( str[0], str[1], str[2], str[3] );
	vstPlugin->setUniqueID( vstid );
	return JS_TRUE;
	JL_BAD;
}


// Send MIDI events back to Host application
DEFINE_FUNCTION_FAST( sendVstEventToHost ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( vstPlugin );

	JSObject *eventObj = JSVAL_TO_OBJECT( J_FARG(1) );

	bool res;

	if ( JS_InstanceOf(cx, eventObj, classMidiEvent, NULL) == JS_TRUE ) {

		VstMidiEvent *pv = (VstMidiEvent*)JS_GetPrivate(cx, eventObj);
		J_S_ASSERT_RESOURCE(pv);

		VstEvents events; // has already 2 allocated events ?
		events.numEvents = 1;
		events.events[1] = NULL;
		events.events[0] = (VstEvent*)pv;
		res = vstPlugin->sendVstEventsToHost(&events);
	}

	*J_FRVAL = res ? JSVAL_TRUE : JSVAL_FALSE;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_HAS_INSTANCE

	BEGIN_PROPERTY_SPEC

		PROPERTY_READ( hostLanguage )
		PROPERTY_READ( directory )

		PROPERTY_WRITE_STORE( numPrograms )
		PROPERTY_WRITE_STORE( numParams )
		PROPERTY_WRITE_STORE( numInputs )
		PROPERTY_WRITE_STORE( numOutputs )
		PROPERTY_WRITE_STORE( uniqueID )

		PROPERTY_WRITE_STORE( inputLatency )
		PROPERTY_WRITE_STORE( outputLatency )

		PROPERTY_READ( samplePos )
		PROPERTY_READ( sampleRate )
		PROPERTY_READ( nanoSeconds )
		PROPERTY_READ( ppqPos )
		PROPERTY_READ( tempo )
		PROPERTY_READ( barStartPos )
		PROPERTY_READ( cycleStartPos )
		PROPERTY_READ( cycleEndPos )
		PROPERTY_READ( timeSigNumerator )
		PROPERTY_READ( timeSigDenominator )
		PROPERTY_READ( smpteOffset )
		PROPERTY_READ( smpteFrameRate )
		PROPERTY_READ( samplesToNextClock )
		PROPERTY_WRITE_STORE( initialDelay )
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC( sendVstEventToHost, 1 )
	END_FUNCTION_SPEC

	BEGIN_CONST_INTEGER_SPEC

		// Language code returned by audioMasterGetLanguage / VstHostLanguage
		CONST_INTEGER_SINGLE( kVstLangEnglish )
		CONST_INTEGER_SINGLE( kVstLangGerman )
		CONST_INTEGER_SINGLE( kVstLangFrench )
		CONST_INTEGER_SINGLE( kVstLangItalian )
		CONST_INTEGER_SINGLE( kVstLangSpanish )
		CONST_INTEGER_SINGLE( kVstLangJapanese )

		// Plug-in Categories / VstPlugCategory
		CONST_INTEGER_SINGLE( kPlugCategUnknown )        // Unknown, category not implemented
		CONST_INTEGER_SINGLE( kPlugCategEffect )			 // Simple Effect
		CONST_INTEGER_SINGLE( kPlugCategSynth )			 // VST Instrument (Synths, samplers,...)
		CONST_INTEGER_SINGLE( kPlugCategAnalysis )		 // Scope, Tuner, ...
		CONST_INTEGER_SINGLE( kPlugCategMastering )		 // Dynamics, ...
		CONST_INTEGER_SINGLE( kPlugCategSpacializer )	 // Panners, ...
		CONST_INTEGER_SINGLE( kPlugCategRoomFx )			 // Delays and Reverbs
		CONST_INTEGER_SINGLE( kPlugSurroundFx )			 // Dedicated surround processor
		CONST_INTEGER_SINGLE( kPlugCategRestoration )	 // Denoiser, ...
		CONST_INTEGER_SINGLE( kPlugCategOfflineProcess ) // Offline Process	
		CONST_INTEGER_SINGLE( kPlugCategShell )			 // Plug-in is container of other plug-ins  @see effShellGetNextPlugin
		CONST_INTEGER_SINGLE( kPlugCategGenerator )		 // ToneGenerator, ...

		// Speaker Arrangement Types / VstSpeakerArrangementType
		CONST_INTEGER_SINGLE( kSpeakerArrUserDefined )     // user defined
		CONST_INTEGER_SINGLE( kSpeakerArrEmpty )				// empty arrangement
		CONST_INTEGER_SINGLE( kSpeakerArrMono )				// M
		CONST_INTEGER_SINGLE( kSpeakerArrStereo )				// L R
		CONST_INTEGER_SINGLE( kSpeakerArrStereoSurround )	// Ls Rs
		CONST_INTEGER_SINGLE( kSpeakerArrStereoCenter )		// Lc Rc
		CONST_INTEGER_SINGLE( kSpeakerArrStereoSide )		// Sl Sr
		CONST_INTEGER_SINGLE( kSpeakerArrStereoCLfe )		// C Lfe
		CONST_INTEGER_SINGLE( kSpeakerArr30Cine )				// L R C
		CONST_INTEGER_SINGLE( kSpeakerArr30Music )			// L R S
		CONST_INTEGER_SINGLE( kSpeakerArr31Cine )				// L R C Lfe
		CONST_INTEGER_SINGLE( kSpeakerArr31Music )			// L R Lfe S
		CONST_INTEGER_SINGLE( kSpeakerArr40Cine )				// L R C   S (LCRS)
		CONST_INTEGER_SINGLE( kSpeakerArr40Music )			// L R Ls  Rs (Quadro)
		CONST_INTEGER_SINGLE( kSpeakerArr41Cine )				// L R C   Lfe S (LCRS+Lfe)
		CONST_INTEGER_SINGLE( kSpeakerArr41Music )			// L R Lfe Ls Rs (Quadro+Lfe)
		CONST_INTEGER_SINGLE( kSpeakerArr50 )					// L R C Ls  Rs 
		CONST_INTEGER_SINGLE( kSpeakerArr51 )					// L R C Lfe Ls Rs
		CONST_INTEGER_SINGLE( kSpeakerArr60Cine )				// L R C   Ls  Rs Cs
		CONST_INTEGER_SINGLE( kSpeakerArr60Music )			// L R Ls  Rs  Sl Sr 
		CONST_INTEGER_SINGLE( kSpeakerArr61Cine )				// L R C   Lfe Ls Rs Cs
		CONST_INTEGER_SINGLE( kSpeakerArr61Music )			// L R Lfe Ls  Rs Sl Sr 
		CONST_INTEGER_SINGLE( kSpeakerArr70Cine )				// L R C Ls  Rs Lc Rc 
		CONST_INTEGER_SINGLE( kSpeakerArr70Music )			// L R C Ls  Rs Sl Sr
		CONST_INTEGER_SINGLE( kSpeakerArr71Cine )				// L R C Lfe Ls Rs Lc Rc
		CONST_INTEGER_SINGLE( kSpeakerArr71Music )			// L R C Lfe Ls Rs Sl Sr
		CONST_INTEGER_SINGLE( kSpeakerArr80Cine )				// L R C Ls  Rs Lc Rc Cs
		CONST_INTEGER_SINGLE( kSpeakerArr80Music )			// L R C Ls  Rs Cs Sl Sr
		CONST_INTEGER_SINGLE( kSpeakerArr81Cine )				// L R C Lfe Ls Rs Lc Rc Cs
		CONST_INTEGER_SINGLE( kSpeakerArr81Music )			// L R C Lfe Ls Rs Cs Sl Sr 
		CONST_INTEGER_SINGLE( kSpeakerArr102 )					// L R C Lfe Ls Rs Tfl Tfc Tfr Trl Trr Lfe2

		// Flags used in #VstPinProperties / VstPinPropertiesFlags
		CONST_INTEGER_SINGLE( kVstPinIsActive )   // pin is active, ignored by Host
		CONST_INTEGER_SINGLE( kVstPinIsStereo )   // pin is first of a stereo pair
		CONST_INTEGER_SINGLE( kVstPinUseSpeaker )	// #VstPinProperties::arrangementType is valid and can be used to get the wanted arrangement


		// DEBUG only ??
		// opcode names / AEffectOpcodes
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
	if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
		return 0;  // old version
	// Create the AudioEffect
	AudioEffect* effect = new JsVst(audioMaster);
	// Return the VST AEffect structur
	return effect->getAeffect();
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

// VST 2.4 specifications: http://asseca.com/vst-24-specs/index.html
