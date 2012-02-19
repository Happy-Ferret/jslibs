// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jstrimesh');
loadModule('jsio');

function meshToTrimesh(filename) {

	var trimeshList = {};
	var mesh = eval('('+(new File(filename).content)+')');
	for ( var id in mesh ) {

		var tm = new Trimesh();
		trimeshList[id] = tm;

		var vertexList = [];
		var normalList = [];
		for each ( var it in mesh[id].vertex ) {
			vertexList.push(it[0], it[1], it[2]);
			normalList.push(it[3], it[4], it[5]);
		}
		tm.defineVertexBuffer(vertexList);
		tm.defineNormalBuffer(vertexList);

		var faceList = [];
		for each ( var it in mesh[id].face )
			faceList.push(it[0], it[1], it[2]);
		tm.defineIndexBuffer(faceList);
	}
	return trimeshList;
}

print(uneval(meshToTrimesh('cube.json')));
