#ifndef HASH_H_
#define HASH_H_

#include <tomcrypt.h>

JSObject *hashInitClass( JSContext *cx, JSObject *obj );

struct HashPrivate {
	ltc_hash_descriptor hash;
	hash_state state;
};

extern JSClass hash_class;

#endif // HASH_H_