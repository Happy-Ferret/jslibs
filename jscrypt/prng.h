#ifndef PRNG_H_
#define PRNG_H_

#include <tomcrypt.h>

JSObject *prngInitClass( JSContext *cx, JSObject *obj );

struct PrngPrivate {
	ltc_prng_descriptor prng;
	prng_state state;
};

extern JSClass prng_class;

#endif // PRNG_H_