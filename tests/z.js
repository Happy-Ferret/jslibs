exec('deflib.js');
LoadModule('jsz');

try {

var compressor = new Z(Z.DEFLATE);
var res = compressor.Transform( '111111111111111111111111111111111111111111111111111111111111111' );
res += compressor.Transform();

//var compressor2 = new Z(Z.INFLATE);
//var res2 = compressor2.Transform( res );
//res2 += compressor2.Transform();
//compressor2.Finalize();

print( res.length );

} catch (ex) {
	print( ex.const );
}