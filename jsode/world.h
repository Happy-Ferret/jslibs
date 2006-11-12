DECLARE_CLASS( World )

inline JSBool ValToWorldID( JSContext *cx, jsval val, ode::dWorldID *worldId ) {
	
	RT_ASSERT_OBJECT(val);
	JSObject *worldObject = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(worldObject,&classWorld);
	*worldId = (ode::dWorldID)JS_GetPrivate(cx,worldObject);
	return JS_TRUE;
}

