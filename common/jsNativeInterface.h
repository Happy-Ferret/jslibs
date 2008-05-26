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

#ifndef _JSNATIVEINTERFACE_H_
#define _JSNATIVEINTERFACE_H_

///////////////////////////////////////////////////////////////////////////////
// Native Interface API

typedef void (*FunctionPointer)(void);

inline JSBool HasNativeInterface( JSContext *cx, JSObject *obj, const char *name, bool *has ) {

	if ( obj == NULL )
		return JS_FALSE;
	JSBool found;
	if ( JS_HasProperty(cx, obj, name, &found) != JS_TRUE )
		return JS_FALSE;
	*has = found == JS_TRUE;
	return JS_TRUE;
}

inline JSBool SetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer function ) {

	// Cannot be called while Finalize
	// the following must works because spidermonkey will never call the getter or setter if it is not explicitly required by the script
	if ( function != NULL )
		return JS_DefineProperty(cx, obj, name, JSVAL_VOID, (JSPropertyOp)function, (JSPropertyOp)NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_DeleteProperty(cx, obj, name);
}


inline JSBool GetNativeInterface( JSContext *cx, JSObject *obj, const char *name, FunctionPointer *function ) {

	// Cannot be called while Finalize
	if ( obj != NULL ) {

		uintN attrs;
		JSBool found;
		JSPropertyOp tmp = NULL; //descriptor;
		if ( JS_GetPropertyAttrsGetterAndSetter(cx, obj, name, &attrs, &found, (JSPropertyOp*)function, &tmp) != JS_TRUE )
			return JS_FALSE;
		if ( found )
			return JS_TRUE;
	}
	*function = NULL;
	return JS_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Interfaces

typedef JSBool (*NIStreamRead)( JSContext *cx, JSObject *obj, char *buf, unsigned int *amount );

inline JSBool HasStreamReadInterface( JSContext *cx, JSObject *obj, bool *has ) {

	return HasNativeInterface(cx, obj, "_NISR", has);
}

inline JSBool SetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead fct ) {

	return SetNativeInterface(cx, obj, "_NISR", (FunctionPointer)fct);
}

inline JSBool GetStreamReadInterface( JSContext *cx, JSObject *obj, NIStreamRead *fct ) {
	
	return GetNativeInterface(cx, obj, "_NISR", (FunctionPointer*)fct);
}


// **pm
//   in: a valid float[16]
//  out: pointer provided as input OR another pointer to float
typedef JSBool (*NIMatrix44Read)( JSContext *cx, JSObject *obj, float **pm ); // **pm allows NIMatrix44Read to return its own data pointer ( should be const )

inline JSBool HasMatrix44ReadInterface( JSContext *cx, JSObject *obj, bool *has ) {
	
	return HasNativeInterface(cx, obj, "_NIRM", has);
}

inline JSBool SetMatrix44ReadInterface( JSContext *cx, JSObject *obj, NIMatrix44Read fct ) {

	return SetNativeInterface(cx, obj, "_NIRM", (FunctionPointer)fct);
}

inline JSBool GetMatrix44ReadInterface( JSContext *cx, JSObject *obj, NIMatrix44Read *fct ) {
	
	return GetNativeInterface(cx, obj, "_NIRM", (FunctionPointer*)fct);
}



// buffer access interface
typedef JSBool (*NIBufferRead)( JSContext *cx, JSObject *obj, const char **buf, size_t *size );

inline JSBool HasBufferReadInterface( JSContext *cx, JSObject *obj, bool *has ) {

	return HasNativeInterface(cx, obj, "_NIBR", has);
}

inline JSBool SetBufferReadInterface( JSContext *cx, JSObject *obj, NIBufferRead fct ) {
		
	return SetNativeInterface(cx, obj, "_NIBR", (FunctionPointer)fct);
}

inline JSBool GetBufferReadInterface( JSContext *cx, JSObject *obj, NIBufferRead *fct ) {
	
	return GetNativeInterface(cx, obj, "_NIBR", (FunctionPointer*)fct);
}


#endif // _JSNATIVEINTERFACE_H_
