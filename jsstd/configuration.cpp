#include "stdafx.h"

#include "configuration.h"

JSClass configuration_class = { "Configuration", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub
};

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