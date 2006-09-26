#include <tomcrypt.h>

JSObject *prngInitClass( JSContext *cx, JSObject *obj );

struct PrngPrivate {
	ltc_prng_descriptor prng;
	prng_state state;
};

JSBool prng_call(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
void prng_Finalize(JSContext *cx, JSObject *obj);

JSClass prng_class = { "Prng", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, prng_Finalize,
	0,0, prng_call
};
