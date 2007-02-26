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
#include "configuration.h"

//JSClass configuration_class = { "Configuration", JSCLASS_HAS_PRIVATE,
//	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
//	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
//};

// returns JS_FALSE on fatal errors only
JSBool GetConfigurationObject(JSContext *cx, JSObject **configurationObject ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	if ( globalObject == NULL )
		return JS_FALSE;

	jsval configurationValue;
	JS_GetProperty(cx, globalObject, CONFIGURATION_OBJECT_NAME, &configurationValue);

	if ( configurationValue == JSVAL_VOID ) { // if configuration object do not exist, we build one

		*configurationObject = JS_DefineObject(cx, globalObject, CONFIGURATION_OBJECT_NAME, NULL, NULL, 0 );
		if ( *configurationObject == NULL ) // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
			return JS_FALSE; // cannot be created
	} else {

		RT_CHECK_CALL( JS_ValueToObject(cx, configurationValue, configurationObject) )
		// (TBD) check if it is the right object
	}
	return JS_TRUE;
}

/*
JSObject *GetConfigurationObject( JSContext *cx ) {

	JSObject *global = JS_GetGlobalObject(cx);
	if ( global == NULL )
		return NULL;
	jsval jsvalConfig;
	JS_GetProperty( cx, global, CONFIGURATION_OBJECT_NAME, &jsvalConfig );
	if ( jsvalConfig == JSVAL_VOID )
		return NULL;
	return JSVAL_TO_OBJECT(jsvalConfig);
}
*/

/*
jsval GetConfigurationValue( JSContext *cx, const char *name ) {

	JSObject *configurationObject = GetConfigurationObject(cx);
	if (configurationObject != NULL) {
		jsval value;
		JS_GetProperty(cx, configurationObject, name, &value);
		return value;
	}
	return JSVAL_VOID;
}
*/


// returns JS_FALSE on fatal errors only
JSBool GetConfigurationValue( JSContext *cx, const char *name, jsval *value ) {

	JSObject *configurationObject;
	RT_CHECK_CALL( GetConfigurationObject(cx, &configurationObject) )
	if (configurationObject == NULL)
		return JS_FALSE;
	RT_CHECK_CALL( JS_GetProperty(cx, configurationObject, name, value) )
	return JS_TRUE;
}


/*
void SetConfigurationValue( JSContext *cx, const char *name, jsval value ) {

	JSObject *configurationObject = GetConfigurationObject(cx);
	if (configurationObject == NULL)
		return JSVAL_VOID;
	jsval value;
	JS_GetProperty(cx, configurationObject, name, &value);
}
*/

/*
Configuration *GetConfiguration( JSContext *cx ) {

	JSObject *configObject = GetConfigurationObject(cx);

	void *pv = JS_GetPrivate( cx, configObject );
	if ( pv == NULL )
		return NULL;

	return (Configuration *)pv;
}
*/