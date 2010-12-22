
// LoadModule('jsstd');  LoadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };  Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();


LoadModule('jsstd'); Exec('../../tests/explodebox.js'); throw 0;

LoadModule('jsode');

LoadModule('jsstd');
Exec('../common/tools.js');

var w = new World();
w.gravity = [0,0,-9.809];
w.gravity = [0,0,0];


function Pod() {
	
	var _this = this;

	var m1 = new JointLMotor(w);
	m1.body1 = new Body(w);
	m1.body1.position = [-2,0,0];
	m1.SetAxis(0, 1, [0,0,1]);
	m1.maxForce = 0;
	m1.velocity = Infinity;

	var m2 = new JointLMotor(w);
	m2.body1 = new Body(w);
	m2.body1.position = [2,0,0];
	m2.SetAxis(0, 1, [0,0,1]);
	m2.maxForce = 0;
	m2.velocity = Infinity;

	var center = new Body(w);
	center.position = [0,0,0];

	var j1 = new JointFixed(w);
	j1.body1 = center;
	j1.body2 = m1.body1;
	j1.Set();

	var j2 = new JointFixed(w);
	j2.body1 = center;
	j2.body2 = m2.body1;
	j2.Set();
	
	function M1Force(ratio) {
		
		if ( ratio < 0 )
			ratio = 0;
		else if ( ratio > 1 )
			ratio = 1;
		m1.maxForce = ratio * 10;
	}

	function M2Force(ratio) {
		
		if ( ratio < 0 )
			ratio = 0;
		else if ( ratio > 1 )
			ratio = 1;
		m2.maxForce = ratio * 10;
	}
	
	this.Draw = function(env3d) {

	//	env3d.AddTrail(1, m1.body1.position);
	//	env3d.AddTrail(2, m2.body1.position);
		env3d.AddTrail(1, center.position);

		env3d.Begin();
		env3d.DrawGrid();
		
		env3d.DrawTrail(1);
	//	env3d.DrawTrail(2);
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(m1.body1);
		env3d.Draw3DArrow();
		Ogl.LineWidth(2);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,1,1, 1 ); Ogl.Vertex(0,0,-1); Ogl.Vertex(0,0,-1-m1.maxForce);
		Ogl.End();
		Ogl.LineWidth(1);

		Ogl.PopMatrix();

		Ogl.PushMatrix();
		Ogl.MultMatrix(m2.body1);
		env3d.Draw3DArrow();

		Ogl.LineWidth(2);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,1,1, 1 ); Ogl.Vertex(0,0,-1); Ogl.Vertex(0,0,-1-m2.maxForce);
		Ogl.End();
		Ogl.LineWidth(1);
		
		Ogl.PopMatrix();
