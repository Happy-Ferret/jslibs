#define XP_WIN
#include <jsapi.h>
#include <jsvalue.h>
#include <string.h>

JSClass global_class = {
	 "global", JSCLASS_GLOBAL_FLAGS, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
};

int count = 0;

static JSBool next(JSContext *cx, uintN argc, jsval *vp) {

	JSObject *obj = JS_THIS_OBJECT(cx, vp);
	if ( ++count == 5 )
		return JS_ThrowStopIteration(cx);
	return JS_TRUE;
}

static JSObject* IteratorObject(JSContext *cx, JSObject *obj, JSBool keysonly) {

	JSObject *itObj = JS_NewObject(cx, NULL, NULL, NULL);
	JS_DefineFunction(cx, itObj, "next", next, 0, 0);
	count = 0;
	return itObj;
}

JSBool constructor( JSContext *cx, uintN argc, jsval *vp ) {

	JSObject *obj;
	obj = JS_NewObjectForConstructor(cx, vp);
	if ( obj == NULL )
		return JS_FALSE;
	JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
	return JS_TRUE;
}

js::Class jl_BlobClass = {
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
		IteratorObject,
		NULL
	}
};


template <class T, int S = 10>
class Allocator {
public:
	T* Alloc() {
		
		return (T*)malloc(sizeof(T)*S);
	}
};


template <class T>
class AllocatorWrapper : public Allocator<T> {
};



template <class T, template<class> class A>
class Stack {
public:
	struct Item {
		T data;
		Item *next;
	};

	A<Item> allocator;

	void Push(T data) {

		Item *item = allocator.Alloc();
		item->data = data;
	}
};



int main(int argc, char* argv[]) {

	Stack<int, Allocator<10>> s;
	s.Push(123);

	return EXIT_SUCCESS;
}
