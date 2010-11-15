LoadModule('jsstd');
LoadModule('jsio');


function Env3D() {
	
	LoadModule('jssdl');
	LoadModule('jsgraphics');
	
	var _this = this;
	
	function InitVideo(width, height) {
	
	//	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
		GlSetAttribute( GL_DOUBLEBUFFER, 1 );
		GlSetAttribute( GL_DEPTH_SIZE, 16 );
		GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
		if ( width && height )
			SetVideoMode( width, height, 32, OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
		else
			SetVideoMode( desktopWidth, desktopHeight, 32, OPENGL | RESIZABLE | FULLSCREEN ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
		Ogl.Hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
		Ogl.Hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
		Ogl.Viewport(0,0,videoWidth,videoHeight);
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.Perspective(60, undefined, 1, 100);
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.ClearColor(0.2, 0.1, 0.4, 1);
		Ogl.Enable(Ogl.DEPTH_TEST);
		Ogl.Enable(Ogl.BLEND);
		Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
	}
	
	InitVideo(640, 480);
	
	
	this.DrawGrid = function() {

		if ( !arguments.callee.geometry ) {
			
			arguments.callee.geometry = Ogl.NewList(false);
			
			var len = 20;
			var max = Math.pow(1.5, len);

			Ogl.Begin(Ogl.LINES);
			for ( var i = 0; i <= len; i++ ) {
			
				Ogl.Color(1, 1, 1, 0.5-Math.abs(i/len)/2);
				
				var powi = Math.pow(1.5, i);
				Ogl.Vertex(-max, powi, 0); Ogl.Vertex(max, powi, 0);
				Ogl.Vertex(powi, -max, 0); Ogl.Vertex(powi, max, 0);
			}
			Ogl.End();
			Ogl.EndList();
		}
		Ogl.CallList(arguments.callee.geometry);
	}
	
	this.Draw3DAxis = function() {
		
		var size = 1;
		with (Ogl) {
			Disable(DEPTH_TEST);
			Begin(LINES);
			Color( 1,0,0, 0.75 ); Vertex( 0,0,0 ); Vertex( size,0,0 );
			Color( 0,1,0, 0.75 ); Vertex( 0,0,0 ); Vertex( 0,size,0 );
			Color( 0,0,1, 0.75 ); Vertex( 0,0,0 ); Vertex( 0,0,size );
			End();
			Enable(DEPTH_TEST);
		}
	}
	
	this.Draw3DArrow = function() {

		if ( !arguments.callee.geometry ) {

			arguments.callee.geometry = Ogl.NewList(false);

			Ogl.Color(1, 1, 1, 0.75);

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0,  1.0);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0,  1.0);
			Ogl.End();

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0, -1.0);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0,  1.0,  1.0);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0,  1.0,  1.0);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0, -1.0);
			Ogl.End();

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0, -1.0, -1.0);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0,  1.0, -1.0);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0,  1.0, -1.0);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0, -1.0, -1.0);
			Ogl.End();

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex(-1.0, -1.0, -1.0);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex( 1.0, -1.0, -1.0);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
			Ogl.End();

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0, -1.0, -1.0);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0, -1.0);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex( 1.0,  1.0,  1.0);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
			Ogl.End();

			Ogl.Begin(Ogl.QUADS);
			Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0, -1.0, -1.0);
			Ogl.TexCoord(1.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
			Ogl.TexCoord(1.0, 1.0); Ogl.Vertex(-1.0,  1.0,  1.0);
			Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0, -1.0);
			Ogl.End();

			Ogl.EndList();
		}

		Ogl.CallList(arguments.callee.geometry);
	}
	
	var trails = {};
	
	this.AddTrail = function(index, pos) {
		
		var t = trails[index];
		if ( !t )
			return;
		if ( t.length > 10000 )
			t.shift();
		var len = t.length;
		if ( len < 2 || Vector3Length(t[len-1], t[len-2]) > 2 )
			t.push(pos);
		else
			t[len-1] = pos;
	}
	
	this.DrawTrail = function(index) {
	
		var t = trails[index] ? trails[index] : (trails[index]=[]);
		var len = t.length;
		Ogl.Begin(Ogl.LINE_STRIP); // Ogl.LINE_STRIP
		for ( var i = 0; i < len; ++i ) {
			
			Ogl.Color( 1,0.5,0, 0.25+(i/len)/2 );
			Ogl.Vertex( t[i][0], t[i][1], t[i][2] );
		}
		Ogl.End();
	}
	
	var eye = [Math.PI, -Math.PI/8, 5], dst = [0,0,0];

	var t;
	this.fps = 0;
	this.frame = 0;

	var keyListeners = {};
	this.AddKeyListener = function(key, fct) {
		
		var kl = keyListeners[key] || (keyListeners[key] = []);
		kl.push(fct);
	}	

	this.RemoveKeyListener = function(key, fct) {
		
		var kl = keyListeners[key] || (keyListeners[key] = []);
		var pos = kl.lastIndexOf(fct);
		if ( pos != -1 )
			kl.splice(pos, 1);
	}	


	var eventHandler = {

		onVideoResize: function(w,h) {

			Ogl.Viewport(0, 0, w, h);
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
					//Halt();
					throw 0;
					break;
			}
		},
		
		onMouseButtonDown: function(button) {
			
			if ( button == BUTTON_RIGHT ) {
			
				showCursor = false;
				grabInput = true;
			}
			
			if ( button == BUTTON_WHEELUP )
				(!GetKeyState(K_LCTRL) ? eye:dst )[2] -= 1/2;
			if ( button == BUTTON_WHEELDOWN )
				(!GetKeyState(K_LCTRL) ? eye:dst )[2] += 1/2;
		},

		onMouseButtonUp: function(button) {
		
			if ( button == BUTTON_RIGHT ) {
			
				grabInput = false;
				showCursor = true;
			}
		},
		
		onMouseMotion: function(x, y, dx, dy, state) {
			
			if ( state & BUTTON_RMASK ) {
			
				(!GetKeyState(K_LCTRL) ? eye:dst )[0] += dx/500;
				(!GetKeyState(K_LCTRL) ? eye:dst )[1] -= dy/500;
			}
		}
	};

	this.Begin = function() {
		
		t = TimeCounter();
		this.frame++;
	
		while( PollEvent(eventHandler) );
		
		Ogl.LoadIdentity();
		
		var e = [0,0,0];
		var d = [0,0,0];
		
		var dist = Math.pow(1.5, eye[2]) * 10;
		var z = Math.abs(Math.cos(eye[1]));
		e[2] += Math.sin(-eye[1]) * dist;
		e[1] += Math.cos(eye[0]) * z * dist;
		e[0] += Math.sin(eye[0]) * z * dist;

//		Ogl.LookAt(0,0,0, e[0], e[1], e[2], 0,0,1);
		Ogl.LookAt(e[0], e[1], e[2], 0,0,0, 0,0,1);

		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	}
	
	this.End = function() {
	
		_this.fps = 1000/(TimeCounter() - t);
		GlSwapBuffers();
	}
}



