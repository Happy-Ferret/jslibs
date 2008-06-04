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

	ArrayIndexOf: function(QA) {
		
		QA.ASSERT( ['a','b','c','d'].indexOf('c'), 2, 'array indexof' );
	},

	EmptyObject: function(QA) {
	
		var o = { __proto__:null };
		QA.ASSERT( o instanceof Object, false, 'no property' );
		QA.ASSERT( 'constructor' in o, false, 'no property' );
		QA.ASSERT( '__proto__' in o, false, 'no property' );
	},
	
	Generator: function(QA) {

		var success;

		var gen = new function() {

			var ret = yield 3;
			QA.ASSERT( ret, 4, 'sent value' );
			success = true;
		}

		try {

			var val = gen.next();
			QA.ASSERT( val, 3, 'generator value' );
			gen.send(4);
		} catch(ex if ex instanceof StopIteration){

			QA.ASSERT( success, true, 'generator end' );
			return;
		}
		QA.FAILED( 'catching StopIteration' );
	},
	
	ScriptObject: function(QA) {
		
		if ( typeof Script == 'undefined' ) {

			QA.FAILED('"Script" object not found');
		} else {
	
			var x = 1;
			(new Script("x++")).exec();
			QA.ASSERT( x, 2, 'script exec' );
		}
	},
	
	SharpVars: function(QA) {
	
		var a = { titi:#1={}, toto:#1# };
		QA.ASSERT( a.titi, a.toto, 'reference' );

		var a = { b:#1={ c:#1# } }
		QA.ASSERT( 'c' in a.b.c.c.c.c.c.c, true, 'nested' );
	},
	
	Uneval: function(QA) {

		var src = uneval({ abc : 123 });
		QA.ASSERT( src, '({abc:123})', 'uneval an object' );
	},

	Freeze: function(QA) {

		if ( typeof Script == 'undefined' ) {

			QA.FAILED('"Script" object not found');
		} else {
		
			var s = new Script('{ a : 1 }');
			QA.ASSERT( typeof s.freeze, 'function', 'has freeze function' );
			if ( s.freeze )
				s.freeze();
		}
	}


})