/*
		Ogl.PushMatrix();
		Ogl.Translate(aim[0], aim[1], aim[2]);
		env3d.Draw3DAxis();
		Ogl.PopMatrix();
*/		
	}

	var taskList = [];

	this.Step = function() {
		
		var task = taskList[0];
		
		if ( IsGenerator(task) ) {
		
			try {

				task.next();
			} catch (ex if ex instanceof StopIteration) {

				taskList.shift();
			}
			return;
		}
		
		if ( IsFunction(task) ) {

			task();
			return;
		}
	}
	
	function CreateTask(task) {
		
		return function() {
			
			taskList.push(task.apply(this, arguments));
		}
	}
	
	
	
	
	this.PushLeft = CreateTask(function(force) {
	
		m1.maxForce = force;
	});
	

	this.PushRight = function(force) taskList.unshift(new function() {
	
		m2.maxForce = force;
		yield;
	});

	this.Straight = function() taskList.unshift(new function() {
	});

	this.TurnUntil = function(direction, predicate) taskList.unshift(new function() {
	});

	this.Stop = function() taskList.unshift(new function() {
	
		var vel = center.linearVel;

		var err = Vector3Dot( center.Vector3ToWorld([0,0,-1]), Vector3Normalize(vel) );

		for (;;) {
			
			var front = center.Vector3ToWorld([0,0,1]);
			var ok = -Vector3Dot( front, Vector3Normalize(vel) );
			
			var d1 = Vector3Dot( vel, center.Vector3ToWorld([-1,0,0]) );
			var d2 = Vector3Dot( vel, center.Vector3ToWorld([1,0,0]) );
			
			var r1 = Vector3Dot( Vector3Sub(m1.body1.linearVel, vel), front );
			var r2 = Vector3Dot( Vector3Sub(m2.body1.linearVel, vel), front );
			
			
			Print( d1.toFixed(2), '  ', d2.toFixed(2), ' ok:', ok.toFixed(2) );
			Print( ' r1:', r1.toFixed(2) );
			Print( ' r2:', r2.toFixed(2) );
			Print( StringRepeat(' ', 20), '\r');

			var f1 = 0, f2 = 0;
			
			if ( ok < 0 && r1 < 1 && r2 < 1 ) {

				if ( d1 > 0 )
					f1 = 1;
				if ( d2 > 0 )
					f2 = 1;
			}

			if ( ok > 0 && r1 < 1 && r2 < 1 ) {
			
			}




			var speed = Vector3Length(vel);
			if ( speed < 0.1 && speed > -0.1 ) {

				M1Force(0);
				M2Force(0);
				return;
			}


			M1Force(f1);
			M2Force(f2);

			yield;
		}	
		
	});

	this.UTurn = function() taskList.unshift(new function() {
		
		var aimdir = center.Vector3ToWorld([0,0,-1]);
		
		for (;;) {

			var curdir = center.Vector3ToWorld([0,0,1]);
			var ok = Vector3Dot( aimdir, curdir );
			
			var d1 = Vector3Dot( aimdir, center.Vector3ToWorld([-1,0,0]) );
			var d2 = Vector3Dot( aimdir, center.Vector3ToWorld([1,0,0]) );
			
			var m1ok = Vector3Dot( m1.body1.linearVel, curdir );
			var m2ok = Vector3Dot( m2.body1.linearVel, curdir );

//			Print( m1vel.toFixed(2), '  ', m2vel.toFixed(2), ' ok:', ok.toFixed(2), '            \r');
			
			var f1 = 0, f2 = 0;
			
			if ( d1 < 0 )
				f1 = 1 - ok;
			if ( d2 < 0 )
				f2 = 1 - ok;
			
			if ( ok > 0 ) {

				// stabilization

				if ( m1ok < 0 ) {

					f2 = 0;
					f1 = -m1ok;
				}
				if ( m2ok < 0 ) {
					
					f1 = 0;
					f2 = -m2ok;
				}
					
				if ( m1ok > 0 && m2ok > 0 ) {
				
					if ( m1ok > m2ok )
						f2 += (m1ok - m2ok);
					else
						f1 += (m2ok - m1ok);
				}

			}

			M1Force(f1);
			M2Force(f2);

			yield;
		}
	
	});
	
	this.Aim = function(aim) taskList.unshift(new function() {
		
		for (;;) {

			var len1 = Vector3Length(Vector3Sub(m1.body1.position, aim));
			var len2 = Vector3Length(Vector3Sub(m2.body1.position, aim));
			
			var dist = Vector3Length(Vector3Sub(center.position, aim));
			var dif = (len2 - len1) / 4;
			var speed = Vector3Length(center.linearVel);
			var relSpeed = center.GetRelativeVelocity(aim, []);
			var veldir = -Vector3Dot(Vector3Normalize(center.linearVel), Vector3Normalize(Vector3Sub(center.position, aim)));
			var dir = -Vector3Dot(center.Vector3ToWorld([0,0,1]), Vector3Normalize(Vector3Sub(center.position, aim)));
			var yvel = center.angularVel[1];
			var absyvel = Math.abs(yvel);
			var attn = 1-3/(dist+3); // curves: http://fooplot.com/
			var hdir = Vector3Dot(center.linearVel, center.Vector3ToWorld([1,0,0]));

			var f = 0;
			var p = 0;

			// orient to aim
			if ( dir < 0.95 && absyvel < 1 )
				f += (dif < 0 ? -1 : 1) * 2;
			
		//	Print( 'v:', relSpeed.toFixed(1), '\n' );

			// approach
			if ( dir > 0.9 && absyvel < 0.1 && relSpeed < 2 ) {

				p += 1;
			}
			
			// stabilization
			if ( dir > 0.5 || absyvel > 1 )
				f += (yvel < 0 ? -1 : 1) * absyvel*3;

			// anti-spiral
		//	if ( veldir > -0.2 && veldir < 0.2 && dir > 0.7 )
		//		f += (hdir < 0 ? -1 : 1) * 3;

			if ( relSpeed < 1 && speed > 3 ) {
			
				f += (hdir < 0 ? -1 : 1) * 2;	
			}
			
			m1.maxForce = p + (f < 0 ? -f : 0);
			m2.maxForce = p + (f > 0 ? f : 0);
			
			yield;
		}
	});
		
}

