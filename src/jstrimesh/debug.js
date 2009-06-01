LoadModule('jsstd');
LoadModule('jstrimesh');
LoadModule('jsio');

function MeshToTrimesh(filename) {

	var trimeshList = {};
	var mesh = eval('('+(new File('cube.json').content)+')');
	for ( var id in mesh ) {

		var tm = new Trimesh();
		trimeshList[id] = tm;

		var vertexList = [];
		for each ( var it in mesh[id].vertex )
			vertexList.push(it[0], it[1], it[2]);
		tm.DefineVertexBuffer(vertexList);

		var faceList = [];
		for each ( var it in mesh[id].face )
			faceList.push(it[0], it[1], it[2]);
		tm.DefineIndexBuffer(faceList);
	}
	return trimeshList;
}

Print(uneval(MeshToTrimesh('cube.json')));