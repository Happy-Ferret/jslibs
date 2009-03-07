


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

toto();

function d() {}
