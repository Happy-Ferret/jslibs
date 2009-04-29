LoadModule('jsdebug');

/// String memory usage [tr]

		QA.GC();
		var mem0 = privateMemoryUsage;

		var str1 = StringRepeat('x', 1000000);
		var str2 = StringRepeat('x', 1000000);
		str2 = undefined;
		QA.GC();

		var mem1 = privateMemoryUsage;
		var diff = mem1 - mem0;
		QA.ASSERT( diff > 3 * 1000000 && diff < 3.1 * 1000000, true, 'check privateMemoryUsage value ('+mem1+' - '+mem0+')' );

	/* try to understant this:
	- src/jsdebug/jsdebug_qa.js:15 check privateMemoryUsage value (10129408 - 5324800), false != true
	- src/jsdebug/jsdebug_qa.js:15 check privateMemoryUsage value (10133504 - 10145792), false != true
	- src/jsdebug/jsdebug_qa.js:15 check privateMemoryUsage value (11202560 - 13312000), false != true
	- src/jsdebug/jsdebug_qa.js:15 check privateMemoryUsage value (13131776 - 10199040), false != true
	*/

