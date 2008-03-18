function( QA ) ({

	GlobalVariables: function() {
		
		QA.ASSERT( scripthostpath != '', true, 'script host path' );
		QA.ASSERT( scripthostname.substr(0, 6), 'jshost', 'script host name' );
		QA.ASSERT( typeof _configuration, 'object', 'has configuration object' );
		QA.ASSERT( typeof global, 'object', 'has global object' );
		var mod = LoadModule('jsstd');
		var mod1 = LoadModule('jsstd');
		QA.ASSERT( mod, mod1, 'LoadModule' );
		QA.ASSERT( _configuration.unsafeMode, false, 'unsafe mode' );
	},

	E4X: function() {

		var sales = <sales vendor="John">
			 <item type="peas" price="4" quantity="6"/>
			 <item type="carrot" price="3" quantity="10"/>
			 <item type="chips" price="5" quantity="3"/>
		  </sales>;

		QA.ASSERT( ''+sales.item.(@type == "carrot").@quantity, '10', 'data access' );
		QA.ASSERT( ''+sales.@vendor, 'John', 'data access' );
	}	

})