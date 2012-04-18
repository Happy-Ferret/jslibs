var loadModule = host.loadModule;

	//loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  halt();
	//loadModule('jsstd'); exec('../../tests/explodebox.js'); throw 0;
	//loadModule('jsstd'); exec('../../tests/podtest.js'); throw 0;
loadModule('jsstd'); exec('../common/tools.js'); runQATests('jsode'); throw 0;


loadModule('jsstd');
loadModule('jsode');


var space = new Space();
//var floor = new GeomPlane(space);


  /*
	var space = new Space();
	var geom = new GeomPlane(space);
	space.destroy();
	geom.destroy();
*/








throw 0;

try {

loadModule('jsdebug');
loadModule('jsstd');
loadModule('jsio');

loadModule('jssdl');

loadModule('jsgraphics');

//////////////////////////////////////////////////////////////////////////////

loadModule('jsode');

var world = new World();
world.quickStepNumIterations = 20;
world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
world.defaultSurfaceParameters.softERP = 1;
world.defaultSurfaceParameters.softCFM = 0.000001;
world.defaultSurfaceParameters.bounce = 0.5;
world.defaultSurfaceParameters.bounceVel = 2;


var inputManager = new function () {
	
	loadModule('jssdl');	
	var self = this;
	
	var listenerList = [];
	this.addEventListener = function(listener) {
		
		listenerList.push(listener);
	}
	
	showCursor = false;
	grabInput = true;

	while ( pollEvent() ); // clear the event queue

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

			Ogl.viewport(0, 0, w, h);
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
	
	this.processEvents = function() {

		pollEvent(listeners);
	}
}


var trimeshManager = new function() {
	
	loadModule('jstrimesh');
	
	var trimeshList = [];
		
	this.load = function(filename) {

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
			tm.defineVertexBuffer(vertexList);
			tm.defineNormalBuffer(vertexList);

			var faceList = [];
			for each ( var it in mesh[id].face ) {
			
				faceList.push(it[0], it[1], it[2]);
			}
			tm.defineIndexBuffer(faceList);
		}
		
		return trimeshList[filename] = objects;
	}
}


var textureManager = new function() {
	
	loadModule('jsimage');
	loadModule('jsprotex');
	loadModule('jssvg');

	var textureList = {};
	
	this.loadSvg = function(name, svgData) {

		var svg = new SVG();
		//svg.onImage = function(href) { return DecodePngImage( new File('img.png').Open(File.RDONLY) ); }
		svg.write(svgData);
		return textureList[name] = svg.renderImage(undefined, undefined, 3);
	}
	
	this.load = function(filename) {
		
		if ( filename in textureList )
			return textureList[filename];
		var texture;
		if ( filename.substr(-4) == '.png' ) {
		
			texture = decodePngImage(new File(filename).open(File.RDONLY));
		} else
		if ( filename.substr(-4) == '.jpg' ) {

			texture = decodeJpegImage(new File(filename).open(File.RDONLY));
		} else
		if ( filename.substr(-4) == '.svg' ) {
			
			texture = this.loadSvg(filename, new File(filename).content);
		}
		return textureList[filename] = texture;
	}
}


var soundManager = new function() {
	
	loadModule('jsaudio');
	loadModule('jssound');
	Oal.open("Generic Software"); // "Generic Hardware", "Generic Software", "DirectSound3D" (for legacy), "DirectSound", "MMSYSTEM"

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
	
	this.load = function(filename) {

		if ( filename in bufferList )
			return bufferList[filename];
		var decoder = filename.substr(-4) == '.ogg' ? OggVorbisDecoder : SoundFileDecoder;
		var decoder = new decoder( new File(filename).open(File.RDONLY) );
		return bufferList[filename] = new OalBuffer( splitChannels(decoder.read())[0] );
	}

	this.play = function(buffer, x,y,z, gain) {
		
		var src = srcPool.pop();
		src.stop();
		src.buffer = buffer;
		src.position(x,y,z);
		src.gain = gain;
		src.play();
		srcPool.unshift(src);
	}
}	


