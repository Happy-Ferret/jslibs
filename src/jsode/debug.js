try {

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsgraphics');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsode');
LoadModule('jssound');
LoadModule('jsaudio');


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

/////////////////////


Oal.Open("Generic Software"); // "Generic Hardware", "Generic Software", "DirectSound3D" (for legacy), "DirectSound", "MMSYSTEM"
var dec = new SoundFileDecoder( new File('29996__thanvannispen__stone_on_stone_impact13.aif').Open(File.RDONLY) );
var b = new OalBuffer( SplitChannels(dec.Read())[0] );

var effect = new OalEffect();
effect.type = Oal.EFFECT_REVERB;
effect.reverbDensity = 0;
effect.reverbDiffusion = 1;
effect.reverbGain = 0.1;
effect.reverbDecayTime = 2;

var effectSlot = new OalEffectSlot();
effectSlot.effect = effect;


var srcPool = [];
for ( var i = 0; i < 1; i++ ) {
	
	var src = new OalSource();
	src.effectSlot = effectSlot;
	src.looping = false;
	src.buffer = b;
	srcPool.push(src);
}


var cubeTrimesh = MeshToTrimesh('cube.json').Cube;
var sphereTrimesh = MeshToTrimesh('sphere.json').Sphere;


var world = new World();
//world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;



var boxes = [];
for ( var i = 0; i < 0; i++ ) {

	var box = new GeomBox(world.space);
	box.lengths = [2,2,2];
	box.body = new Body(world);
	box.body.mass.value = 1;
	box.body.position = [0,0,1 + i*2];
	boxes.push(box);
}


var floor = new GeomPlane(world.space);
floor.params = [0,0,1,0]; // floor
//var ceil = new GeomPlane(world.space);
//ceil.params = [0,0,-1,-50]; // ceil


floor.impact = function(geom, geom2, vel, px, py, pz) {

	if ( vel > 5 ) {
		
		var src = srcPool.pop();
		src.Stop();
		src.Position(px, py, pz);
		src.gain = vel/100;
		src.Play();
		srcPool.unshift(src);
	}
}

/*
var testBox = new GeomBox(world.space);
testBox.lengths = [2,2,5];
testBox.body = new Body(world);
testBox.body.position = [-4,-4,1];
*/


var cursor = new GeomTrimesh(sphereTrimesh, world.space);
//Print( 'triangleCount: ', cursor.triangleCount, '\n' );

cursor.body = new Body(world);
cursor.body.position = [10,-5,1];
cursor.body.mass.value = 1;



var box = new GeomBox(world.space);
box.lengths = [2,2,2];
box.body = new Body(world);
box.body.mass.value = 1;
box.body.position = [0,0,2];
boxes.push(box);


//var j = new JointBall(world);
//j.body1 = box.body;

var j = new	JointAMotor(world);
j.body1 = box.body;
j.SetAxis(0, 1, [0,0,1]);
j.maxForce = 1;
j.velocity = 100;
//j.SetAngle(0, 100);

//j.AddTorque0(1000); // ok



//var j = new JointHinge(world);
//j.body1 = box.body;
/*
var j = new JointLMotor(world);
j.body1 = box.body;
j.SetAxis(0, 1, [0,0,1]);
j.maxForce = 10;
j.velocity = 1;
*/


world.defaultSurfaceParameters.softERP = 1;
world.defaultSurfaceParameters.softCFM = 0.000001;
/*
world.defaultSurfaceParameters.slip1 = 0.1;
world.defaultSurfaceParameters.slip2 = 0.001;
*/
//world.defaultSurfaceParameters.bounce = 0.5;
//world.defaultSurfaceParameters.bounceVel = 10;


GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
SetVideoMode( 640, 480, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

with (Ogl) {

	Hint( PERSPECTIVE_CORRECTION_HINT, NICEST );
	Enable(DEPTH_TEST);
	Enable(BLEND); //Enable alpha blending
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
	Enable(CULL_FACE);

	MatrixMode(PROJECTION);
	Perspective(100, 0.001, 1000);
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
		
		var forceMult = modifierState & KMOD_LCTRL ? 10 : 1;
		var vel = cursor.body.linearVel;
		if ( button == 4 )
			vel[2] += forceMult;
		else
		if ( button == 5 )
			vel[2] -= forceMult;
		cursor.body.linearVel = vel;
	},
	onMouseMotion: function(px,py,dx,dy,button) {
		
		var forceMult = modifierState & KMOD_LCTRL ? 10 : 1;
		var vel = cursor.body.linearVel;
		if ( dx )
			vel[0] += forceMult * dx/100;
		if ( dy )
			vel[1] += forceMult * -dy/100;

		if ( button == 1 )
			vel[2] += Math.abs(dx+dy)/200;

		cursor.body.linearVel = vel;
		x += dx;
		y += dy;
	}
};


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

function Point(x,y,z) {

	with (Ogl) {
		PushAttrib(LIGHTING | DEPTH_TEST);
		Disable(LIGHTING);
		Disable(DEPTH_TEST);
		Enable(POINT_SMOOTH);
		PointSize(3);
		Begin(POINTS);
		Vertex(x,y,z);
		End();
		PopAttrib();
	}
}

function Axis(x,y,z) {

	with (Ogl) {
		PushAttrib(LIGHTING | DEPTH_TEST);
		Disable(LIGHTING);
		Disable(DEPTH_TEST);
		LineWidth(2);
		Begin(LINES);
		Color(1,0,0);
		Vertex(x,y,z);
		Vertex(x+1,y,z);
		Color(0,1,0);
		Vertex(x,y,z);
		Vertex(x,y+1,z);
		Color(0,0,1);
		Vertex(x,y,z);
		Vertex(x,y,z+1);
		End();
		PopAttrib();
	}
}


var cubeId = Ogl.LoadTrimesh(cubeTrimesh);
var sphereId = Ogl.LoadTrimesh(sphereTrimesh);

function Draw() {

	with (Ogl) {
	
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		LoadIdentity();

// camera

//		var p = cursor.position;
		var t = new Transformation(undefined);
		t.LookAt(-10,-10,10, 0,0,0);
		MultMatrix(t);

		OalListener.position = t.translation;
		
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
		Color(1,0,0);
		Ogl.DrawTrimesh(sphereId);
		PopMatrix();
/*
		Color(1,1,1);
		Point.apply(this, j1.anchor);
		Color(1,1,0);
		Point.apply(this, j1.anchor2);
*/
		PushMatrix();
		
		var t = new Transformation(undefined);
//		t.LookAt(cursor.body.position[0], cursor.body.position[1], cursor.body.position[2]);
		MultMatrix(t);
		Axis(0,0,0);
		PopMatrix();
	}
	
	

}

world.quickStepNumIterations = 20;

CollectGarbage();

var defaultTime = 15;
var t0, at, t;

while (PollEvent()); // clear the event queue

while ( !done ) {

	var t0 = TimeCounter();
	PollEvent(listeners);
	if ( t > 30 )
		t = 20;
	if ( t )
		world.Step(t + at);
	Draw();
	GlSwapBuffers();
	t = TimeCounter() - t0;
	at = t < defaultTime ? defaultTime - t : 0
	Sleep(at);
}



} catch(ex) {
	
	Print( ex.fileName+':'+ex.lineNumber+' '+ex, '\n' );
	Halt();
}



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


function DumpMatrix(m) {
    
    for (var y = 0; y < 4; ++y) {
        Print('[  ' );
        for (var x = 0; x < 4; ++x)
            Print( m[x+y*4].toFixed(1) + '  ' );
        Print(']\n\n' );
    }
}

function Renderer() {
	
	var camera = new Transformation();
	var objectList = [];
	
	this.SetCameraPosition = function(x,y,z) {
		
		camera.Translation(-x, -y, -z);
	}

	this.SetCameraTarget = function(x,y,z) {
		
		camera.LookAt(x, y, z);
	}
	
	this.Add = function(object) {
		
		objectList.push(object);
	}
	
	this.Render = function(time) {

		Ogl.Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		Ogl.LoadIdentity();
		Ogl.MultMatrix(camera);

		for each ( object in objectList )
			this.Render();
	}
}


function Particle() {
	
	this.Render = function(time) {
	}
}

