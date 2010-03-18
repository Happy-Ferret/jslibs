LoadModule('jsstd');
LoadModule('jsio');


function UI(currentWidth, currentHeight) {
	
	var _this = this;

	LoadModule('jssdl');
	LoadModule('jsgraphics');
	LoadModule('jsfont');
	LoadModule('jsoglft');
	//LoadModule('jsimage');
	LoadModule('jsprotex');
	//LoadModule('jstrimesh');
	
	var f3d = new Font3D(new Font('c:\\windows\\fonts\\arial.ttf'), Font3D.OUTLINE, 9);
	var f2d = new Font3D(new Font('c:\\windows\\fonts\\arial.ttf'), Font3D.GRAYSCALE, 9); // TRANSLUCENT

	var defaultVideoMode = OPENGL | RESIZABLE;

	unicodeKeyboardTranslation = true;
	keyRepeatDelay = 300;
	keyRepeatInterval = 50;
	maxFPS = 60;
	
	currentWidth = currentWidth || desktopWidth / 4;
	currentHeight = currentHeight || desktopHeight / 4;
	
	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DEPTH_SIZE, 16);
	GlSetAttribute( GL_STENCIL_SIZE, 8 );
	GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
	
	SetVideoMode(currentWidth, currentHeight, undefined, defaultVideoMode);
	
//	Print( 'OpenGL version ', Ogl.GetString(Ogl.VERSION), '\n' );
	
//	Print( Ogl.extensions.indexOf('power_of_two'), '\n' ); throw 0;
	
	Ogl.Hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
	Ogl.Hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
	Ogl.Hint(Ogl.POLYGON_SMOOTH_HINT, Ogl.NICEST);

	Ogl.LightModel(Ogl.LIGHT_MODEL_LOCAL_VIEWER, 1); // see. http://gregs-blog.com/2007/12/21/theres-nothing-wrong-with-opengls-specular-lighting/

	Ogl.PixelStore( Ogl.UNPACK_ALIGNMENT, 1 );

	Ogl.Enable(Ogl.CULL_FACE);  // default: Ogl.FrontFace(Ogl.CCW);  Ogl.CullFace(Ogl.BACK);
	
	Ogl.Enable(Ogl.DEPTH_TEST);