var displayManager = new function() {

	loadModule('jssdl');
	loadModule('jsgraphics');
	
	glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	glSetAttribute( GL_DOUBLEBUFFER, 1 );
	glSetAttribute( GL_DEPTH_SIZE, 16 );
	setVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN

	with (Ogl) {
	
		hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
		hint(POINT_SMOOTH_HINT, NICEST);

		enable(POINT_SMOOTH);

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
	this.loadTexture = function(name) {

		if ( name in textureIdList )
			return textureIdList[name];
		var textureId, texture = textureManager.load(name);
		with (Ogl) {

			defineTextureImage(TEXTURE_2D, undefined, texture);
			textureId = genTexture();
			bindTexture(TEXTURE_2D, textureId);
//			print( 'ogl error: ', Ogl.error, '\n' );
		}
		return textureIdList[name] = textureId;
	}
}


var timeManager = new function() {
	
	var t1, t0 = timeCounter();

	this.__defineGetter__('lap', function() {

		var t = timeCounter();
		var prev = t - t1;
		t1 = t;
		return prev;
	});
	this.__defineGetter__('time', function() t1 - t0 );
}



var sceneManager = new function() {

	var perspective = new Transformation();
	var mat = new Transformation();
	var frustumSphere = [];

	var objectList = [];

	this.cameraPosition = [];
	this.cameraTarget = [];

	this.cameraFOV = function(fov) {
	
		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.perspective(fov, 0.1, 100);
		perspective.load(Ogl);
	}

	this.add = function(object) {
		
		objectList.push(object);
	}
	
	this.remove = function(object) {
		
		objectList.splice(objectList.lastIndexOf(object), 1);
	}
	
	this.update = function() {

		for each ( object in objectList )
			object.update && object.update();
	}

	this.render = function() {
		
		OalListener.position = this.cameraPosition;
		Ogl.clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.loadIdentity();
		var pos = this.cameraPosition;
		var target = this.cameraTarget;
//		Ogl.LookAt(pos[0], pos[1], pos[2], target[0], target[1], target[2], 0,0,1);
		Ogl.lookAt(0,-5,15, 0,0,0 , 0,0,1);
		
		mat.load(Ogl);
		mat.product(perspective);
		mat.invert();
		frustumSphere = frustumSphere(mat, frustumSphere);

		for each ( object in objectList ) {

			if ( !object.render )
				continue;

			// - check if object is inside the frustum (see PlanesCollider::PlanesAABBOverlap)
			// - build the frustum by hand

			if ( object.getBoundarySphere ) {
			
				var bs = object.getBoundarySphere();
				vector3Sub(bs, frustumSphere);
				if ( vector3Length(bs) - bs[3] - frustumSphere[3] > 0 )
					continue;
			}
			
			Ogl.pushMatrix();
			object.render();
			Ogl.popMatrix();
		}
		
		glSwapBuffers();
	}
}


function Floor() {
	
	var self = this;
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.render = function() {

		with (Ogl) {
			
			disable(TEXTURE_2D);
			normal(0,0,1);
			translate(-10,-10,0);
			scale(2,2,0);
			var cx=10, cy=10;
			begin(QUADS);
			for ( var x = 0; x < cx; x++ )
				for ( var y = 0; y < cy; y++ ) {
					if ( (x + y) % 2 )
						color(0,0,0.5);
					else
						color(0.9,0.8,0.7);
					vertex(x,y);
					vertex(x+1,y);
					vertex(x+1,y+1);
					vertex(x, y+1);
				}
			end();
		}
	}	
}



function Ball() {
	
	var self = this;
	
	var trimesh = trimeshManager.load('sphere.json').sphere;
	var glTrimeshId = Ogl.loadTrimesh(trimesh);
	var geom = new GeomTrimesh(trimesh, world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = [10,-5,1];

	
	this.getBoundarySphere = function() geom.boundarySphere;
	
	var impactSound = soundManager.load('29996__thanvannispen__stone_on_stone_impact13.aif');
//	var ballTextureGlid = DisplayManager.LoadTexture('lines.svg');
	
	var texture = new Texture(128, 128, 3);
	texture.Set([1,0,1]);

	Ogl.enable(Ogl.TEXTURE_2D);

	var ballTextureGlid = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, ballTextureGlid);
	Ogl.defineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
	
	geom.impact = function(geom, geom2, vel, px, py, pz) {

		if ( vel > 5 )
			soundManager.play(impactSound, px, py, pz, vel/100);
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
	
	this.update = function() {
		
		var pos = body.position;
		sceneManager.cameraTarget = pos;
	}
	
	this.render = function() {
		
		with (Ogl) {
		
			var bsphere = geom.boundarySphere;
//			var width = bsphere[3]*2;
			var bbox = geom.aabb;
			var width = bbox[3]-bbox[0]; // 2 * radius
					
			var v = newVector3(sceneManager.cameraPosition);
			vector3Sub(v, bsphere);
			var dist = vector3Length(v);

			var objSize = Ogl.pixelWidthFactor() * width / dist;
//			print(objSize, '\n');
	
			multMatrix(geom);
			color(1,0.5,0.75);

			if ( objSize > 10 ) {
			
				enable(TEXTURE_2D);
				bindTexture(TEXTURE_2D, ballTextureGlid);
//				Enable(BLEND);
//				BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);				
//				TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
//				TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
//				TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
//				TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [0,0,0,0]);

				drawTrimesh(glTrimeshId);
			} else {

//				PushAttrib(LIGHTING);
				pushMatrix();
				keepTranslation();
//				Disable(LIGHTING);
				drawDisk(width/2);
				popMatrix();
//				PopAttrib();
			}
		}
	}
	
	this.destroy = function() {

		geom.destroy();
		body.destroy();
	} 
}


sceneManager.cameraFOV(60);
sceneManager.cameraPosition = [-10,-10,10];
sceneManager.cameraTarget = [0,0,0];
sceneManager.add(new Floor());

var ball = new Ball();
sceneManager.add(ball);
inputManager.addEventListener(ball);

collectGarbage();
var fps = 60;

timeManager.lap;
var end = false;
while ( !end ) {
	
	inputManager.processEvents();
	sceneManager.update();
	world.step(20);
	sceneManager.render();
	var sleepTime = 1000/fps - timeManager.lap;
	if ( sleepTime > 0 )
		sleep(sleepTime);
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
j.setAxis(0, 1, [0,0,1]);
j.maxForce = 1;
j.velocity = 100;
*/


/*
		loadIdentity();
		translate(10,10,10);
		lookAt(10, 10, 15, 0,0,0, 0,0,1);
		print('gl '); printGlMatrix();

		var t = new Transformation(undefined);
		t.translate(10,10,10);
		t.lookAt(10, 10, 15, 0,0,0, 0,0,1);
		loadMatrix(t);
		print('tr '); printGlMatrix();
		print('\n'); 
*/

} catch(ex) {
	
	print( ex.fileName+':'+ex.lineNumber+' '+ex, '\n' );
	halt();
}




function point(x,y,z) {

	with (Ogl) {
	
		pushAttrib(LIGHTING | DEPTH_TEST);
		disable(LIGHTING);
		disable(DEPTH_TEST);
		enable(POINT_SMOOTH);
		pointSize(3);
		begin(POINTS);
		vertex(x,y,z);
		end();
		popAttrib();
	}
}


function axis(x,y,z) {

	with (Ogl) {
	
		pushAttrib(LIGHTING | DEPTH_TEST);
		disable(LIGHTING);
		disable(DEPTH_TEST);
		lineWidth(2);
		begin(LINES);
		color(1,0,0);
		vertex(x,y,z);
		vertex(x+1,y,z);
		color(0,1,0);
		vertex(x,y,z);
		vertex(x,y+1,z);
		color(0,0,1);
		vertex(x,y,z);
		vertex(x,y,z+1);
		end();
		popAttrib();
	}
}


function cube() {

	with (Ogl) {
		begin(QUADS);		// Draw The Cube Using quads

		color(1,0,0);
		vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Right)
		vertex( 0.5, 0.5, 0.5);	// Top Left Of The Quad (Right)
		vertex( 0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Right)
		vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Right)

		color(0.5,0,0);
		vertex(-0.5, 0.5, 0.5);	// Top Right Of The Quad (Left)
		vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Left)
		vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Left)
		vertex(-0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Left)

		color(0,1,0);
		vertex( 0.5, 0.5,-0.5);	// Top Right Of The Quad (Top)
		vertex(-0.5, 0.5,-0.5);	// Top Left Of The Quad (Top)
		vertex(-0.5, 0.5, 0.5);	// Bottom Left Of The Quad (Top)
		vertex( 0.5, 0.5, 0.5);	// Bottom Right Of The Quad (Top)

		color(0,0.5,0);
		vertex( 0.5,-0.5, 0.5);	// Top Right Of The Quad (Bottom)
		vertex(-0.5,-0.5, 0.5);	// Top Left Of The Quad (Bottom)
		vertex(-0.5,-0.5,-0.5);	// Bottom Left Of The Quad (Bottom)
		vertex( 0.5,-0.5,-0.5);	// Bottom Right Of The Quad (Bottom)

		color(0,0,1);
		vertex( 0.5, 0.5, 0.5);	// Top Right Of The Quad (Front)
		vertex(-0.5, 0.5, 0.5);	// Top Left Of The Quad (Front)
		vertex(-0.5,-0.5, 0.5);	// Bottom Left Of The Quad (Front)
		vertex( 0.5,-0.5, 0.5);	// Bottom Right Of The Quad (Front)

		color(0,0,0.5);
		vertex( 0.5,-0.5,-0.5);	// Top Right Of The Quad (Back)
		vertex(-0.5,-0.5,-0.5);	// Top Left Of The Quad (Back)
		vertex(-0.5, 0.5,-0.5);	// Bottom Left Of The Quad (Back)
		vertex( 0.5, 0.5,-0.5);	// Bottom Right Of The Quad (Back)
		end();
	}
}