DECLARE_CLASS( Body )

#define BODY_SLOT_WORLD 0 // the world

inline JSBool ValToBodyID( JSContext *cx, jsval val, ode::dBodyID *bodyId ) {

	RT_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(obj,&classBody);
	*bodyId = (ode::dBodyID)JS_GetPrivate(cx,obj); // (TBD) ! manage null body ( environment connected; see world.body property )
	// *bodyId == NULL is not an error !
	// (TBD) use another way to detect if body is correct 
	return JS_TRUE;
}
