({

	StringMemUsage: function(QA) {
		
		CollectGarbage();
		var mem0 = privateMemoryUsage;

		var str1 = StrSet('x', 1000000);
		var str2 = StrSet('x', 1000000);
		StrSet('x', 1000000);
		CollectGarbage();
		
		Print( privateMemoryUsage - mem0, '\n\n' );
	}
})
