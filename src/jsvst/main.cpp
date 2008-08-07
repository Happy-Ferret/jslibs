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

extern void* hInstance;

DECLARE_CLASS( AudioEffect );


class JsVst : public AudioEffectX {
private:



public:

	JSContext *cx;
	JSObject *jsAudioEffect;

	~JsVst() {

		DestroyHost(cx);
	}

	JsVst( audioMasterCallback audioMaster ) : AudioEffectX (audioMaster, 1, 1) { 	// 1 program, 1 parameter only
	
		cx = CreateHost(-1, -1);
		InitHost(cx, true, NULL, NULL);
		InitializeClassAudioEffect(cx, JS_GetGlobalObject(cx));

		jsAudioEffect = JS_NewObject(cx, classAudioEffect, NULL, NULL);
		JS_SetPrivate(cx, jsAudioEffect, this);

		//JSBool ExecuteScript( JSContext *cx, const char *scriptFileName, bool compileOnly, int argc, const char * const * argv, jsval *rval );
	}

/*
	void setParameter (long index, float value) {
	}

	float getParameter (long index) {
	}
*/

	void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames) {

		float *in1 = inputs[0];
		float *in2 = inputs[1];
		float *out1 = outputs[0]; 
		float *out2 = outputs[1]; 
		while (--sampleFrames >= 0) { 

			(*out1++) = (*in1++); 
			(*out2++) = (*in2++); 
		} 
	}

	VstInt32 getProgram () {
		
//		MaybeReloadScript(cx);

		jsval rval;
		JSBool res = JL_CallFunctionName(cx, jsAudioEffect, "getProgram", &rval, 0);
		if ( res && JSVAL_IS_INT(rval) ) {
		
			return JSVAL_TO_INT(rval);
		}

		if ( JS_IsExceptionPending(cx) ) { // let the script manage its errors.

			jsval ex;
			JS_GetPendingException(cx, &ex);
			JL_CallFunctionName(cx, jsAudioEffect, "onException", &ex, 0);
		}

		
		
/*
		jsval fctVal;
		JS_GetProperty(cx, jsAudioEffect, "getProgram", &fctVal);

		if ( JsvalIsFunction(cx, fctVal) ) {

			jsval rval;
			JS_CallFunctionValue(cx, jsAudioEffect, fctVal, 0, NULL, &rval);
			
			if ( !JS_IsExceptionPending(cx) ) {

				if ( JSVAL_IS_INT(rval) ) {
					
					J_REPORT_WARNING( "Invalid value type." );
					// report an error
					return JSVAL_TO_INT(rval);
				}
			}
		}

//		CallFunction(cx, jsAudioEffect, fctVal, &rval, 0);
*/
		return AudioEffectX::getProgram();
	}

	/*
	void setProgram (long program) {
	}

	void setProgramName (char *name) {
	}

	void getProgramName (char *name) {
	}

	void getParameterLabel(long index, char *label) {
	}

	void getParameterDisplay (long index, char *text) {
	}

	void getParameterName (long index, char *text) {
	}
*/
// vst2.x
/*
	VstInt32 canDo (char* text) {
	}

	bool getProgramNameIndexed (long category, long index, char *text) {
	}

	VstPlugCategory getPlugCategory () {
	}

	bool getProductString (char* text) {
	}

	bool getVendorString (char* text) {
	}

	bool setBypass (bool onOff) {
	}

	bool string2parameter (long index, char *text) {
	}
*/
};

AudioEffect* createEffectInstance (audioMasterCallback audioMaster) {

	return new JsVst(audioMaster);
}



/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( AudioEffect )

DEFINE_HAS_INSTANCE() {

	*bp = !JSVAL_IS_PRIMITIVE(v) && OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v)) == _class;
	return JS_TRUE;
}


CONFIGURE_CLASS

	HAS_PRIVATE
	HAS_HAS_INSTANCE

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	BEGIN_FUNCTION_SPEC
	END_FUNCTION_SPEC

END_CLASS

