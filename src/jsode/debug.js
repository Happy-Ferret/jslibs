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

//////////////////////////////////////////////////////////////////////////////

var world = new World();
world.quickStepNumIterations = 20;
world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
world.defaultSurfaceParameters.softERP = 1;
world.defaultSurfaceParameters.softCFM = 0.000001;


var InputManager = new function () {
	
	var self = this;
	
	var listenerList = [];
	this.AddEventListener = function(listener) {
		
		listenerList.push(listener);
	}
	
	showCursor = false;
	grabInput = true;

	while (PollEvent()); // clear the event queue

	var listeners = {
		onQuit: function() done = true,
		onKeyDown: function(key, mod) {
			
			switch (key) {
				case K_ESCAPE:
					end = true;
					break;
			}
		},
		onVideoResize: function(w,h) {

			Ogl.Viewport(0, 0, w, h);
		},
		onMouseButtonDown: function(button) {
			
			for each ( var listener in listenerList )
				listener.onMouseDown && listener.onMouseButtonDown(button);
		},
		onMouseButtonUp: function(button) {
			
			for each ( var listener in listenerList )
				listener.onMouseButtonUp && listener.onMouseButtonUp(button);
		},
		onMouseMotion: function(px,py,dx,dy,button) {
			
			for each ( var listener in listenerList )
				listener.onMouseMotion && listener.onMouseMotion(px,py,dx,dy,button);
		}
	};
	
	this.ProcessEvents = function() {

		PollEvent(listeners);
	}
}


var SoundManager = new function() {

	Oal.Open("Generic Software"); // "Generic Hardware", "Generic Software", "DirectSound3D" (for legacy), "DirectSound", "MMSYSTEM"

	var bufferList = {};
	var srcPool = [];
		
	var effect = new OalEffect();
	effect.type = Oal.EFFECT_REVERB;
	effect.reverbDensity = 0;
	effect.reverbDiffusion = 1;
	effect.reverbGain = 0.1;
	effect.reverbDecayTime = 2;

	var effectSlot = new OalEffectSlot();
	effectSlot.effect = effect;

	for ( var i = 0; i < 10; i++ ) {
		
		var src = new OalSource();
		src.effectSlot = effectSlot;
		src.looping = false;
		srcPool.push(src);
	}
	
	this.Load = function(filename) {

		if ( filename in bufferList )
			return bufferList[filename];
		var decoder = filename.substr(-4) == '.ogg' ? OggVorbisDecoder :SoundFileDecoder;
		var decoder = new decoder( new File(filename).Open(File.RDONLY) );
		return bufferList[filename] = new OalBuffer( SplitChannels(decoder.Read())[0] );
	}

	this.Play = function(buffer, x,y,z, gain) {
		
		var src = srcPool.pop();
		src.Stop();
		src.buffer = buffer;
		src.Position(x,y,z);
		src.gain = gain;
		src.Play();
		srcPool.unshift(src);
	}
}	


var DisplayManager = new function() {
	
	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_DEPTH_SIZE, 16 );
	SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

	with (Ogl) {
	
		Hint( PERSPECTIVE_CORRECTION_HINT, NICEST );
		MatrixMode(PROJECTION);
		Perspective(60, 0.001, 1000);
		MatrixMode(MODELVIEW);

		Enable(BLEND); //Enable alpha blending
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
		Enable(CULL_FACE);
	
		Enable(LIGHTING);
		Enable(LIGHT0);
		Light(LIGHT0, SPECULAR, [1, 1, 1, 1]);
//	Light(LIGHT0, POSITION, [-1,-2, 10]);
//  LightModel(LIGHT_MODEL_AMBIENT, [0.5, 0.5, 0.5, 1]);
//  ShadeModel(SMOOTH);
 
	  Enable( COLOR_MATERIAL );
//  ColorMaterial( FRONT_AND_BACK, EMISSION );
	}

}


var Renderer = new function() {

	var px, py, pz,  tx, ty, tz;
	var objectList = [];

	this.SetCameraPosition = function(x,y,z) {
		
		px = x; py = y; pz = z;
	}

	this.SetCameraTarget = function(x,y,z) {
		
		tx = x; ty = y; tz = z;
	}
	
	this.Add = function(object) {
		
		objectList.push(object);
	}
	
	this.Remove = function(object) {
		
		objectList.splice(objectList.lastIndexOf(object), 1);
	}

	this.Render = function() {
		
		OalListener.position = [px, py, pz];
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		Ogl.LoadIdentity();
		Ogl.LookAt(px, py, pz, tx, ty, tz, 0,0,1);

		for each ( object in objectList ) {
		
			Ogl.PushMatrix();
			object.Render();
			Ogl.PopMatrix();
		}
		
		GlSwapBuffers();
	}
}



