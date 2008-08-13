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

DECLARE_CLASS( MidiEvent );
DECLARE_CLASS( AudioMaster );
DECLARE_CLASS( VSTPlugin );

class JsException {
public:
	JsException( JSContext *cx, const char *text ) {
		
		JS_ReportError(cx, text);
	}
};

class JsvalHelper {
protected:
	virtual JSContext * JsContext() = 0;

	inline jsval GetProperty( JSObject *obj, const char *propertyName ) {
	
		jsval val;
		if ( !JS_GetProperty(JsContext(), obj, propertyName, &val) )
			throw JsException(JsContext(), "cannot get the property");
		return val;
	}

	inline void SetProperty( JSObject *obj, const char *propertyName, jsval val ) {
	
		if ( !JS_SetProperty(JsContext(), obj, propertyName, &val) )
			throw JsException(JsContext(), "cannot set the property");
	}

	inline int JsvalToInt( jsval val ) {
		
		int32 i;
		if ( !JS_ValueToInt32(JsContext(), val, &i) )
			throw JsException(JsContext(), "cannot convert to integer");
		return i;
	}

	inline jsval IntToJsval( int i ) {
	
		if ( INT_FITS_IN_JSVAL(i) )
			return INT_TO_JSVAL(i);
		jsval rval;
		if ( !JS_NewNumberValue(JsContext(), i, &rval) )
			throw JsException(JsContext(), "cannot convert integer to jsval");
		return rval;
	}

	inline double JsvalToDouble( jsval val ) {
		
		jsdouble d;
		if ( !JS_ValueToNumber(JsContext(), val, &d) )
			throw JsException(JsContext(), "cannot convert to double");
		if ( DOUBLE_TO_JSVAL(&d) == JS_GetNaNValue(JsContext()) ) // (TBD) needed ?
			throw JsException(JsContext(), "not a number");
		return d;
	}

	inline jsval DoubleToJsval( double d ) {
	
		jsval rval;
		if ( !JS_NewNumberValue(JsContext(), d, &rval) )
			throw JsException(JsContext(), "cannot convert double to jsval");
		if ( rval == JS_GetNaNValue(JsContext()) ) // (TBD) needed ?
			throw JsException(JsContext(), "not a number");
		return rval;
	}

	inline bool JsvalToBool( jsval val ) {
		
		JSBool b;
		if ( !JS_ValueToBoolean(JsContext(), val, &b) )
			throw JsException(JsContext(), "cannot convert to boolean");
		return b == JS_TRUE;
	}

	inline jsval BoolToJsval( bool b ) {
	
		return b ? JSVAL_TRUE : JSVAL_FALSE;
	}

	inline const char * JsvalToString( jsval val ) {
		
		JSString *jsstr = JS_ValueToString(JsContext(), val);
		if ( jsstr == NULL )
			throw JsException(JsContext(), "cannot convert to string");
		return JS_GetStringBytes(jsstr); // never fails
	}

	inline jsval StringToJsval( const char *str ) {

		JSString *jsstr = JS_NewStringCopyZ(JsContext(), str);
		if ( jsstr == NULL )
			throw JsException(JsContext(), "cannot create the string");
		return STRING_TO_JSVAL(jsstr);
	}

	inline jsval StringToJsval( const char *str, size_t length ) {

		JSString *jsstr = JS_NewStringCopyN(JsContext(), str, length);
		if ( jsstr == NULL )
			throw JsException(JsContext(), "cannot create the string");
		return STRING_TO_JSVAL(jsstr);
	}

	inline int CheckRange( int value, int min, int max ) {
		
		if ( value < min || value > max )
			throw JsException(JsContext(), "value out of range");
		return value;
	}
	
	inline bool InstanceOf( JSObject *obj, JSClass *jsClass ) {
		
		return JS_InstanceOf(JsContext(), obj, jsClass, NULL) == JS_TRUE;
	}

	inline bool InstanceOf( jsval val, JSClass *jsClass ) {

		if ( !JSVAL_IS_PRIMITIVE(val) )
			return false;
		return JS_InstanceOf(JsContext(), JSVAL_TO_OBJECT(val), jsClass, NULL) == JS_TRUE;
	}

