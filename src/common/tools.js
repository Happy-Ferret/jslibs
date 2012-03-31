loadModule('jsstd');
loadModule('jsio');


function env3D() {
	
	loadModule('jssdl');
	loadModule('jsgraphics');
	
	var _this = this;
	
	function initVideo(width, height) {
	
	//	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
		glSetAttribute( GL_DOUBLEBUFFER, 1 );
		glSetAttribute( GL_DEPTH_SIZE, 16 );
//		GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
		if ( width && height )
			setVideoMode( width, height, 32, OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
		else
			setVideoMode( desktopWidth, desktopHeight, 32, OPENGL | RESIZABLE | FULLSCREEN ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
		Ogl.hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
		Ogl.hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
		Ogl.viewport(0,0,videoWidth,videoHeight);
		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.perspective(60, undefined, 1, 10000);
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.clearColor(0.2, 0.1, 0.4, 1);
		Ogl.enable(Ogl.DEPTH_TEST);
		Ogl.enable(Ogl.BLEND);
		Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
	}
	
	initVideo(800, 600);
	
	
	this.drawGrid = function() {

		if ( !arguments.callee.geometry ) {
			
			arguments.callee.geometry = Ogl.newList(false);
			
			var len = 20;
			var max = Math.pow(1.5, len);

			Ogl.begin(Ogl.LINES);
			for ( var i = 0; i <= len; i++ ) {
			
				Ogl.color(1, 1, 1, 0.5-Math.abs(i/len)/2);
				
				var powi = Math.pow(1.5, i);
				Ogl.vertex(-max, powi, 0); Ogl.vertex(max, powi, 0);
				Ogl.vertex(powi, -max, 0); Ogl.vertex(powi, max, 0);
			}
			Ogl.end();
			Ogl.endList();
		}
		Ogl.callList(arguments.callee.geometry);
	}
	
	this.draw3DAxis = function() {
		
		var size = 1;
		with (Ogl) {
			disable(DEPTH_TEST);
			begin(LINES);
			color( 1,0,0, 0.75 ); vertex( 0,0,0 ); vertex( size,0,0 );
			color( 0,1,0, 0.75 ); vertex( 0,0,0 ); vertex( 0,size,0 );
			color( 0,0,1, 0.75 ); vertex( 0,0,0 ); vertex( 0,0,size );
			end();
			enable(DEPTH_TEST);
		}
	}
	
	this.draw3DArrow = function() {

		if ( !arguments.callee.geometry ) {

			arguments.callee.geometry = Ogl.newList(false);

			Ogl.color(1, 1, 1, 0.75);

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0,  1.0);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0,  1.0);
			Ogl.end();

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0, -1.0);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0,  1.0,  1.0);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0,  1.0,  1.0);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0, -1.0);
			Ogl.end();

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0, -1.0, -1.0);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0,  1.0, -1.0);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0,  1.0, -1.0);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0, -1.0, -1.0);
			Ogl.end();

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex(-1.0, -1.0, -1.0);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex( 1.0, -1.0, -1.0);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
			Ogl.end();

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0, -1.0, -1.0);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0, -1.0);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex( 1.0,  1.0,  1.0);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
			Ogl.end();

			Ogl.begin(Ogl.QUADS);
			Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0, -1.0, -1.0);
			Ogl.texCoord(1.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
			Ogl.texCoord(1.0, 1.0); Ogl.vertex(-1.0,  1.0,  1.0);
			Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0, -1.0);
			Ogl.end();

			Ogl.endList();
		}

		Ogl.callList(arguments.callee.geometry);
	}
	
	var trails = {};
	
	this.addTrail = function(index, pos) {
		
		var t = trails[index];
		if ( !t )
			return;
		if ( t.length > 10000 )
			t.shift();
		var len = t.length;
		if ( len < 2 || vec3Length(t[len-1], t[len-2]) > 2 )
			t.push(pos);
		else
			t[len-1] = pos;
	}
	
	this.drawTrail = function(index) {
	
		var t = trails[index] ? trails[index] : (trails[index]=[]);
		var len = t.length;
		Ogl.begin(Ogl.LINE_STRIP); // Ogl.LINE_STRIP
		for ( var i = 0; i < len; ++i ) {
			
			Ogl.color( 1,0.5,0, 0.25+(i/len)/2 );
			Ogl.vertex( t[i][0], t[i][1], t[i][2] );
		}
		Ogl.end();
	}
	
	var eye = [Math.PI, -Math.PI/8, 5], dst = [0,0,0];

	var t;
	this.fps = 0;
	this.frame = 0;

	var keyListeners = {};
	this.addKeyListener = function(key, fct) {
		
		var kl = keyListeners[key] || (keyListeners[key] = []);
		kl.push(fct);
	}	

	this.removeKeyListener = function(key, fct) {
		
		var kl = keyListeners[key] || (keyListeners[key] = []);
		var pos = kl.lastIndexOf(fct);
		if ( pos != -1 )
			kl.splice(pos, 1);
	}
	
	this.exit = function() {
		
		throw 0;
	}


	var eventHandler = {

		onVideoResize: function(w,h) {

			Ogl.viewport(0, 0, w, h);
		},
	
		onKeyUp: function(key, mod) {

			var kl = keyListeners[key];
			if ( kl )
				for ( var i = kl.length-1; i >= 0; --i )
					kl[i](false, key, mod);
		},
		
		onKeyDown: function(key, mod) {
			
			var kl = keyListeners[key];
			if ( kl )
				for ( var i = kl.length-1; i >= 0; --i )
					kl[i](true, key, mod);
						
			switch (key) {
				
				case K_SPACE:

//					InitVideo();
					break;
				case K_ESCAPE:
					//halt();
					_this.exit();
					break;
			}
		},
		
		onMouseButtonDown: function(button) {
			
			if ( button == BUTTON_RIGHT ) {
			
				showCursor = false;
				grabInput = true;
			}
			
			if ( button == BUTTON_WHEELUP )
				(!getKeyState(K_LCTRL) ? eye:dst )[2] -= 1/2;
			if ( button == BUTTON_WHEELDOWN )
				(!getKeyState(K_LCTRL) ? eye:dst )[2] += 1/2;
		},

		onMouseButtonUp: function(button) {
		
			if ( button == BUTTON_RIGHT ) {
			
				grabInput = false;
				showCursor = true;
			}
		},
		
		onMouseMotion: function(x, y, dx, dy, state) {
			
			if ( state & BUTTON_RMASK ) {
			
				(!getKeyState(K_LCTRL) ? eye:dst )[0] += dx/500;
				(!getKeyState(K_LCTRL) ? eye:dst )[1] -= dy/500;
			}
		}
	};

	this.begin = function() {
		
		t = timeCounter();
		this.frame++;
	
		while( pollEvent(eventHandler) );
		
		Ogl.loadIdentity();
		
		var e = [0,0,0];
		var d = [0,0,0];
		
		var dist = Math.pow(1.5, eye[2]) * 10;
		var z = Math.abs(Math.cos(eye[1]));
		e[2] += Math.sin(-eye[1]) * dist;
		e[1] += Math.cos(eye[0]) * z * dist;
		e[0] += Math.sin(eye[0]) * z * dist;

//		Ogl.LookAt(0,0,0, e[0], e[1], e[2], 0,0,1);
		Ogl.lookAt(e[0], e[1], e[2], 0,0,0, 0,0,1);

		Ogl.clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	}
	
	this.end = function() {
	
		_this.fps = 1000/(timeCounter() - t);
		glSwapBuffers();
	}
}



