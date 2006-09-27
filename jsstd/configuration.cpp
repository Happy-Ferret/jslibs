#include "stdafx.h"

#include "configuration.h"

JSClass configuration_class = { "Configuration", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};
/*
Configuration *CreateConfigurationObject( JSContext *cx ) {

	JSObject *global = JS_GetGlobalObject(cx);
	if ( global == NULL )
		return NULL;

	Configuration *configuration = (Configuration *)JS_malloc( cx, sizeof(Configuration) );
	if ( configuration == NULL )
		return NULL;
	
	JSObject *configObject = JS_ConstructObject( cx, &configuration_class, NULL, NULL );
	if ( configObject == NULL )
		return NULL;

	JS_SetPrivate( cx, configObject, configuration );

	jsval jsvalConfig = OBJECT_TO_JSVAL(configObject);
	JS_SetProperty( cx, global, "configuration", &jsvalConfig );

	return configuration;
}
*/

Configuration *GetConfiguration( JSContext *cx ) {

	JSObject *global = JS_GetGlobalObject(cx);
	if ( global == NULL )
		return NULL;

	jsval jsvalConfig;
	JS_GetProperty( cx, global, "configuration", &jsvalConfig );
	if ( jsvalConfig == JSVAL_VOID )
		return NULL;

	JSObject *configObject = JSVAL_TO_OBJECT(jsvalConfig);
	void *pv = JS_GetPrivate( cx, configObject );
	if ( pv == NULL )
		return NULL;

	return (Configuration *)pv;
}