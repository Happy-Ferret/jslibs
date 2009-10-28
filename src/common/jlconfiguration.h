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

#ifndef _JSCONFIGURATION_H_
#define _JSCONFIGURATION_H_

#include "jlnames.h"
#include "jlhelper.h"


inline JSBool RemoveConfiguration(JSContext *cx) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_S_ASSERT( globalObject != NULL, "Unable to find the global object." );
	return JS_DeleteProperty(cx, globalObject, NAME_CONFIGURATION_OBJECT);
	JL_BAD;
}


inline JSObject *GetConfigurationObject(JSContext *cx) {

	JSObject *cobj, *globalObject = JS_GetGlobalObject(cx);
	JL_CHK( globalObject );
	jsval configurationValue;
	JL_CHK( JS_GetProperty(cx, globalObject, NAME_CONFIGURATION_OBJECT, &configurationValue) );
	if ( JSVAL_IS_VOID( configurationValue ) ) { // if configuration object do not exist, we build one

		cobj = JS_DefineObject(cx, globalObject, NAME_CONFIGURATION_OBJECT, NULL, NULL, 0 );
		JL_CHK( cobj ); // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
	} else {
		JL_CHK( JSVAL_IS_OBJECT(configurationValue) );
		cobj = JSVAL_TO_OBJECT( configurationValue );
	}
	return cobj;
bad:
	return NULL;
}


inline JSBool GetConfigurationValue(JSContext *cx, const char *name, jsval *value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_GetProperty(cx, cobj, name, value);
	*value = JSVAL_VOID;
	return JS_TRUE;
}

inline JSBool SetConfigurationValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_ENUMERATE);
	return JS_TRUE;
}

inline JSBool SetConfigurationReadonlyValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

inline JSBool SetConfigurationPrivateValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

#endif // _JSCONFIGURATION_H_
