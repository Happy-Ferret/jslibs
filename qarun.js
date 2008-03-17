LoadModule('jsstd');
LoadModule('jsio');

var issues = 0;

function QAASSERT( cond, message ) {

	if ( !cond ) {

		issues++;
		Print( '    - ' + message, '\n' );
	}
}

var dirList = Directory.List('.', Directory.SKIP_BOTH | Directory.SKIP_FILE | Directory.SKIP_OTHER );
dirList.sort();

for each ( var dirName in dirList ) {
	
	var f = new File(dirName + '/qa.js');
	if ( f.exist ) {
		
		var qatests = Exec(f.name, false)(QAASSERT);

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
	
				qatests[testName]();

			} catch ( ex ) {

				Print( '    ! ' + ex.constructor.name +': '+ ex.text + ' ('+ex.code+') "'+(ex.stack||'???')+'"', '\n' );
			}
			
			CollectGarbage();
		}
	}
}

Print( issues + ' issues found' );



