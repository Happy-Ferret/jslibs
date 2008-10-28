LoadModule('jsstd');
LoadModule('jstrimesh');
LoadModule('jsio');

var t = new Trimesh();

t.AddVertex(1,1,1);
t.AddVertex(2,2,2);
t.AddVertex(2,2,0);

t.AddTriangle(0,1,2);


