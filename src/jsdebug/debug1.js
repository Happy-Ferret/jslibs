


function a() {}
function b() {}

function toto() {

	var i = 5;
	i++;
	function c() {
		i++;
	}
	const a = 123;
	i++;
	i++;
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
	
	for ( var i=0; i < 100; i++ )
		loop();


}

function d() {}
