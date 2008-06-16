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

#include "../common/jsNames.h"
#include "../common/jsHelper.h"

inline JSBool RemoveConfiguration(JSContext *cx) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	J_S_ASSERT( globalObject != NULL, "Unable to find the global object." );
	return JS_DeleteProperty(cx, globalObject, NAME_CONFIGURATION_OBJECT);
}


inline JSObject *GetConfigurationObject(JSContext *cx) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JSObject *configurationObject;
	if ( globalObject == NULL )
		return NULL;
	jsval configurationValue;
	JSBool status;
	status = JS_GetProperty(cx, globalObject, NAME_CONFIGURATION_OBJECT, &configurationValue);
	if ( status != JS_TRUE )
		return NULL;
	if ( configurationValue == JSVAL_VOID ) { // if configuration object do not exist, we build one

		configurationObject = JS_DefineObject(cx, globalObject, NAME_CONFIGURATION_OBJECT, NULL, NULL, 0 );
		if ( configurationObject == NULL ) // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
			return NULL; // cannot be created
	} else {

		if ( !JSVAL_IS_OBJECT(configurationValue) )
			return NULL;
		configurationObject = JSVAL_TO_OBJECT( configurationValue );
	}
	return configurationObject;
}


// returns JSVAL_VOID on errors
inline JSBool GetConfigurationValue(JSContext *cx, const char *name, jsval *value) {

	JSObject *configurationObject = GetConfigurationObject(cx);
	if (configurationObject == NULL) {
		
		*value = JSVAL_VOID;
		return JS_TRUE;
	}
	J_CHK( JS_GetProperty(cx, configurationObject, name, value) );
	return JS_TRUE;
}

inline JSBool SetConfigurationPrivateValue(JSContext *cx, const char *name, jsval value) {

	JSObject *configObject = GetConfigurationObject(cx);
	J_S_ASSERT( configObject != NULL, "Unable to get configuration object" );
	J_CHKM1( JS_DefineProperty(cx, configObject, name, value, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT ), "Unable to store %1 configuration item.", name );
	return JS_TRUE;
}

inline JSBool SetConfigurationValue(JSContext *cx, const char *name, jsval value) {

	JSObject *configObject = GetConfigurationObject(cx);
	J_S_ASSERT( configObject != NULL, "Unable to get configuration object" );
	J_CHKM1( JS_DefineProperty(cx, configObject, name, value, NULL, NULL, JSPROP_ENUMERATE), "Unable to store %1 configuration item.", name );
	return JS_TRUE;
}

#endif // _JSCONFIGURATION_H_
