try {

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jsio');


//////////////////////////////////////////////////////////////////////////////

LoadModule('jsode');

var world = new World();
world.quickStepNumIterations = 20;
world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
world.defaultSurfaceParameters.softERP = 1;
world.defaultSurfaceParameters.softCFM = 0.000001;
world.defaultSurfaceParameters.bounce = 0.5;
world.defaultSurfaceParameters.bounceVel = 2;


var InputManager = new function () {
	
	LoadModule('jssdl');	
	var self = this;
	
	var listenerList = [];
	this.AddEventListener = function(listener) {
		
		listenerList.push(listener);
	}
	
	showCursor = false;
	grabInput = true;

	while ( PollEvent() ); // clear the event queue

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
				listener.onMouseButtonDown && listener.onMouseButtonDown.apply(listener, arguments);
		},
		onMouseButtonUp: function(button) {
			
			for each ( var listener in listenerList )
				listener.onMouseButtonUp && listener.onMouseButtonUp.apply(listener, arguments);
		},
		onMouseMotion: function(px,py,dx,dy,button) {
			
			for each ( var listener in listenerList )
				listener.onMouseMotion && listener.onMouseMotion.apply(listener, arguments);
		}
	};
	
	this.ProcessEvents = function() {

		PollEvent(listeners);
	}
}


var TrimeshManager = new function() {
	
	LoadModule('jstrimesh');
	
	var trimeshList = [];
		
	this.Load = function(filename) {

		if ( filename in trimeshList )
			return trimeshList[filename];

		var objects = {};
		var mesh = eval('('+(new File(filename).content)+')');
		for ( var id in mesh ) {

			var tm = new Trimesh();
			objects[id] = tm;

			var vertexList = [];
			var normalList = [];
			for each ( var it in mesh[id].vertex ) {
			
				vertexList.push(it[0], it[1], it[2]);
				normalList.push(it[3], it[4], it[5]);
			}
			tm.DefineVertexBuffer(vertexList);
			tm.DefineNormalBuffer(vertexList);

			var faceList = [];
			for each ( var it in mesh[id].face ) {
			
				faceList.push(it[0], it[1], it[2]);
			}
			tm.DefineIndexBuffer(faceList);
		}
		
		return trimeshList[filename] = objects;
	}
}


var SoundManager = new function() {
	
	LoadModule('jsaudio');
	LoadModule('jssound');
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
		var decoder = filename.substr(-4) == '.ogg' ? OggVorbisDecoder : SoundFileDecoder;
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

	LoadModule('jssdl');
	LoadModule('jsgraphics');
	
	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_DEPTH_SIZE, 16 );
	SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

	with (Ogl) {
	
		Hint( PERSPECTIVE_CORRECTION_HINT, NICEST );

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


var TimeManager = new function() {
	
	var prev, t1, t0 = TimeCounter();
	this.Lap = function() {
		
		var t = TimeCounter();
		prev = t - t1;
		t1 = t;
	}
	
	this.__defineGetter__('time', function() t1-t0 );
	this.__defineGetter__('prev', function() prev );
}


var SceneManager = new function() {

	var px, py, pz,  tx, ty, tz;
	var objectList = [];

	this.CameraFOV = function(fov) {
	
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.Perspective(fov, 0.1, 1000);
	}

	this.CameraPosition = function(x,y,z) {
		
		px = x; py = y; pz = z;
	}

	this.CameraTarget = function(x,y,z) {
		
		tx = x; ty = y; tz = z;
	}
	
	this.Add = function(object) {
		
		objectList.push(object);
	}
	
	this.Remove = function(object) {
		
		objectList.splice(objectList.lastIndexOf(object), 1);
	}
	
	this.Update = function() {

		for each ( object in objectList )
			object.Update && object.Update();
	}

	this.Render = function() {
		
		OalListener.position = [px, py, pz];
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.LoadIdentity();
		Ogl.LookAt(px, py, pz, tx, ty, tz, 0,0,1);

		for each ( object in objectList ) {

			if ( !object.Render )
				continue;
			// check if object is inside the frustum (see PlanesCollider::PlanesAABBOverlap)
			// frustum bounding sphere: http://www.flipcode.com/archives/Frustum_Culling.shtml
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
	
	var trimesh = TrimeshManager.Load('sphere.json').Sphere;
	var glTrimeshId = Ogl.LoadTrimesh(trimesh);
	var geom = new GeomTrimesh(trimesh, world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = [10,-5,1];
	
	var impactSound = SoundManager.Load('29996__thanvannispen__stone_on_stone_impact13.aif');

	geom.impact = function(geom, geom2, vel, px, py, pz) {

		if ( vel > 5 )
			SoundManager.Play(impactSound, px, py, pz, vel/100);
	}

	this.onMouseButtonDown = function(button) {
		
		if ( button == BUTTON_WHEELDOWN ) {
		
			var vel = body.linearVel;
			vel[2] -= 10;
			body.linearVel = vel;
		}
	}
	
	this.onMouseMotion = function(px,py,dx,dy,button) {
	
		var vel = body.linearVel;
		vel[0] += dx / 100;
		vel[1] -= dy / 100;
		body.linearVel = vel;
	}
	
	this.Update = function() {
		
		var pos = body.position;
		SceneManager.CameraTarget(pos[0], pos[1], pos[2]);
	}
	
	this.Render = function() {

		Ogl.MultMatrix(geom);
		Ogl.Color(1,0,0);
		Ogl.DrawTrimesh(glTrimeshId);
	}
	
	this.Destroy = function() {

		geom.Destroy();
		body.Destroy();
	} 
}


SceneManager.CameraFOV(60);
SceneManager.CameraPosition(-10,-10,10);
SceneManager.CameraTarget(0,0,0);
SceneManager.Add(new Floor());

var ball = new Ball();
SceneManager.Add(ball);
InputManager.AddEventListener(ball);

CollectGarbage();

var end = false;
while ( !end ) {
	
	TimeManager.Lap();
	InputManager.ProcessEvents();
	SceneManager.Update();
	world.Step(20);
	SceneManager.Render();
	Sleep(10);
	Print( (1000/(TimeManager.prev - 10)).toFixed(0),' fps    \r' );
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