	inline bool JsvalIsFunction( jsval val ) {
		
		return JS_TypeOfValue(JsContext(), val) == JSTYPE_FUNCTION;
	}

	inline bool JsvalIsString( jsval val ) {

		if ( JSVAL_IS_STRING(val) )
			return true;
		if ( !JSVAL_IS_PRIMITIVE(val) )
			return false;
		JSObject *stringPrototype;
		JS_GetClassObject(JsContext(), JS_GetGlobalObject(JsContext()), JSProto_String, &stringPrototype);
		if ( JS_GetPrototype(JsContext(), JSVAL_TO_OBJECT(val)) == stringPrototype )
			return true;
		return false;
	}

	inline jsval FunctionCall0( JSObject *obj, jsval fval ) {

		jsval rval;
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, 0, NULL, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall1( JSObject *obj, jsval fval, jsval arg1 ) {

		jsval rval;
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, 1, &arg1, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall2( JSObject *obj, jsval fval, jsval arg1, jsval arg2 ) {

		jsval rval, args[] = { arg1, arg2 };
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, 2, args, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall3( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3 ) {

		jsval rval, args[] = { arg1, arg2, arg3 };
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, sizeof(args)/sizeof(*args), args, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall4( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3, jsval arg4 ) {

		jsval rval, args[] = { arg1, arg2, arg3, arg4 };
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, sizeof(args)/sizeof(*args), args, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}

	inline jsval FunctionCall5( JSObject *obj, jsval fval, jsval arg1, jsval arg2, jsval arg3, jsval arg4, jsval arg5 ) {

		jsval rval, args[] = { arg1, arg2, arg3, arg4, arg5 };
		if ( !JS_CallFunctionValue(JsContext(), obj, fval, sizeof(args)/sizeof(*args), args, &rval) )
			throw JsException(JsContext(), "unable to call the function" );
		return rval;
	}
};





class JsVst : public AudioEffectX, public JsvalHelper {
private:

	JSContext *cx;
	JSObject *vstPlugin;
	jsval _arg, _rval;

	JSContext * JsContext() {
		
		return cx;
	}

	void ManageException() {
	
		if ( JS_IsExceptionPending(cx) ) {

			jsval fval, rval, ex;
			fval = GetProperty(vstPlugin, "catch");
			if ( JsvalIsFunction(fval) ) {

				JS_GetPendingException(cx, &ex);
				JS_CallFunctionValue(cx, vstPlugin, fval, 1, &ex, &rval);
				if ( JSVAL_IS_OBJECT(rval) )
					JS_SetPendingException(cx, rval);
				else
					JS_ClearPendingException(cx);
			}

			if ( JS_IsExceptionPending(cx) ) {
				
				JS_ReportPendingException(cx);
				JS_ClearPendingException(cx);
			}
		}
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

		JS_AddRoot(cx, &_rval);
		JS_AddRoot(cx, &_arg);

		InitializeClassMidiEvent(cx, JS_GetGlobalObject(cx));
		InitializeClassAudioMaster(cx, JS_GetGlobalObject(cx));
		InitializeClassVSTPlugin(cx, JS_GetGlobalObject(cx));

		JSObject *audioMasterObject = CreateAudioMasterObject(cx, audioMaster);
		_arg = OBJECT_TO_JSVAL(audioMasterObject);
		JS_SetProperty( cx, JS_GetGlobalObject(cx), "audioMaster", &_arg );

		vstPlugin = JS_DefineObject(cx, JS_GetGlobalObject(cx), "vstPlugin", classVSTPlugin, NULL, NULL);
		JS_SetPrivate(cx, vstPlugin, this);

		ExecuteScript(cx, "vstPlugin.js", false, 0, NULL, &_rval);

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

	// H->
	VstIntPtr dispatcher(VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt) {

		jsval fval = GetProperty(vstPlugin, "dispatcher");
		if ( JsvalIsFunction(fval) )
			FunctionCall4(vstPlugin, fval, IntToJsval(opcode), IntToJsval(index), IntToJsval(value), DoubleToJsval(opt) ); // , STRING_TO_JSVAL(JS_NewStringCopyZ(cx, ptr))
		return AudioEffectX::dispatcher(opcode, index, value, ptr, opt);
	}


	void open() {

		try {
			jsval fval = GetProperty(vstPlugin, "open");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::open();
	}

	void close() {

		try {
			jsval fval = GetProperty(vstPlugin, "close");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::close();
	}

	void suspend() {

		try {
			jsval fval = GetProperty(vstPlugin, "suspend");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::suspend();
	}

	void resume() {

		try {
			jsval fval = GetProperty(vstPlugin, "resume");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall0(vstPlugin, fval);
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::resume();
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

					JSObject *jsMidiEvent = JS_NewObject(cx, classMidiEvent, NULL, NULL);
					JS_SetPrivate(cx, jsMidiEvent, (VstMidiEvent*)events->events[i]);
					FunctionCall1(vstPlugin, jsProcessMidiEvent, OBJECT_TO_JSVAL(jsMidiEvent));
				} else
				if ( events->events[i]->type == kVstSysExType && !JSVAL_IS_VOID(jsSysExEvent) ) {

					// (TBD)
				}
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::processEvents(events);
	}

	// H->P Called when a parameter changed.
	void setParameter(VstInt32 index, float value) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "setParameter");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall2(vstPlugin, fval, IntToJsval(index), DoubleToJsval(value) );
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setParameter(index, value);
	}

	// H->P 
	float getParameter(VstInt32 index) {
		
		try {
			jsval fval = GetProperty(vstPlugin, "getParameter");
			if ( JsvalIsFunction(fval) )
				return JsvalToDouble(FunctionCall1(vstPlugin, fval, IntToJsval(index)));
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameter(index);
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
			if ( JsvalIsFunction(fval) ) {
				
				FunctionCall1(vstPlugin, fval, IntToJsval(program));
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setProgram(program);
	}

	// H->P Stuff the name field of the current program with name. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void setProgramName(char *name) {
	
		try {
			jsval fval = GetProperty(vstPlugin, "setProgramName");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall1(vstPlugin, fval, StringToJsval(name));
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setProgramName(name);
	}

	// H->P Stuff name with the name of the current program. Limited to kVstMaxProgNameLen. The program name is displayed in the rack, and can be edited by the user.
	void getProgramName(char *name) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProgramName");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall0(vstPlugin, fval);
				strncpy(name, JsvalToString(_rval), kVstMaxProgNameLen); // truncate
				name[kVstMaxProgNameLen-1] = '\0';
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProgramName(name);
	}

	// H->P Stuff label with the units in which parameter index is displayed.
	void getParameterLabel(VstInt32 index, char* label) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterLabel");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, IntToJsval(index));
				strncpy(label, JsvalToString(_rval), kVstMaxParamStrLen); // truncate
				label[kVstMaxParamStrLen-1] = '\0';
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterLabel(index, label);
	}

	// H->P Stuff text with a string representation ("0.5", "-3", "PLATE", etc...) of the value of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterDisplay(VstInt32 index, char *text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterDisplay");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, IntToJsval(index));
				strncpy(text, JsvalToString(_rval), kVstMaxParamStrLen); // truncate
				text[kVstMaxParamStrLen-1] = '\0';
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterDisplay(index, text);
	}

	// H->P Stuff text with the name ("Time", "Gain", "RoomType", etc...) of parameter index. Limited to kVstMaxParamStrLen. 
	void getParameterName(VstInt32 index, char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getParameterName");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, IntToJsval(index));
				strncpy(text, JsvalToString(_rval), kVstMaxParamStrLen); // truncate
				text[kVstMaxParamStrLen-1] = '\0';
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getParameterName(index, text);
	}


	// H->P
	//Reports what the plug-in is able to do (plugCanDos in audioeffectx.cpp).
	//Report what the plug-in is able to do. In general you can but don't have to report whatever you support 
	//or not support via canDo. Some application functionality may require some specific reply, 
	//but in that case you will probably know. Best is to report whatever you know for sure. 
	//A Host application cannot make assumptions about the presence of the new 2.x features of a plug-in. 
	//Ignoring this inquiry methods and trying to access a 2.x feature from a 1.0 plug, or vice versa, 
	//will mean the plug-in or Host application will break. 
	//It is not the end-users job to pick and choose which plug-ins can be supported by which Host.
	//Parameters: text 	A string from plugCanDos
	//Returns:
	//        * 0: don't know (default)
	//        * 1: yes
	//        * -1: no
	//Note:
	//    This should be supported. 
	VstInt32 canDo(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "canDo");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, StringToJsval(text));
				if ( JSVAL_IS_VOID(_rval) )
						return 0; // 0: don't know (default)
				return JsvalToBool(_rval) ? 1 : -1; // 1: yes, -1: no
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::canDo(text);
	}

	// H->P Fill text with name of program index (category deprecated in VST 2.4). Allows a Host application to list the plug-in's programs (presets).
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProgramNameIndexed");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, IntToJsval(index));
				if ( !JsvalIsString(_rval) ) // end of list
					return false;
				strncpy(text, JsvalToString(_rval), kVstMaxProgNameLen); // truncate
				text[kVstMaxProgNameLen-1] = '\0';
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProgramNameIndexed(category, index, text);
	}

	// H->P Specify a category that fits the plug (VstPlugCategory). 
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
	bool getProductString(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getProductString");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall0(vstPlugin, fval);
				strncpy(text, JsvalToString(_rval), kVstMaxProductStrLen); // truncate
				text[kVstMaxProductStrLen-1] = '\0';
				return true;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::getProductString(text);
	}

	// H->P Fill text with a string identifying the vendor.  text 	A string up to 64 chars 
	bool getVendorString(char* text) {

		try {
			jsval fval = GetProperty(vstPlugin, "getVendorString");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall0(vstPlugin, fval);
				strncpy(text, JsvalToString(_rval), kVstMaxVendorStrLen); // truncate
				text[kVstMaxVendorStrLen-1] = '\0';
				return true;
			}
		} catch( JsException ) { ManageException(); }
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
		
		try {
			jsval fval = GetProperty(vstPlugin, "setBypass");
			if ( JsvalIsFunction(fval) ) {

				_rval = FunctionCall1(vstPlugin, fval, BoolToJsval(onOff));
				return JsvalToBool(_rval);
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setBypass(onOff);
	}

	VstInt32 getMidiProgramName(VstInt32 channel, MidiProgramName* mpn) {

		try {
			jsval fval = GetProperty(vstPlugin, "getMidiProgramName");
			if ( JsvalIsFunction(fval) ) {

				JSObject *jsMpn = JS_NewObject(cx, NULL, NULL, NULL);
				_rval = FunctionCall3(vstPlugin, fval, IntToJsval(channel), IntToJsval(mpn->thisProgramIndex), OBJECT_TO_JSVAL(jsMpn));
				if ( !JSVAL_IS_INT(_rval) )
					throw JsException(cx, "invalid return value");
				strncpy(mpn->name, JsvalToString(GetProperty(jsMpn, "name")), kVstMaxNameLen);
				mpn->name[kVstMaxNameLen] = '\0';
				_arg = GetProperty(jsMpn, "midiProgram");
				mpn->midiProgram = JSVAL_IS_VOID(_arg) ? -1 : CheckRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "midiBankMsb");
				mpn->midiBankMsb = JSVAL_IS_VOID(_arg) ? -1 : CheckRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "midiBankLsb");
				mpn->midiBankLsb = JSVAL_IS_VOID(_arg) ? -1 : CheckRange(JsvalToInt(_arg), 0, 127);
				_arg = GetProperty(jsMpn, "parentCategoryIndex");
				mpn->parentCategoryIndex = JSVAL_IS_VOID(_arg) ? -1 : JsvalToInt(_arg);
				mpn->flags = JsvalToBool(GetProperty(jsMpn, "isOmny")) ? kMidiIsOmni : 0;
				return JsvalToInt(_rval);
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getMidiProgramName(channel, mpn);
	}


	void setSampleRate(float sampleRate) {

		try {
			jsval fval = GetProperty(vstPlugin, "setSampleRate");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall1(vstPlugin, fval, DoubleToJsval(sampleRate));
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setSampleRate(sampleRate);
	}


	void setBlockSize(VstInt32 blockSize) {
	
		try {
			jsval fval = GetProperty(vstPlugin, "setBlockSize");
			if ( JsvalIsFunction(fval) ) {

				FunctionCall1(vstPlugin, fval, IntToJsval(sampleRate));
				return;
			}
		} catch( JsException ) { ManageException(); }
		return AudioEffectX::setBlockSize(sampleRate);
	}


	bool getInputProperties(VstInt32 index, VstPinProperties* properties) {

		try {
			jsval fval = GetProperty(vstPlugin, "getInputProperties");
			if ( JsvalIsFunction(fval) ) {

				JSObject *tmpObj = JS_NewObject(cx, NULL, NULL, NULL);
				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(index), OBJECT_TO_JSVAL(tmpObj));

				strncpy(properties->label, JsvalToString(GetProperty(tmpObj, "label")), kVstMaxLabelLen);
				properties->label[kVstMaxLabelLen] = '\0';

				strncpy(properties->shortLabel, JsvalToString(GetProperty(tmpObj, "shortLabel")), kVstMaxShortLabelLen);
				properties->shortLabel[kVstMaxShortLabelLen] = '\0';

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

				JSObject *tmpObj = JS_NewObject(cx, NULL, NULL, NULL);
				_rval = FunctionCall2(vstPlugin, fval, IntToJsval(index), OBJECT_TO_JSVAL(tmpObj));

				strncpy(properties->label, JsvalToString(GetProperty(tmpObj, "label")), kVstMaxLabelLen);
				properties->label[kVstMaxLabelLen] = '\0';

				strncpy(properties->shortLabel, JsvalToString(GetProperty(tmpObj, "shortLabel")), kVstMaxShortLabelLen);
				properties->shortLabel[kVstMaxShortLabelLen] = '\0';

				properties->flags = JsvalToInt(GetProperty(tmpObj, "flags"));
				properties->arrangementType = JsvalToInt(GetProperty(tmpObj, "arrangementType"));
				
				return JsvalToBool(_rval);
			}
		} catch ( JsException ) { ManageException(); }
		return AudioEffectX::getOutputProperties(index, properties);
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

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_BOOLEAN( *vp );
		vstPlugin->canProcessReplacing( JSVAL_TO_BOOLEAN(*vp) == JS_TRUE ? true : false );
	}
	return JS_TRUE;
}

/* perhaps later
DEFINE_PROPERTY( canDoubleReplacing ) {

	if ( *vp != JSVAL_VOID ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_BOOLEAN( *vp );
		vstPlugin->canDoubleReplacing( JSVAL_TO_BOOLEAN(*vp) == JS_TRUE ? true : false );
	}
	return JS_TRUE;
}
*/

DEFINE_PROPERTY( numPrograms ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->SetNumPrograms( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numParams ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->SetNumParams( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numInputs ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->setNumInputs( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( numOutputs ) {

	if ( JSVAL_IS_VOID(*vp) ) {

		JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, obj);
		J_S_ASSERT_RESOURCE( vstPlugin );
		J_S_ASSERT_INT( *vp );
		vstPlugin->setNumOutputs( JSVAL_TO_INT(*vp) );
	}
	return JS_TRUE;
}


DEFINE_PROPERTY( uniqueID ) {

	if ( JSVAL_IS_VOID(*vp) ) {

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


DEFINE_FUNCTION_FAST( sendVstEventToHost ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_FARG(1) );

	JsVst *vstPlugin = (JsVst *)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE( vstPlugin );

	JSObject *eventObj = JSVAL_TO_OBJECT( J_FARG(1) );

	if ( JS_InstanceOf(cx, eventObj, classMidiEvent, NULL) == JS_TRUE ) {

		VstMidiEvent *pv = (VstMidiEvent*)JS_GetPrivate(cx, eventObj);
		J_S_ASSERT_RESOURCE(pv);

		VstEvents events;
		events.numEvents = 1;
		events.events[0] = (VstEvent*)pv;
		events.events[0]->type = kVstMidiType;
		events.events[0]->byteSize = sizeof(VstMidiEvent);
		vstPlugin->sendVstEventsToHost(&events);
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

		// Plug-in Categories
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

		// Speaker Arrangement Types
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

		// Flags used in #VstPinProperties
		CONST_INTEGER_SINGLE( kVstPinIsActive )   // pin is active, ignored by Host
		CONST_INTEGER_SINGLE( kVstPinIsStereo )   // pin is first of a stereo pair
		CONST_INTEGER_SINGLE( kVstPinUseSpeaker )	// #VstPinProperties::arrangementType is valid and can be used to get the wanted arrangement




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

