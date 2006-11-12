DECLARE_CLASS( Space );

inline JSBool ValToSpaceID( JSContext *cx, jsval val, ode::dSpaceID *spaceId ) {

	RT_ASSERT_OBJECT(val);
	JSObject *obj = JSVAL_TO_OBJECT(val);
	RT_ASSERT_CLASS(obj,&classSpace);
	*spaceId = (ode::dSpaceID)JS_GetPrivate(cx,obj);
	return JS_TRUE;
}
