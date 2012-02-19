loadModule('jsdebug');


/// list all properties of the global object  [rt]

	propertiesList( global );


/// test OBJ_SCOPE in JS_PropertyIterator [rmtf]

//	PropertiesList([]); // see bug 688571 - JS_PropertyIterator is broken
//	PropertiesList({}); // ... bug 688571
	propertiesList({ a:123 });
	propertiesList(new String());
	propertiesList(propertiesList);


/// gcMallocBytes test [d]

	var v0 = gcMallocBytes;
	var s = stringRepeat('x', 100000);
	var v1 = gcMallocBytes;
	s = undefined;
	QA.GC();
	var v2 = gcMallocBytes;
	
	QA.ASSERT( v1-v0 > 100000 && v1-v0 < 301000, true, 'Before GC' );
	QA.ASSERT( v2 < 100, true, 'After GC');

