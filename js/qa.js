({

	E4X: function(QA) {

		var sales = <sales vendor="John">
			 <item type="peas" price="4" quantity="6"/>
			 <item type="carrot" price="3" quantity="10"/>
			 <item type="chips" price="5" quantity="3"/>
		  </sales>;
		QA.ASSERT( ''+sales.item.(@type == "carrot").@quantity, '10', 'data access' );
		QA.ASSERT( ''+sales.@vendor, 'John', 'data access' );
	},
	
	
	DestructuringAssignmentShorthand: function(QA) {
		
		var { a, b, c } = { a:1, b:2, c:3 }
		QA.ASSERT( a, 1, 'destructuring assignment shorthand' );
		QA.ASSERT( c, 3, 'destructuring assignment shorthand' );
	},
	
	
	ExpressionsClosures: function(QA) {
	
		var f = function() 123;
		QA.ASSERT( f(), 123, 'expression closures' );
	},
	
	
	GeneratorExpressions: function(QA) {
	
		var res = [ a for each ( a in [5,6,7,8] ) if (a > 6) ];
		QA.ASSERT( res.join(','), '7,8', 'Generator Expressions' );
	},

	BlockScope: function(QA) {
	
		var a = 5;
		let ( a = 6 ) {
		
			QA.ASSERT( a, 6, 'inside let block' );
		}
		QA.ASSERT( a, 5, 'outside let block' );
		
	},

})