var env3d = new Env3D();

var pod1 = new Pod();

//pod1.Aim([10,0,20]);
//pod1.Stay(2);	// radius

var pause = false;
env3d.AddKeyListener(K_SPACE, function(polarity) { if ( polarity ) pause = !pause });
env3d.AddKeyListener(K_LEFT, function(polarity) { pod1.PushLeft(polarity ? 10 : 0) });
env3d.AddKeyListener(K_RIGHT, function(polarity) { pod1.PushRight(polarity ? 10 : 0) });

env3d.AddKeyListener(K_DOWN, function(polarity) { if ( polarity ) pod1.Stop() });

while ( true ) {
	
	pod1.Step();

	pause || w.Step(10);

	pod1.Draw(env3d);

	env3d.End();
//	Print(env3d.fps.toFixed(0), '    \r');
//	Sleep(10);
}

throw 0;








try {

LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jsio');

LoadModule('jssdl');

LoadModule('jsgraphics');

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


var TextureManager = new function() {
	
	LoadModule('jsimage');
	LoadModule('jsprotex');
	LoadModule('jssvg');

	var textureList = {};
	
	this.LoadSvg = function(name, svgData) {

		var svg = new SVG();
		//svg.onImage = function(href) { return DecodePngImage( new File('img.png').Open(File.RDONLY) ); }
		svg.Write(svgData);
		return textureList[name] = svg.RenderImage(undefined, undefined, 3);
	}
	
	this.Load = function(filename) {
		
		if ( filename in textureList )
			return textureList[filename];
		var texture;
		if ( filename.substr(-4) == '.png' ) {
		
			texture = DecodePngImage(new File(filename).Open(File.RDONLY));
		} else
		if ( filename.substr(-4) == '.jpg' ) {

			texture = DecodeJpegImage(new File(filename).Open(File.RDONLY));
		} else
		if ( filename.substr(-4) == '.svg' ) {
			
			texture = this.LoadSvg(filename, new File(filename).content);
		}
		return textureList[filename] = texture;
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
	
		Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
		Hint(POINT_SMOOTH_HINT, NICEST);

		Enable(POINT_SMOOTH);

//		Enable(BLEND); //Enable alpha blending
//		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
//		Enable(CULL_FACE);
	
//		Enable(TEXTURE_2D);
				
//		Enable(LIGHTING);
//		Enable(LIGHT0);
//		Light(LIGHT0, SPECULAR, [1, 1, 1, 1]);
		
//	Light(LIGHT0, POSITION, [-1,-2, 10]);
//  LightModel(LIGHT_MODEL_AMBIENT, [0.5, 0.5, 0.5, 1]);
//  ShadeModel(SMOOTH);
 
//		Enable( COLOR_MATERIAL );
//		ColorMaterial( FRONT_AND_BACK, EMISSION );
	}
	
	var textureIdList = {};
	this.LoadTexture = function(name) {

		if ( name in textureIdList )
			return textureIdList[name];
		var textureId, texture = TextureManager.Load(name);
		with (Ogl) {

			DefineTextureImage(TEXTURE_2D, undefined, texture);
			textureId = GenTexture();
			BindTexture(TEXTURE_2D, textureId);
//			Print( 'ogl error: ', Ogl.error, '\n' );
		}
		return textureIdList[name] = textureId;
	}
}


var TimeManager = new function() {
	
	var t1, t0 = TimeCounter();

	this.__defineGetter__('lap', function() {

		var t = TimeCounter();
		var prev = t - t1;
		t1 = t;
		return prev;
	});
	this.__defineGetter__('time', function() t1 - t0 );
}



var SceneManager = new function() {

	var perspective = new Transformation();
	var mat = new Transformation();
	var frustumSphere = [];

	var objectList = [];

	this.cameraPosition = [];
	this.cameraTarget = [];

	this.CameraFOV = function(fov) {
	
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.Perspective(fov, 0.1, 100);
		perspective.Load(Ogl);
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
		
		OalListener.position = this.cameraPosition;
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.LoadIdentity();
		var pos = this.cameraPosition;
		var target = this.cameraTarget;
//		Ogl.LookAt(pos[0], pos[1], pos[2], target[0], target[1], target[2], 0,0,1);
		Ogl.LookAt(0,-5,15, 0,0,0 , 0,0,1);
		
		mat.Load(Ogl);
		mat.Product(perspective);
		mat.Invert();
		frustumSphere = FrustumSphere(mat, frustumSphere);

		for each ( object in objectList ) {

			if ( !object.Render )
				continue;

			// - check if object is inside the frustum (see PlanesCollider::PlanesAABBOverlap)
			// - build the frustum by hand

			if ( object.GetBoundarySphere ) {
			
				var bs = object.GetBoundarySphere();
				Vector3Sub(bs, frustumSphere);
				if ( Vector3Length(bs) - bs[3] - frustumSphere[3] > 0 )
					continue;
			}
			
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

		with (Ogl) {
			
			Disable(TEXTURE_2D);
			Normal(0,0,1);
			Translate(-10,-10,0);
			Scale(2,2,0);
			var cx=10, cy=10;
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

	
	this.GetBoundarySphere = function() geom.boundarySphere;
	
	var impactSound = SoundManager.Load('29996__thanvannispen__stone_on_stone_impact13.aif');
//	var ballTextureGlid = DisplayManager.LoadTexture('lines.svg');
	
	var texture = new Texture(128, 128, 3);
	texture.Set([1,0,1]);

	Ogl.Enable(Ogl.TEXTURE_2D);

	var ballTextureGlid = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, ballTextureGlid);
	Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
	
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
		SceneManager.cameraTarget = pos;
	}
	
	this.Render = function() {
		
		with (Ogl) {
		
			var bsphere = geom.boundarySphere;
//			var width = bsphere[3]*2;
			var bbox = geom.aabb;
			var width = bbox[3]-bbox[0]; // 2 * radius
					
			var v = NewVector3(SceneManager.cameraPosition);
			Vector3Sub(v, bsphere);
			var dist = Vector3Length(v);

			var objSize = Ogl.PixelWidthFactor() * width / dist;
//			Print(objSize, '\n');
	
			MultMatrix(geom);
			Color(1,0.5,0.75);

			if ( objSize > 10 ) {
			
				Enable(TEXTURE_2D);
				BindTexture(TEXTURE_2D, ballTextureGlid);
//				Enable(BLEND);
//				BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);				
//				TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
//				TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
//				TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
//				TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [0,0,0,0]);

				DrawTrimesh(glTrimeshId);
			} else {

//				PushAttrib(LIGHTING);
				PushMatrix();
				KeepTranslation();
//				Disable(LIGHTING);
				DrawDisk(width/2);
				PopMatrix();
//				PopAttrib();
			}
		}
	}
	
	this.Destroy = function() {

		geom.Destroy();
		body.Destroy();
	} 
}


SceneManager.CameraFOV(60);
SceneManager.cameraPosition = [-10,-10,10];
SceneManager.cameraTarget = [0,0,0];
SceneManager.Add(new Floor());

var ball = new Ball();
SceneManager.Add(ball);
InputManager.AddEventListener(ball);

CollectGarbage();
var fps = 60;

TimeManager.lap;
var end = false;
while ( !end ) {
	
	InputManager.ProcessEvents();
	SceneManager.Update();
	world.Step(20);
	SceneManager.Render();
	var sleepTime = 1000/fps - TimeManager.lap;
	if ( sleepTime > 0 )
		Sleep(sleepTime);
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