//	Ogl.Enable(Ogl.TEXTURE_2D);
//	Ogl.ShadeModel(Ogl.SMOOTH);
//	Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, [1.0, 1.0, 1.0, 1.0]);
//	Ogl.Material(Ogl.FRONT, Ogl.SHININESS, 50);
//	Ogl.ColorMaterial ( Ogl.FRONT_AND_BACK, Ogl.AMBIENT_AND_DIFFUSE ) ;
//	Ogl.Enable( Ogl.COLOR_MATERIAL );
//	Ogl.Enable(Ogl.BLEND);
//	Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);

	
	var lightPos;// = [0,10,10, 1];
	var lightDir = [0,0,0];
	this.SetLight = function(pos, dir) {
		
		if ( !lightPos ) {
			
			Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1, 1, 1, 1);
			//Ogl.Light(Ogl.LIGHT0, Ogl.CONSTANT_ATTENUATION, 0.1);
			//Ogl.Light(Ogl.LIGHT0, Ogl.LINEAR_ATTENUATION, 0.003);
			//Ogl.Light(Ogl.LIGHT0, Ogl.SPOT_EXPONENT, 0);
			//Ogl.Light(Ogl.LIGHT0, Ogl.SPOT_CUTOFF, 180);
			Ogl.Enable(Ogl.LIGHTING);
			Ogl.Enable(Ogl.LIGHT0);
		}

		if ( pos ) {
		
			lightPos = pos;
			Ogl.Light(Ogl.LIGHT0, Ogl.POSITION, pos);
		}
		if ( dir ) {
			
			lightDir = dir;
			Ogl.Light(Ogl.LIGHT0, Ogl.SPOT_DIRECTION, dir);
		}
	}

	this.RenderWithShadows = function( renderCallback, plane ) {
		
		// see http://www.opengl.org/resources/code/samples/mjktips/TexShadowReflectLight.html

		Ogl.Clear(Ogl.STENCIL_BUFFER_BIT);
		Ogl.PolygonOffset(-2, -1); // set the scale and units used to calculate depth values.
		
		// Draw the floor with stencil value 3.  This helps us only draw the shadow once per floor pixel (and only on the floor pixels).
		Ogl.Enable(Ogl.STENCIL_TEST);
		Ogl.StencilFunc(Ogl.ALWAYS, 3, -1);
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.REPLACE);

		renderCallback(4);
		
		// Render the projected shadow.
		Ogl.StencilFunc(Ogl.LESS, 2, -1);  /* draw if ==1 */
		Ogl.StencilOp(Ogl.REPLACE, Ogl.REPLACE, Ogl.REPLACE);
		
		// To eliminate depth buffer artifacts, we use polygon offset to raise the depth of the projected shadow slightly so that it does not depth buffer alias with the floor.
		Ogl.Enable(Ogl.POLYGON_OFFSET_FILL);
		
      // Render 50% black shadow color on top of whatever the floor appareance is.
      Ogl.Enable(Ogl.BLEND);
      Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
      
      Ogl.Disable(Ogl.LIGHTING); // Force the 50% black.
      Ogl.Color(0.0, 0.0, 0.0, 0.6);
      Ogl.PushMatrix();
		// Project the shadow.
		Ogl.MultMatrix(ShadowMatrix(plane, lightPos));

		renderCallback(3);

		Ogl.PopMatrix();
      Ogl.Enable(Ogl.LIGHTING);
		
      Ogl.Disable(Ogl.BLEND);
		
		Ogl.Disable(Ogl.POLYGON_OFFSET_FILL);
		Ogl.Disable(Ogl.STENCIL_TEST);
	}

	var testTexture = CreateTexture();

	var shadowMapSize = 256;
	var shadowMapTexture = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);
	Ogl.TexImage2D(Ogl.TEXTURE_2D, 0, Ogl.DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0, Ogl.DEPTH_COMPONENT, Ogl.UNSIGNED_BYTE, null);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.NEAREST);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.NEAREST);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);

	this.RenderWithShadows1 = function( renderCallback ) {
		
		// http://www.paulsprojects.net/tutorials/smt/smt.html ( and http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html )
		
		var mat = new Transformation(0.5, 0.0, 0.0, 0.0,  0.0, 0.5, 0.0, 0.0,  0.0, 0.0, 0.5, 0.0,  0.5, 0.5, 0.5, 1.0);

		Ogl.Enable(Ogl.NORMALIZE);

// First pass - from light's point of view

		Ogl.PushAttrib( Ogl.VIEWPORT_BIT | Ogl.LIGHTING_BIT ); // | Ogl.ENABLE_BIT
		Ogl.Viewport(0, 0, shadowMapSize, shadowMapSize);

		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		
		//var lightFov = Ogl.GetLight(Ogl.LIGHT0, Ogl.SPOT_CUTOFF) * 2;

		Ogl.Perspective(60, 1, 10, 50);
		//var lightProjectionMatrix = Ogl.GetDouble(Ogl.PROJECTION_MATRIX, 16);
		mat.Product(Ogl, true);
		
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		Ogl.LookAt( lightPos[0], lightPos[1], lightPos[2],  lightDir[0], lightDir[1], lightDir[2],  0, 1, 0);
//		var lightViewMatrix = Ogl.GetDouble(Ogl.MODELVIEW_MATRIX, 16);
		mat.Product(Ogl, true);


//		Ogl.ColorMaterial(Ogl.FRONT, Ogl.AMBIENT_AND_DIFFUSE);
//		Ogl.Enable(Ogl.COLOR_MATERIAL);
//		Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, 1,1,1,1);
//		Ogl.Material(Ogl.FRONT, Ogl.SHININESS, 16);
//		Ogl.Color(1);


		Ogl.Disable(Ogl.LIGHTING);
		Ogl.CullFace(Ogl.FRONT);
		Ogl.ShadeModel(Ogl.FLAT);
		Ogl.ColorMask(false, false, false, false);
		Ogl.ClearDepth(1);
		Ogl.DepthFunc(Ogl.LEQUAL);
		Ogl.Enable(Ogl.DEPTH_TEST);
		Ogl.Clear(Ogl.DEPTH_BUFFER_BIT);
//		Ogl.PolygonOffset(-2, -1); // set the scale and units used to calculate depth values.

Ogl.Disable(Ogl.TEXTURE_2D);		
		renderCallback(3); // render occluders + shape only
		
		Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);
		Ogl.CopyTexSubImage2D(Ogl.TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSize, shadowMapSize);
		Ogl.ColorMask(true, true, true, true);
		Ogl.ShadeModel(Ogl.SMOOTH);
		Ogl.CullFace(Ogl.BACK);
		Ogl.Disable(Ogl.TEXTURE_2D);

		
		Ogl.PopMatrix();
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.PopMatrix();
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.PopAttrib();

