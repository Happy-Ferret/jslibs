inline ode::dReal JSValToODEReal( JSContext *cx, jsval val ) {

	jsdouble value;
	if ( JS_ValueToNumber(cx, val, &value) == JS_FALSE ) // (TBD) manage errors
		return 0;
	if ( value > dInfinity )
		return dInfinity;
	if ( value < -dInfinity )
		return -dInfinity;
	return value;
}