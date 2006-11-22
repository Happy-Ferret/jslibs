DECLARE_CLASS( World )

#define DEFAULT_SURFACE_PARAMETERS_PROPERTY_NAME "defaultSurfaceParameters"
#define WORLD_SPACE_PROPERTY_NAME "space"


//#define WORLD_SLOT_CONTACTGROUP 0
#define WORLD_SLOT_SPACE 1

inline JSBool ValToWorldID( JSContext *cx, jsval val, ode::dWorldID *worldId ) {
	
	RT_ASSERT_OBJECT(val);
	JSObject *worldObject = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	*worldId = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	return JS_TRUE;
}

