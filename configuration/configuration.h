#define XP_WIN
#include <jsapi.h>

#include <stdio.h>

#define CONFIGURATION_OBJECT_NAME "configuration"

extern JSClass configuration_class;

JSBool GetConfigurationObject(JSContext *cx, JSObject **configurationObject );
JSBool GetConfigurationValue( JSContext *cx, const char *name, jsval *value );


/*
struct Configuration {

	JSVersion jsVersion;
	bool reportWarnings;
	bool unsafeMode;
	int (*stdOut)(const char *data, int length);
	int (*stdErr)(const char *data, int length);
};


Configuration *GetConfiguration( JSContext *cx );
*/
