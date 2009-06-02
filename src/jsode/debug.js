LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsode');

function MeshToTrimesh(filename) {

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
		tm.DefineVertexBuffer(vertexList);
		tm.DefineNormalBuffer(vertexList);

		var faceList = [];
		for each ( var it in mesh[id].face )
			faceList.push(it[0], it[1], it[2]);
		tm.DefineIndexBuffer(faceList);
	}
	return trimeshList;
}



var cubeTrimesh = MeshToTrimesh('cube.json').Cube;
var sphereTrimesh = MeshToTrimesh('sphere.json').Sphere;


var world = new World();
world.gravity = [0,0,-9.809];

var floor = new GeomPlane(world.space);
floor.params = [0,0,1,0]; // floor
var floor = new GeomPlane(world.space);
floor.params = [0,0,-1,-50]; // ceil



var boxes = [];
for ( var i = 0; i < 20; i++ ) {

//	var box = new GeomTrimesh(world.space, cubeTrimesh);
	var box = new GeomBox(world.space);
	box.lengths = [2,2,2];
	box.body = new Body(world);
	box.body.position = [0,0,1 + i*2];
	boxes.push(box);
}

//var cursor = new GeomTrimesh(world.space, cubeTrimesh);
var cursor = new GeomSphere(world.space);
cursor.radius = 2;
cursor.body = new Body(world);
cursor.body.position = [10,10,cursor.radius];
cursor.body.mass.value = 20;


world.defaultSurfaceParameters.softERP = 0.2;
/*
world.defaultSurfaceParameters.softCFM = 0.000001;
world.defaultSurfaceParameters.slip1 = 0.1;
world.defaultSurfaceParameters.slip2 = 0.001;
*/
world.defaultSurfaceParameters.bounce = 0.3;
world.defaultSurfaceParameters.bounceVel = 10;


GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
SetVideoMode( 800, 600, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

with (Ogl) {

	Enable(DEPTH_TEST);
	Enable(BLEND); //Enable alpha blending
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
	Enable(CULL_FACE);

	MatrixMode(PROJECTION);
	Perspective(120, 0.01, 1000);
	MatrixMode(MODELVIEW);
	
	Enable(LIGHTING);
	Enable(LIGHT0);
	Light(LIGHT0, SPECULAR, [1, 1, 1, 1]);
//	Light(LIGHT0, POSITION, [-1,-2, 10]);
//  LightModel(LIGHT_MODEL_AMBIENT, [0.5, 0.5, 0.5, 1]);
//  ShadeModel(SMOOTH);
  
  Enable( COLOR_MATERIAL );
//  ColorMaterial( FRONT_AND_BACK, EMISSION );

	LoadIdentity();
	PushMatrix();
}

var [x,y,] = cursor.body.position;

showCursor = false;
grabInput = true;

var done = false;
var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) {
		
		switch (key) {
			case K_ESCAPE:
				done = true;
				break;
		}
	},
	onVideoResize: function(w,h) {

		Ogl.Viewport(0, 0, w, h);
	},
	onMouseButtonUp: function(button) {
		
		var vel = cursor.body.linearVel;
		if ( button == 4 )
			vel[2] += 1;
		else
		if ( button == 5 )
			vel[2] -= 1;
		cursor.body.linearVel = vel;
		
	},
	onMouseMotion: function(px,py,dx,dy,button) {
		
		var vel = cursor.body.linearVel;
		if ( dx )
			vel[0] += dx/100;
		if ( dy )
			vel[1] += -dy/100;

		if ( button == 1 )
			vel[2] += Math.abs(dx+dy)/200;

		cursor.body.linearVel = vel;
		x += dx;
		y += dy;
	}
};


function Cube(x, y, z) {

	with (Ogl) {
		Begin(QUADS);
		Normal(0.5,0.5,-1);
		Vertex(0,0,0);
		Vertex(0,y,0);
		Vertex(x,y,0);
		Vertex(x,0,0);

		Vertex(0,0,0);
		Vertex(0,0,z);
		Vertex(0,y,z);
		Vertex(0,y,0);

		Vertex(0,0,0);
		Vertex(x,0,0);
		Vertex(x,0,z);
		Vertex(0,0,z);

		Vertex(x,0,0);
		Vertex(x,y,0);
		Vertex(x,y,z);
		Vertex(x,0,z);

		Vertex(0,y,0);
		Vertex(0,y,z);
		Vertex(x,y,z);
		Vertex(x,y,0);

		Vertex(0,0,z);
		Vertex(x,0,z);
		Vertex(x,y,z);
		Vertex(0,y,z);
		End();
	}

}

