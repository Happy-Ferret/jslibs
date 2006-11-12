DECLARE_CLASS( Body )

#define BODY_SLOT_PARENT 0 // the world

inline JSBool ValToBodyID( JSContext *cx, jsval val, ode::dBodyID *bodyId ) {

	RT_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(obj,&classBody);
	*bodyId = (ode::dBodyID)JS_GetPrivate(cx,obj);
	return JS_TRUE;
}
