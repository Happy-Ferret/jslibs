LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jsdebug');

////////////////////////

var testIterate = 3;


/////////////////////////

var issues = 0;

var _QA = new function() {
	this.ASSERT = function( value, expect, testName ) {

		if ( value !== expect ) {

			issues++;
			Print( '    - ' + testName + ' ('+value+' != '+expect+')', '\n' );
		}
	}
}


var dirList = Directory.List('.', Directory.SKIP_BOTH | Directory.SKIP_FILE | Directory.SKIP_OTHER );
dirList.sort();

for each ( var dirName in dirList ) {
	
	var f = new File(dirName + '/qa.js');
	if ( f.exist ) {
		
		var qatests = Exec(f.name, false)(_QA);

		Print( f.name, '\n' );
		for ( var testName in qatests ) {
			
			Print( '  ' + testName, '\n' );

			if ( arguments[1] && arguments[1] != testName ) {
			
				Print( '    X Skiped.', '\n' );
				continue;
			}
			
			if ( testName[0] == '_' ) {
			
				Print( '    X Disabled', '\n' );
				continue;
			}

			try {
				
				for ( var i = 0; i<testIterate; i++ ) {
				
					qatests[testName]();
					CollectGarbage();
				}

			} catch ( ex ) {

				Print( '    ! ' + ex.constructor.name +': '+ ex.text + ' ('+ex.code+') '+(ex.stack||'<no stack>'), '\n' );
			}
			
		}
	}
}

Print( issues + ' issues found' );