function Floor() {
	
	var self = this;
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.Render = function() {
		
		Ogl.Normal(0,0,1);
		Ogl.Translate(-10,-10,0);
		Ogl.Scale(2,2,0);
		var cx=10, cy=10;
//		Ogl.Translate(-1,-1,0);
		Ogl.Begin(Ogl.QUADS);
		for ( var x = 0; x < cx; x++ )
			for ( var y = 0; y < cy; y++ ) {
				if ( (x + y) % 2 )
					Ogl.Color(0,0,0.5);
				else
					Ogl.Color(0.9,0.8,0.7);
				Ogl.Vertex(x,y);
				Ogl.Vertex(x+1,y);
				Ogl.Vertex(x+1,y+1);
				Ogl.Vertex(x, y+1);
			}
		Ogl.End();
	}	
}



function Ball() {
	
	var self = this;
	var trimesh = MeshToTrimesh('sphere.json').Sphere;
	var glTrimeshId = Ogl.LoadTrimesh(trimesh);
	var geom = new GeomTrimesh(trimesh, world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = [10,-5,10];
	
	var impactSound = SoundManager.Load('29996__thanvannispen__stone_on_stone_impact13.aif');

	geom.impact = function(geom, geom2, vel, px, py, pz) {

		if ( vel > 5 ) {

			SoundManager.Play(impactSound, px, py, pz, vel/100);
		}
	}
	
	this.Render = function() {

		Ogl.MultMatrix(body);
		Ogl.Color(1,0,0);
		Ogl.DrawTrimesh(glTrimeshId);
	}
	
	this.Destroy = function() {

		geom.Destroy();
		body.Destroy();
	} 
}

Renderer.SetCameraPosition(-10,-10,10);
Renderer.SetCameraTarget(0,0,0);
Renderer.Add(new Floor());
Renderer.Add(new Ball());


var end = false;

while ( !end ) {

	InputManager.ProcessEvents();
	world.Step(20);
	Renderer.Render();
	Sleep(10);
}



/*
var box = new GeomBox(world.space);
box.lengths = [2,2,2];
box.body = new Body(world);
box.body.mass.value = 1;
box.body.position = [0,0,2];
boxes.push(box);

var j = new	JointAMotor(world);
j.body1 = box.body;
j.SetAxis(0, 1, [0,0,1]);
j.maxForce = 1;
j.velocity = 100;
*/


/*
		LoadIdentity();
		Translate(10,10,10);
		LookAt(10, 10, 15, 0,0,0, 0,0,1);
		Print('gl '); PrintGlMatrix();

		var t = new Transformation(undefined);
		t.Translate(10,10,10);
		t.LookAt(10, 10, 15, 0,0,0, 0,0,1);
		LoadMatrix(t);
		Print('tr '); PrintGlMatrix();
		Print('\n'); 
*/

} catch(ex) {
	
	Print( ex.fileName+':'+ex.lineNumber+' '+ex, '\n' );
	Halt();
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


function Cube() {

	with (Ogl) {
		Begin(QUADS);		// Draw The Cube Using quads

		Color(1,0,0);
		Vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Right)
		Vertex( 0.5, 0.5, 0.5);	// Top Left Of The Quad (Right)
		Vertex( 0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Right)
		Vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Right)

		Color(0.5,0,0);
		Vertex(-0.5, 0.5, 0.5);	// Top Right Of The Quad (Left)
		Vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Left)
		Vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Left)
		Vertex(-0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Left)

		Color(0,1,0);
		Vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Top)
		Vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Top)
		Vertex(-0.5, 0.5, 0.5);	// Bottom Left Of The Quad (Top)
		Vertex( 0.5, 0.5, 0.5);	// Bottom Right Of The Quad (Top)

		Color(0,0.5,0);
		Vertex( 0.5,-0.5, 0.5);	// Top Right Of The Quad (Bottom)
		Vertex(-0.5,-0.5, 0.5);	// Top Left Of The Quad (Bottom)
		Vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Bottom)
		Vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Bottom)

		Color(0,0,1);
		Vertex( 0.5, 0.5, 0.5);	// Top Right Of The Quad (Front)
		Vertex(-0.5, 0.5, 0.5);	// Top Left Of The Quad (Front)
		Vertex(-0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Front)
		Vertex( 0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Front)

		Color(0,0,0.5);
		Vertex( 0.5,-0.5,-0.5);	// Top Right Of The Quad (Back)
		Vertex(-0.5,-0.5,-0.5);	// Top Left Of The Quad (Back)
		Vertex(-0.5, 0.5,-0.5);	// Bottom Left Of The Quad (Back)
		Vertex( 0.5, 0.5,-0.5);	// Bottom Right Of The Quad (Back)
		End();
	}
}