#define XP_WIN
#include <jsapi.h>

#include <stdio.h>

struct Configuration {

	JSVersion jsVersion;
	bool reportWarnings;
	bool unsafeMode;
	int (*stdOut)(const char *data, int length);
	int (*stdErr)(const char *data, int length);
};

extern JSClass configuration_class;

//Configuration *CreateConfigurationObject( JSContext *cx );
Configuration *GetConfiguration( JSContext *cx );

