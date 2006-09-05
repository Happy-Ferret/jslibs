exec('deflib.js');
LoadModule('jsz');


//for (var i=0; i<10000; i++);

try {

var compressor = new Z(Z.DEFLATE);
var res = compressor.Transform( 'hello ' );
res += compressor.Transform( 'world.' );
res += compressor.Transform();


var compressor2 = new Z(Z.INFLATE);
var res2 = compressor2.Transform( res );
res2 += compressor2.Transform();


print( '['+res2+']' );

} catch (ex if ex instanceof ZError) {
	print( ex.const );
}