//2nd pass - Draw from camera's point of view

		//Use dim light to represent shadowed areas
//		Ogl.Light(Ogl.LIGHT0, Ogl.POSITION, lightPos); // needed ?
		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 0.2, 0.2, 0.2, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 0.2, 0.2, 0.2, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 0, 0, 0, 1);
		
		Ogl.Enable(Ogl.LIGHTING);
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		renderCallback(4); // render objects that receive shadow

//3rd pass - Draw with bright light

		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1,1,1, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 1,1,1, 1);
		
//		var mat = new Transformation(0.5, 0.0, 0.0, 0.0,  0.0, 0.5, 0.0, 0.0,  0.0, 0.0, 0.5, 0.0,  0.5, 0.5, 0.5, 1.0);
//		mat.Product(lightProjectionMatrix);
//		mat.Product(lightViewMatrix);

//DumpMatrix(mat);


		//Set up texture coordinate generation.
		Ogl.TexGen(Ogl.S, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[4], mat[8], mat[12]);
//		Ogl.TexGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[1], mat[2], mat[3]);
		Ogl.TexGen(Ogl.T, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.T, Ogl.EYE_PLANE, mat[5], mat[5], mat[9], mat[13]);
//		Ogl.TexGen(Ogl.T, Ogl.EYE_PLANE, mat[4], mat[5], mat[6], mat[7]);
		Ogl.TexGen(Ogl.R, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.R, Ogl.EYE_PLANE, mat[2], mat[6], mat[10], mat[14]);
//		Ogl.TexGen(Ogl.R, Ogl.EYE_PLANE, mat[8], mat[9], mat[10], mat[11]);
		Ogl.TexGen(Ogl.Q, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.Q, Ogl.EYE_PLANE, mat[3], mat[7], mat[11], mat[15]);
//		Ogl.TexGen(Ogl.Q, Ogl.EYE_PLANE, mat[12], mat[13], mat[14], mat[15]);

		Ogl.Enable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_R, Ogl.TEXTURE_GEN_Q);

		//Bind & enable shadow map texture
