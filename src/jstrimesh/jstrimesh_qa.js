loadModule('jsstd');
loadModule('jstrimesh');

/// basic test [rmtf]

	var t = new Trimesh();


	t.defineVertexBuffer([
		0,0,0,
		0,0,1,
		0,1,0,
		1,0,0
	]);

	t.defineIndexBuffer([
		0,1,2,
		0,3,1
	]);



