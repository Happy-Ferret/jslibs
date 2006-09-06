exec('deflib.js');
LoadModule('jsz');


function randomString(size) {

//return 'L\'autre jour, au café, je commande un demi. J\'en bois la moitié. Il ne m\'en restait plus. L\'héroïsme, c\'est encore la meilleure façon de devenir célèbre quand on n\'a pas de talent.';

	var res = '';
	while(size--)	res+=String.fromCharCode(Math.random()*256)
	return res;
}

function test(count) {

	while(count--) {

		var compressor = new Z(Z.DEFLATE,9);
		
		var str = '';
		var res = '';
		for ( var i=100; i>=0; --i ) {
			var chunk = randomString(10000);
			res += compressor.Transform( chunk );
			str += chunk;
		}
		res += compressor.Transform();

		var compressor2 = new Z(Z.INFLATE);
		var res2 = compressor2.Transform( res );
		res2 += compressor2.Transform();
		
		print( 'ratio:'+ res.length + ': ' + Math.round( 100 * res.length / str.length ) + '%','\n');
		if ( res2 != str ) {
			print('error\n');
		}
	
	}
}



function test2() {

		var compressor = new Z(Z.DEFLATE,0);
		var compressor2 = new Z(Z.INFLATE);
		
		var str = '';
		var str2 = '';
		for ( var i=10; i>=0; --i ) {
		
		
			str = compressor.Transform( randomString( i*10000 +1 ) );
			str2 += compressor2.Transform(str);
		}
		
		str2 += compressor2.Transform( compressor.Transform() );
		compressor2.Transform();

		print( 'adler32:' + (compressor2.adler32 == compressor.adler32) ,'\n');
		
		
		
}


try {

/*
var compressor = new Z(Z.DEFLATE);
var res = compressor.Transform( 'hello ' );
res += compressor.Transform( 'world.' );
res += compressor.Transform();


var compressor2 = new Z(Z.INFLATE);
var res2 = compressor2.Transform( res );
res2 += compressor2.Transform();

print( '['+res2+']' );
*/

test2();

} catch (ex if ex instanceof ZError) {
	print( ex.const + ' ' + ex.text, '\n' );
}

print('done')
