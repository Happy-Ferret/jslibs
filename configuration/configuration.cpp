#include "stdafx.h"

#include "configuration.h"

JSClass configuration_class = { "Configuration", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};


JSObject *GetConfigurationObject( JSContext *cx ) {

	JSObject *globalObject = JS_GetGlobalObject(cx);
	if ( globalObject == NULL )
		return NULL;
	jsval configurationValue;
	JS_GetProperty( cx, globalObject, CONFIGURATION_OBJECT_NAME, &configurationValue );

	JSObject *configObject;
	if ( configurationValue == JSVAL_VOID ) {

		configObject = JS_NewObject( cx, &configuration_class, NULL, NULL );
		if ( configObject == NULL )
			return NULL;
		configurationValue = OBJECT_TO_JSVAL(configObject);
		JS_SetProperty( cx, globalObject, CONFIGURATION_OBJECT_NAME, &configurationValue );
	} else {
		configObject = JSVAL_TO_OBJECT(configurationValue);
	}
	return globalObject;
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

jsval GetConfigurationValue( JSContext *cx, const char *name ) {

	JSObject *configurationObject = GetConfigurationObject(cx);
	if (configurationObject == NULL)
		return JSVAL_VOID;
	jsval value;
	JS_GetProperty(cx, configurationObject, name, &value);
	return value;
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