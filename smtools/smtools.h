JSBool IntArrayToVector( JSContext *cx, int count, const jsval *vp, int *vector );
JSBool IntVectorToArray( JSContext *cx, int count, const int *vector, jsval *vp );

JSBool FloatArrayToVector( JSContext *cx, int count, const jsval *vp, float *vector );
JSBool FloatVectorToArray( JSContext *cx, int count, const float *vector, jsval *vp );
