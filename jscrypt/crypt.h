#ifndef CRYPT_H_
#define CRYPT_H_


JSObject *cryptInitClass( JSContext *cx, JSObject *obj );

#define MODE_CTR "CTR"
#define MODE_CFB "CFB"

enum CryptMode {
	mode_ctr,
	mode_cfb
};

struct CryptPrivate {

	CryptMode mode;
	void *symmetric_XXX;
};


struct crypt_class;

#endif // CRYPT_H_