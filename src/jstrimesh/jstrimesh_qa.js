LoadModule('jsstd');
LoadModule('jstrimesh');

/// basic test

	var t = new Trimesh();


	t.DefineVertexBuffer([
		0,0,0,
		0,0,1,
		0,1,0,
		1,0,0
	]);

	t.DefineIndexBuffer([
		0,1,2,
		0,3,1
	]);



