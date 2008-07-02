({

	StringMemUsage: function(QA) {
		
		QA.GC();
		var mem0 = privateMemoryUsage;

		var str1 = StringRepeat('x', 1000000);
		StringRepeat('x', 1000000);
		QA.GC();
		
		var diff = privateMemoryUsage - mem0;
		QA.ASSERT( diff > 3 * 1000000 && diff < 3.1 * 1000000, true, 'check privateMemoryUsage value' );
	}
})
