


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

function Debug1() {

	debugger;
	
	var s = new Script('j++');
	
	var a = new loop();
	
	var t = 0;
	eval('toto()');
	
	function inner() {
		function inner1() {
			t++;
		}
	}
	
	inner()
	
	b();
	toto();
	toto();
	toto();
	
	for ( var i=0; i < 100; i++ )
		loop();

//	xxx();

}

function d() {}