var textureId;
function DisplayTexture( texture ) {

	if ( !textureId )
		textureId = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);

	Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);

	Ogl.MatrixMode(Ogl.PROJECTION);
	Ogl.LoadIdentity();
//	Ogl.Ortho(0, width, 0, height, -1, 1);
	Ogl.MatrixMode(Ogl.MODELVIEW);
	Ogl.LoadIdentity();

	Ogl.Enable(Ogl.TEXTURE_2D);
	Ogl.Disable(Ogl.DEPTH_TEST);

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);
	
//	Ogl.Translate(0,0,-1);
	
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1,1);
	Ogl.TexCoord(1, 0); Ogl.Vertex(1,1);
	Ogl.TexCoord(1, 1); Ogl.Vertex(1,-1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();
	
	GlSwapBuffers();
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

function DumpVector(v) {

	Print( v[0].toFixed(1), '  ', v[1].toFixed(1), '  ', v[2].toFixed(1), '\n' );
}

function DumpMatrix(m) {
    
	for (var y = 0; y < 4; ++y) {
		Print('[ ' );
		for (var x = 0; x < 4; ++x)
			Print( m[x+y*4].toFixed(3) + '  ' );
		Print(']\n' );
	}
	Print('\n' );
}

function Dump(/*...*/) {
	
	for ( var i = 0; i < arguments.length; i++ )
		Print(uneval(arguments[i]), '  ');
	Print('\n');
}

function RunLocalQAFile() {
	
	LoadModule('jsio');
	global.QA = { __noSuchMethod__:function(id, args) {
		Print( id, ':', uneval(args), '\n' )
	} };
	Exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');
	throw 0;
}


function RunQATests( argStr ) {

	LoadModule('jsio');
	currentDirectory += '/../..';
	global.arguments = Array.concat('qa.js', argStr.split(' '));
	Exec(global.arguments[0], false);
	throw 0;
}

function RunJsircbot( withDebuggerEnabled ) {

	LoadModule('jsio');
	LoadModule('jsdebug');
	withDebuggerEnabled && Exec('../jsdebug/debugger.js', false);
	currentDirectory += '/../../../jsircbot';
	global.arguments[1] = 'my_configuration.js'; // simulate: jshost main.js my_configuration.js
	Print( 'RunJsircbot arguments: '+uneval(global.arguments), '\n' );
	Exec('main.js', false);
	throw 0;
}

var FakeQAApi = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };


/* z-pass
		
		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 0.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 0.5, 0.5, 0.5, 1);

		renderCallback(6);

		Ogl.Enable(Ogl.CULL_FACE);

		Ogl.ColorMask(false);
		Ogl.DepthMask(false);
		
		Ogl.ShadeModel(Ogl.FLAT);
		Ogl.Disable(Ogl.LIGHTING);

		Ogl.Clear(Ogl.STENCIL_BUFFER_BIT);
		Ogl.Enable(Ogl.STENCIL_TEST);
		Ogl.StencilFunc(Ogl.ALWAYS, 0, -1);

		shadowVolumeProgram.On();
		
		Ogl.DepthFunc(Ogl.LESS);
		
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.INCR);
		Ogl.CullFace(Ogl.BACK);
		var list = Ogl.NewList();
		renderCallback(3); // render occluders shape only
		Ogl.EndList()
	
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.DECR);
		Ogl.CullFace(Ogl.FRONT);
		Ogl.CallList(list);
		Ogl.DeleteList(list);

		shadowVolumeProgram.Off();
		
		Ogl.ShadeModel(Ogl.SMOOTH);
		Ogl.Enable(Ogl.LIGHTING);
		
		Ogl.ColorMask(true);
		Ogl.DepthMask(true);

		Ogl.DepthFunc(Ogl.EQUAL);
		Ogl.StencilFunc(Ogl.EQUAL, 0, -1);
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.KEEP);

		Ogl.CullFace(Ogl.BACK);

		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 1, 1, 1, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1, 1, 1, 1);
				
		renderCallback(6);
		
		Ogl.Disable(Ogl.STENCIL_TEST);
		Ogl.DepthFunc(Ogl.LEQUAL);


*/