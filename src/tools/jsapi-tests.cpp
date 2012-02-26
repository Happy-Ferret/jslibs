#include "malloc.h"

#include "jsapi.h"
//#include "jsscript.h"
//#include "jscntxt.h"
//#include "jsarena.h"
//#include "jsemit.h"

JS_BEGIN_EXTERN_C
void* (*custom_malloc)( size_t ) = malloc;
void* (*custom_calloc)( size_t, size_t ) = calloc;
void* (*custom_realloc)( void*, size_t ) = realloc;
void (*custom_free)( void* ) = free;
JS_END_EXTERN_C


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "tests.h"

int count = 0;

static JSBool soubokTest_next(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	if ( ++count == 5 )
		return JS_ThrowStopIteration(cx);
	return JS_TRUE;
}

static JSObject* soubokTest_IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly) {

	JSObject *itObj = JS_NewObject(cx, NULL, NULL, NULL);
	JS_DefineFunction(cx, itObj, "next", soubokTest_next, 0, 0);
	count = 0;
	return itObj;
}

JSBool soubokTest_constructor( JSContext *cx, uintN argc, jsval *vp ) {

	JSObject *obj;
	obj = JS_NewObjectForConstructor(cx, vp);
	if ( obj == NULL )
		return JS_FALSE;
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

js::Class soubokTest_BlobClass = {
    "Blob",
	0,
	js::PropertyStub,   /* addProperty */
	js::PropertyStub,   /* delProperty */
	js::PropertyStub,   /* getProperty */
	js::PropertyStub,   /* setProperty */
	js::EnumerateStub,
	js::ResolveStub,
	js::ConvertStub,
    NULL,
    NULL,           /* reserved0   */
    NULL,           /* checkAccess */
    NULL,           /* call        */
    NULL,           /* construct   */
    NULL,           /* xdrObject   */
    NULL,           /* hasInstance */
    NULL,           /* mark        */
    {
		NULL,
		NULL,
		NULL,
		soubokTest_IteratorObject,
		NULL
	}
};

/*
BEGIN_TEST(soubokTest)
{
//	CHECK( JS_InitClass(cx, global, NULL, js::Jsvalify(&soubokTest_BlobClass), soubokTest_constructor, 0, NULL, NULL, NULL, NULL) );
//	EXEC("for ( var i = 0; i < 2; i++ ) [ 0 for ( it in Blob() ) ]");
	return true;
}
END_TEST(soubokTest)
*/


BEGIN_TEST(soubokTest2)
{
	jschar str[] = L"ABCD";
	JSString *jsstr = JL_NewUCString(cx, str, 2);


	return true;
}
END_TEST(soubokTest2)
