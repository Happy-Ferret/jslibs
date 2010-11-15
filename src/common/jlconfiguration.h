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

#ifndef _JLCONFIGURATION_H_
#define _JLCONFIGURATION_H_

#include "jlhelper.h"


ALWAYS_INLINE JSBool RemoveConfiguration(JSContext *cx) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	JL_S_ASSERT( globalObject != NULL, "Unable to find the global object." );
	return JS_DeletePropertyById(cx, globalObject, JLID(cx, _configuration));
	JL_BAD;
}


inline JSObject *GetConfigurationObject(JSContext *cx) {

	JSObject *cobj, *globalObject = JS_GetGlobalObject(cx);
	JL_CHK( globalObject );
	jsval configurationValue;
//	JL_CHK( JS_GetProperty(cx, globalObject, NAME_CONFIGURATION_OBJECT, &configurationValue) );
//	jsid configurationId = JL_GetPrivateJsid(cx, JL_GetHostPrivate(cx), NAME_CONFIGURATION_OBJECT, PRIVATE_JSID__configuration);
	jsid configurationId;
	configurationId = JLID(cx, _configuration);
	JL_CHK( configurationId != JL_NullJsid() );
	JL_CHK( JS_GetPropertyById(cx, globalObject, configurationId, &configurationValue) );

	if ( JSVAL_IS_VOID( configurationValue ) ) { // if configuration object do not exist, we build one

		cobj = JS_DefineObject(cx, globalObject, JLID_NAME(cx, _configuration), NULL, NULL, 0 );
		JL_CHK( cobj ); // Doc: If the property already exists, or cannot be created, JS_DefineObject returns NULL.
	} else {
		JL_CHK( JSVAL_IS_OBJECT(configurationValue) );
		cobj = JSVAL_TO_OBJECT( configurationValue );
	}
	return cobj;
bad:
	return NULL;
}


ALWAYS_INLINE JSBool GetConfigurationValueById(JSContext *cx, jsid id, jsval *value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_GetPropertyById(cx, cobj, id, value);
	*value = JSVAL_VOID;
	return JS_TRUE;
}

ALWAYS_INLINE JSBool GetConfigurationValue(JSContext *cx, const char *name, jsval *value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_GetProperty(cx, cobj, name, value);
	*value = JSVAL_VOID;
	return JS_TRUE;
}

ALWAYS_INLINE JSBool SetConfigurationValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_ENUMERATE);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool SetConfigurationReadonlyValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

ALWAYS_INLINE JSBool SetConfigurationPrivateValue(JSContext *cx, const char *name, jsval value) {

	JSObject *cobj = GetConfigurationObject(cx);
	if ( cobj )
		return JS_DefineProperty(cx, cobj, name, value, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

#endif // _JLCONFIGURATION_H_