function DrawFloor(cx, cy) {

	with (Ogl) {
		Begin(QUADS);
		for ( var x = 0; x < cx; x++ )
			for ( var y = 0; y < cy; y++ ) {
				if ( (x + y) % 2 )
					Color(0,0,0.5);
				else
					Color(0.9,0.8,0.7);
				Vertex(x,y);
				Vertex(x+1,y);
				Vertex(x+1,y+1);
				Vertex(x, y+1);
			}
		End();
	}
}

var cubeId = Ogl.LoadTrimesh(cubeTrimesh);
var sphereId = Ogl.LoadTrimesh(sphereTrimesh);

function Draw(t) {

	with (Ogl) {
	
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();

// camera		
		Translate(0, 0, -10);
		Rotate(-60, 1, 0, 0);
		Rotate(30, 0, 0, 1);
		Translate(0, 0, -10);

// floor
		PushMatrix();
		Scale(5,5,0);
		Translate(-5, -5, 0);
		DrawFloor(10, 10);
		PopMatrix();
	
// objects
		for each (box in boxes) {
			PushMatrix();
			MultMatrix(box.body);
			Color(0.5,0.5,0.5);
			Ogl.DrawTrimesh(cubeId);
			PopMatrix();
		}
		
		PushMatrix();
		MultMatrix(cursor.body);
		Color(0,0,1);
		Scale(2,2,2);
		Ogl.DrawTrimesh(sphereId);
		PopMatrix();
	}
}


var t0, t1;

CollectGarbage();
while ( !done ) {

	PollEvent(listeners);
	
	var t1 = TimeCounter()/1000;

	if ( !t0 )
		t0 = t1-0.001;
	
//	cursor.body.position = [x/100, -y/100, 0.5];
//	cursor.body.quaternion = [0,0,0,1];

	world.Step(t1-t0, 20);

	Draw(t1);
	t0 = t1;
	GlSwapBuffers();
	Sleep(10);
}



Halt(); //////////////////////////////////////////////////////////////////////








var world = new World;


var t = new Trimesh();

t.DefineVertexBuffer([
	0,0,0,
	1,0,0,
	0,1,0,
	0,0,1,
]);

t.DefineIndexBuffer([
	0,3,1,
	0,1,2,
	0,2,3,
	1,3,2,
]);


var g = new GeomTrimesh( world.space, t );



Halt();

function main() {

	var g = new GeomBox();

//	return;

	/*
	var world = new World;
	world.gravity = [0,0,-0.81];

	var body1 = new Body(world);
	var body2 = new Body(world);
	var joint = new JointHinge(world);


	joint.body1 = body1;
	joint.body2 = body2;
	joint.anchor = [4,4,0];
	joint.axis = [1,0,0];
	joint.loStop = 1;
	joint.hiStop = 1.5;

	body1.linearVel = [0,0,19];
	body1.angularVel = [1,1,0];
	*/



	var world = new World;
	//world.Body = Body;
	//var b = new world.Body();

	//world.gravity = [0,0,-9.81];

	var body1 = new Body(world);
	var g = new GeomBox( world.space );
	g.body = body1;
	g.impact = function(n, against, pos) { Print('hit'+n+'@['+pos+']\n') }


	var body2 = new Body(world);
	new GeomBox( world.space ).body = body2;

	//body1.mass.SetBoxTotal(10,[1,1,100]);
	//body.mass.mass = 1;
	//body.mass.Translate([2,1,0]);
	//body.mass.center = [2,0,0];
	//body.mass.Adjust(10);

	var joint = new JointHinge(world);

	//joint.Attach(body,body1);
	joint.body1 = body1;
	joint.body2 = body2;
	joint.anchor = [10,0,0];
	joint.axis = [1,1,1];
	
	joint.CFM = Infinity;

	joint.useFeedback = true;

	body1.position = [0,0,0]
	new JointHinge(world)

	body2.position = [0,0,5]
	body2.linearVel = [0,0,-5];

	var floor = new GeomPlane(world.space);

	//body.linearVel = [10,10,10];
	//body.angularVel = [10,0,0];

	for ( var i = 0; i<20; i++ ) {
	//	Print( 'Step:', i,  '\n' );


	//	Print( ' Position:', body1.position , '\n' );

	//	Print( ' Force:', body.force , '\n' );
		world.Step(0.1,20);
		
		Print( ' ->', joint.body1Torque, '\n' );
		
	}

	//Print( 'joint angle rate:'+joint.angleRate ,'\n');
	
	delete g.impact;
}

main();