var textureId;
function displayTexture( texture ) {

	if ( !textureId )
		textureId = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, textureId);

	Ogl.defineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);

	Ogl.matrixMode(Ogl.PROJECTION);
	Ogl.loadIdentity();
//	Ogl.Ortho(0, width, 0, height, -1, 1);
	Ogl.matrixMode(Ogl.MODELVIEW);
	Ogl.loadIdentity();

	Ogl.enable(Ogl.TEXTURE_2D);
	Ogl.disable(Ogl.DEPTH_TEST);

	Ogl.clear(Ogl.COLOR_BUFFER_BIT);
	
//	Ogl.Translate(0,0,-1);
	
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0, 0); Ogl.vertex(-1,1);
	Ogl.texCoord(1, 0); Ogl.vertex(1,1);
	Ogl.texCoord(1, 1); Ogl.vertex(1,-1);
	Ogl.texCoord(0, 1); Ogl.vertex(-1,-1);
	Ogl.end();
	
	glSwapBuffers();
}



var textureId;
function displayImage( image ) {

	setVideoMode( image.width,image.height, 32, OPENGL | RESIZABLE );
	Ogl.hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
	Ogl.viewport(0,0,image.width,image.height);

	if ( !textureId )
		textureId = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, textureId);

	Ogl.defineTextureImage(Ogl.TEXTURE_2D, undefined, image);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.NEAREST);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.NEAREST);

	Ogl.matrixMode(Ogl.PROJECTION);
	Ogl.loadIdentity();
	Ogl.matrixMode(Ogl.MODELVIEW);
	Ogl.loadIdentity();

	Ogl.enable(Ogl.TEXTURE_2D);
	Ogl.disable(Ogl.DEPTH_TEST);

	Ogl.clear(Ogl.COLOR_BUFFER_BIT);

	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0, 0); Ogl.vertex(-1,1);
	Ogl.texCoord(1, 0); Ogl.vertex(1,1);
	Ogl.texCoord(1, 1); Ogl.vertex(1,-1);
	Ogl.texCoord(0, 1); Ogl.vertex(-1,-1);
	Ogl.end();
	
	glSwapBuffers();
	
	var end = false;
	var listeners = {
	
		onKeyDown:function(sym, mod, chr) {
		
			end = true;
		}
	}
	
	while ( !end )
		processEvents(SDLEvents(listeners));
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

