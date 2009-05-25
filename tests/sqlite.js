LoadModule('jsstd');
LoadModule('jssqlite');

try {

	Print('Using SQLite version ' + Database.version ,'\n' );

	var db = new Database(); // in-memory database
	db.Exec('CREATE TABLE color (id INTEGER PRIMARY KEY AUTOINCREMENT, name,r,g,b);');
	function AddColor(name, r,g,b) {
		
		Print( 'Adding '+name+' color.\n');
		db.Exec('insert into color (name,r,g,b) values (?,?,?,?)', arguments);
	}
	
	AddColor('red', 1,0,0);
	AddColor('green', 0,1,0);
	AddColor('blue', 0,0,1);
	AddColor('black', 0,0,0);
	AddColor('white', 1,1,1);

	Print( 'Last ID :'+db.lastInsertRowid ,'\n' );
	
	Print('\n');

	db.toHtmlCode = function(r,g,b) {
		
		return '#' + Math.floor(r*15).toString(16) + Math.floor(g*15).toString(16) + Math.floor(b*15).toString(16);
	}
	
	for each ( row in db.Query('SELECT id, name, toHtmlCode(r,g,b) as htmlCode from color') )
		Print( Expand('<p style="color:$(htmlCode)">$(id): $(name)</p>\n', row) ); // or Print( '<p style="color:'+row.htmlCode+'">'+row.id+': '+row.name+'</p>\n' );
	
	db.Close();

} catch ( ex if ex instanceof SqliteError ) {

	Print( 'SqliteError at '+ex.fileName+':'+ex.lineNumber+': ' + ex.text + ' (code:'+ex.code+')' );
}