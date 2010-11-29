LoadModule('jsdebug');


/// list all properties of the global object  [rt]

	PropertiesList( global );


/// test OBJ_SCOPE in JS_PropertyIterator [rmtf]

	PropertiesList({ a:123 });
	PropertiesList(new String());
	PropertiesList(PropertiesList);
	PropertiesList([]);


/// gcMallocBytes test [d]

	var v0 = gcMallocBytes;
	var s = StringRepeat('x', 100000);
	var v1 = gcMallocBytes;
	s = undefined;
	QA.GC();
	var v2 = gcMallocBytes;
	
	QA.ASSERT( v1-v0 > 100000 && v1-v0 < 301000, true, 'Before GC' );
	QA.ASSERT( v2 < 100, true, 'After GC');