function dumpVector(v) {

	print( v[0].toFixed(1), '  ', v[1].toFixed(1), '  ', v[2].toFixed(1), '\n' );
}

function dumpMatrix(m) {
    
	for (var y = 0; y < 4; ++y) {
		print('[ ' );
		for (var x = 0; x < 4; ++x)
			print( m[x+y*4].toFixed(3) + '  ' );
		print(']\n' );
	}
	print('\n' );
}

function dump(/*...*/) {
	
	for ( var i = 0; i < arguments.length; i++ )
		print(uneval(arguments[i]), '  ');
	print('\n');
}

function runLocalQAFile() {
	
	loadModule('jsio');
	global.QA = { __noSuchMethod__:function(id, args) {
		print( id, ':', uneval(args), '\n' )
	} };
	exec( /[^/\\]+$/.exec(currentDirectory)[0] + '_qa.js');
	throw 0;
}

function runSavedQAFile(fileName) {

	loadModule('jsstd');
	loadModule('jsio');
	var itemList = eval('('+new File(fileName).content+')');
	var qaapi = { __noSuchMethod__:function() {} };
	for ( var i = 0; i < itemList.length; i++ )
		itemList[i].func(qaapi);
}


function runQATests( argStr ) {

	loadModule('jsio');
	currentDirectory += '/../common';
	host.arguments = Array.concat('qa.js', argStr.split(' '));
	exec(host.arguments[0], false);
	throw 0;
}

function runJsircbot( withDebuggerEnabled ) {

	loadModule('jsio');
	loadModule('jsdebug');
	withDebuggerEnabled && exec('../jsdebug/debugger.js', false);
	currentDirectory += '/../../../jsircbot';
	host.arguments[1] = 'my_configuration.js'; // simulate: jshost main.js my_configuration.js
	print( 'RunJsircbot arguments: '+uneval(host.arguments), '\n' );
	exec('main.js', false);
	throw 0;
}

var fakeQAApi = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args).substr(0,32), '\n' ) } };


/* z-pass
		
		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 0.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 0.5, 0.5, 0.5, 1);

		renderCallback(6);

		Ogl.enable(Ogl.CULL_FACE);

		Ogl.colorMask(false);
		Ogl.depthMask(false);
		
		Ogl.shadeModel(Ogl.FLAT);
		Ogl.disable(Ogl.LIGHTING);

		Ogl.clear(Ogl.STENCIL_BUFFER_BIT);
		Ogl.enable(Ogl.STENCIL_TEST);
		Ogl.stencilFunc(Ogl.ALWAYS, 0, -1);

		shadowVolumeProgram.on();
		
		Ogl.depthFunc(Ogl.LESS);
		
		Ogl.stencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.INCR);
		Ogl.cullFace(Ogl.BACK);
		var list = Ogl.newList();
		renderCallback(3); // render occluders shape only
		Ogl.endList()
	
		Ogl.stencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.DECR);
		Ogl.cullFace(Ogl.FRONT);
		Ogl.callList(list);
		Ogl.deleteList(list);

		shadowVolumeProgram.off();
		
		Ogl.shadeModel(Ogl.SMOOTH);
		Ogl.enable(Ogl.LIGHTING);
		
		Ogl.colorMask(true);
		Ogl.depthMask(true);

		Ogl.depthFunc(Ogl.EQUAL);
		Ogl.stencilFunc(Ogl.EQUAL, 0, -1);
		Ogl.stencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.KEEP);

		Ogl.cullFace(Ogl.BACK);

		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 1, 1, 1, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1, 1, 1, 1);
				
		renderCallback(6);
		
		Ogl.disable(Ogl.STENCIL_TEST);
		Ogl.depthFunc(Ogl.LEQUAL);


*/