//		Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);
//Ogl.BindTexture(Ogl.TEXTURE_2D, testTexture);

		Ogl.PixelTransfer(
/*
		//Enable shadow comparison
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_MODE, Ogl.COMPARE_R_TO_TEXTURE);
		//Shadow comparison should be true (ie not in shadow) if r<=texture
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_FUNC, Ogl.LEQUAL);
		//Shadow comparison should generate an INTENSITY result
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.DEPTH_TEXTURE_MODE, Ogl.INTENSITY);
*/
		// ?? Each component is then multiplied by the signed scale factor GL_c_SCALE, added to the signed bias GL_c_BIAS, and clamped to the range [0,1] (see glPixelTransfer).

		//Set alpha test to discard false comparisons
		Ogl.AlphaFunc(Ogl.GEQUAL, 0.99);
		Ogl.Enable(Ogl.ALPHA_TEST);
		Ogl.Enable(Ogl.TEXTURE_2D);
		
		renderCallback(6); // render all objects

		Ogl.Disable(Ogl.TEXTURE_2D);
		Ogl.Disable(Ogl.ALPHA_TEST);
		Ogl.Disable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_R, Ogl.TEXTURE_GEN_Q);
	}

	
	this.DrawGrid = function() {

		if ( !arguments.callee.geometry ) {
			
			arguments.callee.geometry = Ogl.NewList(false);

			Ogl.Enable(Ogl.DEPTH_TEST);
			Ogl.Enable(Ogl.BLEND);
			Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
			
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
	
	
	this.DrawText = function(text, infrontFct) {

		Ogl.PushAttrib(Ogl.ENABLE_BIT | Ogl.POLYGON_BIT);
		if ( infrontFct ) {

			Ogl.Disable(Ogl.DEPTH_TEST);
			Ogl.Disable(Ogl.LIGHTING);
			Ogl.PushMatrix();
			Ogl.LoadIdentity();
			infrontFct();
		}
		Ogl.Disable(Ogl.CULL_FACE);
		var str = text;
		f3d.SetColor();
		f3d.Draw(text, -f3d.Width(text)/2, f3d.height);
		if ( infrontFct )
			Ogl.PopMatrix();
		Ogl.PopAttrib();
	}
	
	function CreateTexture() {

		var size = 256;
		var texture = new Texture(size, size, 4);
		texture.Set(0).AddGradiantQuad('#ff0000ff', '#00ff00ff', '#0000ffff', '#ffffff00');
		var glTexture = Ogl.GenTexture();
		Ogl.BindTexture(Ogl.TEXTURE_2D, glTexture);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.NEAREST);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.NEAREST);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);		
		Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
		texture.Free();
		return glTexture;
	}

	
	//////////

	this.Idle = function() { // default function
	}
	
	this.Draw = function() { // default function

		Ogl.PushAttrib(Ogl.ENABLE_BIT);
		Ogl.Disable(Ogl.CULL_FACE);
		var str = 'nothing to draw';
		Ogl.Translate(-f3d.Width(str)/2, 0, -80);
		f3d.Draw(str);
		Ogl.PopAttrib();
	}
	
	var fps;
	
	function SurfaceReady() {
		
		var t0 = TimeCounter();
		Ogl.Viewport(0, 0, videoWidth, videoHeight);
		Ogl.ClearColor(0.15, 0.2, 0.4, 0);
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);

		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.LoadIdentity();
		this.Projection();
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.LoadIdentity();
	
		this.Draw(frame);

		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.LoadIdentity();
		Ogl.Ortho(0, videoWidth, 0, videoHeight, 0, 1);
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.LoadIdentity();
		Ogl.PushAttrib(Ogl.ENABLE_BIT);
		Ogl.Disable(Ogl.LIGHTING);
		Ogl.Disable(Ogl.DEPTH_TEST);
		Ogl.Color(0);
		Ogl.Begin(Ogl.QUADS);
		Ogl.Vertex(0,0);  Ogl.Vertex(videoWidth,0);  Ogl.Vertex(videoWidth,16);  Ogl.Vertex(0,16);
		Ogl.End();

		f2d.SetBackgroundColor([0,0,0,0]);
		f2d.SetColor([1]);
		var str = fps+'fps\t'+this.status;
		for ( var [i,chunk] in Iterator(str.split('\t')) )
			f2d.Draw(chunk, 2 + i * 150, 2);
		Ogl.PopAttrib();

		Ogl.Finish();
		GlSwapBuffers(true);
		this.Idle();
		
		fps = (1000/(TimeCounter()-t0)).toFixed(0);
		frame++;
	}
	
	var keyObjListeners = {};

	this.keyState = new ObjEx(undefined, undefined, function(name) {
			
		var sym = global['K_'+name.toUpperCase()] || global['K_'+name.toLowerCase()];
		return GetKeyState(sym);
	});
	
	this.key = new ObjEx(function(name, fct) {
			
			var sym = global['K_'+name.toUpperCase()] || global['K_'+name.toLowerCase()];
			keyObjListeners[sym] = fct;
		},
		function(name) {
			
			var sym = global['K_'+name.toUpperCase()] || global['K_'+name.toLowerCase()];
			delete keyObjListeners[sym];
		}
	);

	this.mouse = {};
	
	var eventListenerList = [];
	this.AddEventListener = function(obj) {

		eventListenerList.push(obj);
	}
	this.RemoveEventListener = function(obj) {

		eventListenerList.splice(eventListenerList.lastIndexOf(obj), 1);
	}
	
	var listeners = {
	
		onKeyDown:function(sym, mod, chr) {
		
			for each ( var l in eventListenerList )
				l.onKeyDown && l.onKeyDown.apply(l, arguments);
		
			for ( var name in keyObjListeners )
				if ( sym == name )
					keyObjListeners[name](true);

			if ( sym == K_ESCAPE ) {
				
				endSignal = true;
			}
	
			if ( sym == K_RETURN && (mod & KMOD_LALT) ) {

				ProcessEvents( SurfaceReadyEvents() );
				if ( videoFlags & FULLSCREEN ) {
				
					SetVideoMode(currentWidth, currentHeight, undefined, defaultVideoMode);
				} else {
				
					currentWidth = videoWidth;
					currentHeight = videoHeight;
					SetVideoMode(desktopWidth, desktopHeight, undefined, defaultVideoMode | FULLSCREEN);
				}
			}
		},
		onKeyUp:function(sym, mod, chr) {

			for each ( var l in eventListenerList )
				l.onKeyUp && l.onKeyUp.apply(l, arguments);

			for ( var name in keyObjListeners )
				if ( sym == name )
					keyObjListeners[name](false);
		},
		onMouseButtonDown:function(button, x, y, buttonState, modState) {

			for each ( var l in eventListenerList )
				l.onMouseButtonDown && l.onMouseButtonDown.apply(l, arguments);
		
			var fct = _this.mouse['button'+button];
			fct && fct(true);
		},
		onMouseButtonUp:function(button, x, y, buttonState, modState) {

			for each ( var l in eventListenerList )
				l.onMouseButtonUp && l.onMouseButtonUp.apply(l, arguments);

			var fct = _this.mouse['button'+button];
			fct && fct(false);
		},
		onMouseMotion:function(x, y, relx, rely, state, mod) {

			for each ( var l in eventListenerList )
				l.onMouseMotion && l.onMouseMotion.apply(l, arguments);

			var fct = _this.mouse.move;
			fct && fct(false);
		},
		
		onQuit: function() {
		 
			endSignal = true;
		},
		onVideoResize: function(w, h) {
			
//			Print('resize '+w+'x'+h, '\n')
//			SetVideoMode();
//			Ogl.Viewport(0, 0, w,h);
		},
	};

	this.status = 'status';
	var frame = 0;
	
	this.Projection = function() {
		
		Ogl.Perspective(60, undefined, 0.5, 1000);
	}

	this.Loop = function() {
		
		while ( !endSignal )
			ProcessEvents( EndSignalEvents(), SDLEvents(listeners), SurfaceReadyEvents.call(this, SurfaceReady) );
	}
}







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
		Ogl.Perspective(60, undefined, 0.1, 100000);
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
	Exec('main.js', false);
	throw 0;
}

var FakeQAApi = { __noSuchMethod__:function(id, args) { Print( id, ':', uneval(args), '\n' ) } };