


function a() {}
function b() {}

function toto() {

	var i = 5;
	i++;
	function cc() {
	
		i++;
		i++;
		i++;
	}
	const a = 123;
	i++;
	i++;
	cc();
}

var j = 0;
function loop() {

	j++; j++;
	
}

function test() {
	
	return 12345;
}

function Debug1() {

	debugger;
	
	var data = { d1:#1={ a:[4,5,6], b:null, b1:undefined, c:'test', d:{x:1, y:0, z:1}, e:123, e1:new Number(123), f:#1# } };

	test(123, 'abc', [4,5,6], new Date(), {}, {__proto__:null}, null, undefined );


	!function(v) {
		
		if ( v > 0 )
			arguments.callee(v-1)
	}(100);
	
	
	for ( k = 0; k < 100; k++ ) {
		
		j++;
	}
	
	var s = new Script('j++');
	
	var a = new loop();
	
	var t = 0;
	eval('toto()');
	
	function inner() {
		function inner1() {
			t++;
		}
		inner1();
	}
	
	inner()
	
	b();
	toto();
	toto();
	toto();
	
	for ( var i=0; i < 100; i++ )
		loop();

	xxx();

}

function d() {}


