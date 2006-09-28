#ifndef RSA_H_
#define RSA_H_

#include <tomcrypt.h>

JSObject *rsaInitClass( JSContext *cx, JSObject *obj );

struct RsaPrivate {
	rsa_key key;
};

extern JSClass rsa_class;

#endif // RSA_H_