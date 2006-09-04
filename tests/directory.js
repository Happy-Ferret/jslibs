exec('deflib.js');
LoadModule('jsnspr');

try {
var dir=new Directory( 'c:/tmp' );

dir.Open();

for ( var entry; ( entry = dir.Read() ); ) {
	var file = new File(dir.name+'/'+entry);
	print( entry + ' ('+ file.info.type +')', '\n');
}
} catch(ex) {
print( ex.text );
}