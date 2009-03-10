


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
	
	a();
	b();
	toto();
	toto();
	toto();
	
	for ( var i=0; i < 100; i++ )
		loop();

	xxx();

}

function d() {}
