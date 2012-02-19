// loadModule('jsstd');  loadModule('jsio');  var QA = { __noSuchMethod__:function(id, args) { print( id, ':', uneval(args), '\n' ) } };  exec( /[^/\\]+$/(currentDirectory)[0] + '_qa.js');  Halt();

loadModule('jsstd');
loadModule('jsio');
loadModule('jsimage');
loadModule('jsode');
loadModule('jsprotex');
loadModule('jstrimesh');
loadModule('jssdl');
loadModule('jsgraphics');

try {

//loadModule('jsstd'); exec('../../tests/arabesques.js'); throw 0;
//loadModule('jsstd'); exec('../../tests/explodebox.js'); throw 0;
//loadModule('jsstd'); exec('../../tests/fragmentShaderTest.js'); throw 0;
loadModule('jsstd'); exec('../../tests/podtest.js'); throw 0;

} catch(ex) {

	print( uneval(ex) );
}

throw 0;



// OpenGl doc: http://www.opengl.org/sdk/docs/man/

GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );
GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );

GlSetAttribute( GL_MULTISAMPLEBUFFERS, 1 );
GlSetAttribute( GL_MULTISAMPLESAMPLES, 1 );

SetVideoMode( 64, 64, 32, OPENGL );

print( 'LINE_WIDTH_RANGE: ', Ogl.GetDouble(Ogl.LINE_WIDTH_RANGE, 2).toString(), '\n' );



Halt(); //////////////////////////////////////////////////////////////////////////




/*

loadModule('jsstd');
loadModule('jsio');
loadModule('jsimage');
loadModule('jsode');
loadModule('jsprotex');
loadModule('jstrimesh');
loadModule('jssdl');
loadModule('jsgraphics');

var e = QuaternionToEuler(EulerToQuaternion([-0.1, -0.2, -0.3]));
var e = AxisAngleToQuaternion(QuaternionToAxisAngle([.1,.2,.3,.4]) );


e = Vector3Length([2,2,2], [1,1,1]);

print( e, '\n' );

Halt();




//Halt(); //////////////////////////////////////////////////////////////////////


function Trace() {
	
	for ( var i=0; i<arguments.length; i++ )
		print( arguments[i], ' ' );
	print( '\n' );
}

function DumpMatrix(m) {
    
	for (var y = 0; y < 4; ++y) {
		print('[ ' );
		for (var x = 0; x < 4; ++x)
			print( m[x+y*4].toFixed(3) + '  ' );
		print(']\n' );
	}
	print('\n' );
}


function MinMax(val, min, max) val < min ? min : val > max ? max : val;

function Range(min, max) ({ __iterator__:function() { for (var i = min; i <= max; i++) yield i }});

function Count(n) ({ __iterator__:function() { for (var i = 0; i < n; i++) yield i }});


const curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }

var perspective = new Transformation();
var mat = new Transformation();

var t, t1;
var isCameraMoving;
var cx, cy, cz; // camera
var vx=0, vy=0, vz=2.5, t=0;
var lines = [];

var listeners = {
	onQuit: function() { end = true },
	onKeyDown: function(key, mod) { end = key == K_ESCAPE },
	onVideoResize: function(w,h) { Ogl.Viewport(0, 0, w, h) },
	onMouseButtonDown: function(button, x, y) {
		
		if ( button == BUTTON_LEFT ) {
			
			x = 2*x/videoWidth - 1;
			y = -(2*y/videoHeight - 1);

			var p1 = mat.TransformVector([x,y,0,1]);
			p1[0] /= p1[3];
			p1[1] /= p1[3];
			p1[2] /= p1[3];

			var p2 = mat.TransformVector([x,y,1,1]);
			p2[0] /= p2[3];
			p2[1] /= p2[3];
			p2[2] /= p2[3];

			lines.push([p1,p2]);
		}
		if ( button == BUTTON_RIGHT ) {
			showCursor = false;
			grabInput = true;
		}
	},
	onMouseButtonUp: function(button) {
		
		if ( button == BUTTON_RIGHT ) {
			showCursor = true;
			grabInput = false;
		}
		if ( button == BUTTON_WHEELUP ) {
		
			vz -= modifierState & KMOD_LCTRL ? 0.1 : 1;
			isCameraMoving = true;
		}
		if ( button == BUTTON_WHEELDOWN ) {
		
			vz += modifierState & KMOD_LCTRL ? 0.1 : 1;
			isCameraMoving = true;
		}
	},
	onMouseMotion: function(px,py,dx,dy,button) {
		if ( grabInput ) {
			vx += dx;
			vy += dy;
			isCameraMoving = true;
		}
	}
}

var listList = { __proto__:null };
function CondNewList(name, invalidate) {
	
	if ( name in listList && !invalidate ) {
		
		Ogl.CallList(listList[name]);
		return false;
	} else {
		
		listList[name] = Ogl.NewList();
		return true;
	}
}

function Axis(size) {

	with (Ogl) {
	
		LineWidth(1);
		Begin(LINES);
		Color( 1,0,0, 0.5 ); Vertex( 0,0,0 ); Vertex( size,0,0 );
		Color( 0,1,0, 0.5 ); Vertex( 0,0,0 ); Vertex( 0,size,0 );
		Color( 0,0,1, 0.5 ); Vertex( 0,0,0 ); Vertex( 0,0,size );
		End();
	}
}

function Quad() {
	
	with (Ogl) {
		
		Begin(QUADS);
		TexCoord(0, 0); Vertex(-1, -1);
		TexCoord(1, 0); Vertex(+1, -1);
		TexCoord(1, 1); Vertex(+1, +1);
		TexCoord(0, 1); Vertex(-1, +1);
		End();
	}
}


function Cloud( size, amp ) {

	var octaves = Math.log(size) / Math.LN2;
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.ClearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.AddNoise(a);
		a *= amp;
		s *= 2;
		cloud.Resize(s, s, false);
		cloud.BoxBlur(3, 3);
	}
	cloud.NormalizeLevels();
	return cloud;
}


function CreateCloudTextureLayer() {
	
	var texture = Cloud(32, 1);
	var gaussian = new Texture(texture.width, texture.height, 1);
	gaussian.Set(0);
	gaussian.AddGradiantRadial(curveGaussian(0.5), true);
//	gaussian.NormalizeLevels();
	gaussian.Add(-gaussian.GetBorderLevelRange()[1]);

	texture.Mult(gaussian);
	var tid = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, tid);
	Ogl.DefineTextureImage(Ogl.TEXTURE_2D, Ogl.ALPHA, texture);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	return tid;
}





RandSeed(1235);
// var cloudLayerList = [ [ (RandReal()-0.5)*5, (RandReal()-0.5)*5, (RandReal()-0.5)*5, 5, CreateCloudTextureLayer() ] for (i in Count(5)) ];

function Draw3DCloud() {
	
	// http://graphicsrunner.blogspot.com/2008/03/volumetric-clouds.html / http://www.inframez.com/events_volclouds_slide18.htm
	// volumetric cloud and particules: http://www.inframez.com/events_volclouds_slide01.htm

	with (Ogl) {

		PushAttrib(ENABLE_BIT | DEPTH_BUFFER_BIT | LIGHTING_BIT | TEXTURE_BIT) // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/pushattrib.html

		ShadeModel(FLAT);
//		DepthFunc(ALWAYS);
		Disable(DEPTH_TEST);
		
		// color = polygon * src + screen * dst
		Enable(BLEND); 
		// http://www.opengl.org/sdk/docs/man/xhtml/glBlendFunc.xml

// with LUMINANCE only		
		//BlendFunc(ONE, ONE); // radioactive cloud
		//BlendFunc(DST_COLOR, ONE); // less radioactive cloud
		//BlendFunc(ONE_MINUS_DST_COLOR, ONE); // normal cloud
		//BlendFunc(ONE_MINUS_SRC_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 1
		//BlendFunc(ONE_MINUS_DST_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 2
		//BlendFunc(ZERO, ONE_MINUS_SRC_COLOR); // dark cloud
		//BlendFunc(SRC_COLOR, ONE_MINUS_SRC_COLOR); // less dark cloud
		
// with ALPHA only
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
		
		
		Enable(TEXTURE_2D);

		for each ( var [x,y,z, scale, tid] in cloudLayerList ) {

			BindTexture(TEXTURE_2D, tid);

			PushMatrix();
			Translate(x, y, z);
			KeepTranslation();

			var dis = Vector3Length(cx-x, cy-y, cz-z);
			var fx = dis > 2 ? 1 : dis/2;
//			Color(0.7 * fx, 0.6 * fx, 0.4 * fx);
			Color(0.6, 0.5, 0, 1);

			Begin(QUADS);
			TexCoord(0, 0); Vertex(-scale, -scale);
			TexCoord(1, 0); Vertex(+scale, -scale);
			TexCoord(1, 1); Vertex(+scale, +scale);
			TexCoord(0, 1); Vertex(-scale, +scale);
			End();
			//DrawDisk(scale, 6);
			PopMatrix();
		}
		PopAttrib();
	}
}




with (Ogl) {

	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	Hint(LINE_SMOOTH_HINT, NICEST);
	Hint(POINT_SMOOTH_HINT, NICEST);

//	PointParameter( POINT_SIZE_MIN, 0 );
//	PointParameter( POINT_SIZE_MAX, 1 );
//	PointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01/PixelWidthFactor(), 0] ); // 1/(a + b*d + c *d^2)
//	Enable(POINT_SPRITE); // http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
//	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
// Enable(POINT_SPRITE);

	TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
//	TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [1,1,1,0]);

// see. http://jerome.jouvie.free.fr/OpenGl/Tutorials/Tutorial9.php

//	Enable(LINE_SMOOTH);

	ClearColor(0.2, 0.1, 0.4, 1);
	Enable(DEPTH_TEST);

	MatrixMode(PROJECTION);
	Perspective(60, 0.1, 10000);
	perspective.Load(Ogl);
	MatrixMode(MODELVIEW);
}


function Scene1() {

	with (Ogl) {

		LineWidth(2);
		Begin(LINES);
		Color(1,1,1, 1);
		Vertex( -10, 0 );
		Vertex( 10, 0 );
		Color(0,0,0, 1);
		Vertex( -10, 0.2 );
		Vertex( 10, 0.2 );
		Color(1,0,0, 1);
		Vertex( -10, 0.4 );
		Vertex( 10, 0.4 );
		Color(0,1,0, 1);
		Vertex( -10, 0.6 );
		Vertex( 10, 0.6 );
		Color(0,0,1, 1);
		Vertex( -10, 0.8 );
		Vertex( 10, 0.8 );
		End();

		if ( CondNewList('clouds', isCameraMoving) ) { // clouds are static.
			
			Draw3DCloud();
			EndList();
		}

		Enable(BLEND);
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
		
		LineWidth(4);
		Begin(LINES);
		Color(1,1,1, 1);
		Vertex( -10, 1 );
		Vertex( 10, 1 );
		Color(0,0,0, 1);
		Vertex( -10, 1.2 );
		Vertex( 10, 1.2 );
		Color(1,0,0, 1);
		Vertex( -10, 1.4 );
		Vertex( 10, 1.4 );
		Color(0,1,0, 1);
		Vertex( -10, 1.6 );
		Vertex( 10, 1.6 );
		Color(0,0,1, 1);
		Vertex( -10, 1.8 );
		Vertex( 10, 1.8 );
		End();

		Disable(TEXTURE_2D);
		Axis(1);
	}
}


var Scene2 = new function() {
	
	var vertexList = [];
	var colorList = [];
	
	for ( var i = 0; i < 10000; i++ ) {

//	for ( var x = -2; x <= 2; x += 0.1 )
//		for ( var y = -2; y <= 2; y += 0.1 )
//			for ( var z = -2; z <= 2; z += 0.1 ) {
				
//					var ox = (PerlinNoise( 1.6, 0.5, 5, x )-0.5)*20;
//					var oy = (PerlinNoise( 1.6, 0.5, 5, y )-0.5)*20;
//					var oz = (PerlinNoise( 1.6, 0.5, 5, z )-0.5)*20;
				
				var ox = RandReal()-0.5;
				var oy = RandReal()-0.5;
				var oz = RandReal()-0.5;
				
				var v = (PerlinNoise( 1.3, 0.3, 3, ox/2, oy/2, oz/2 ))*1.5;
			
				colorList.push( v, 1-v, 0.5, v/2 );
				vertexList.push( ox,oy,oz );

			}

	print( vertexList.length / 3, '\n' );
	var tm = new Trimesh();
	tm.DefineVertexBuffer(vertexList);
	tm.DefineColorBuffer(colorList);
	var tmid = Ogl.LoadTrimesh(tm);
	
	var tex = new Texture(16,16, 1).Set(0).AddGradiantRadial( curveGaussian( 0.5 ) );
	
	with (Ogl) {
	
		var textureId = GenTexture();
		BindTexture(TEXTURE_2D, textureId);
		DefineTextureImage(TEXTURE_2D, ALPHA, tex);
		TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
		TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
//		PointParameter(POINT_DISTANCE_ATTENUATION, [0, 0, 0.01]); // 1/(a + b*d + c *d^2)
		Enable(POINT_SPRITE);
		TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
//		TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	}
	
	do {

//		Ogl.Color(0, 0, 1, 0.5);
//		Ogl.Enable(Ogl.POINT_SMOOTH);
		Ogl.Disable(Ogl.DEPTH_TEST);
		Ogl.Enable(Ogl.BLEND);
		Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.PointSize(10);
		Ogl.Enable(Ogl.TEXTURE_2D);
		Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);
		Ogl.DrawTrimesh( tmid, Ogl.POINTS );
//		Quad();
	
	} while ( !(yield) );

}



var Scene3 = new function() {
	
	function FractalCubeFace( px, py, pz,  x1, y1, z1,  x2, y2, z2 ) {

		var face = new Texture(128,128,1).Set(0);
		for ( var scale = 2; scale <= 64; scale *= 2 )
			face.AddPerlin2([px*scale, py*scale, pz*scale], [x1*scale, y1*scale, z1*scale], [x2*scale, y2*scale, z2*scale], 1/scale);
		face.Add(0.25);
		face.Mult(1.5);
		return face;
	}	

	function GenFaceTexture( px, py, pz,  x1, y1, z1,  x2, y2, z2 ) {

		var tid = Ogl.GenTexture();
		Ogl.BindTexture(Ogl.TEXTURE_2D, tid);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP );
		Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP );
		Ogl.DefineTextureImage(Ogl.TEXTURE_2D, Ogl.ALPHA, FractalCubeFace(px, py, pz,  x1, y1, z1,  x2, y2, z2));
		return tid;
	}
	
	var fractalCube = Ogl.NewList(true);
	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(-1,-1,1, 2,0,0, 0,2,0));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0,  1.0);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0,  1.0);
	Ogl.End();

	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(-1,1,1, 2,0,0, 0,0,-2));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0, -1.0);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0,  1.0,  1.0);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0,  1.0,  1.0);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0, -1.0);
	Ogl.End();

	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(-1,1,-1, 2,0,0, 0,-2,0));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0, -1.0, -1.0);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0,  1.0, -1.0);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0,  1.0, -1.0);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0, -1.0, -1.0);
	Ogl.End();

	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(1,-1,1, -2,0,0, 0,0,-2));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex(-1.0, -1.0, -1.0);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex( 1.0, -1.0, -1.0);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
	Ogl.End();

	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(1,-1,1, 0,0,-2, 0,2,0));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex( 1.0, -1.0, -1.0);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex( 1.0,  1.0, -1.0);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex( 1.0,  1.0,  1.0);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex( 1.0, -1.0,  1.0);
	Ogl.End();
	
	Ogl.BindTexture(Ogl.TEXTURE_2D, GenFaceTexture(-1,-1,-1, 0,0,2, 0,2,0));
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0.0, 0.0); Ogl.Vertex(-1.0, -1.0, -1.0);
	Ogl.TexCoord(1.0, 0.0); Ogl.Vertex(-1.0, -1.0,  1.0);
	Ogl.TexCoord(1.0, 1.0); Ogl.Vertex(-1.0,  1.0,  1.0);
	Ogl.TexCoord(0.0, 1.0); Ogl.Vertex(-1.0,  1.0, -1.0);
	Ogl.End();


	Ogl.EndList();

	Ogl.Enable(Ogl.TEXTURE_2D);
	Ogl.Enable(Ogl.BLEND);
	Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
	Ogl.TexEnv(Ogl.TEXTURE_ENV, Ogl.TEXTURE_ENV_MODE, Ogl.MODULATE);
	Ogl.Disable(Ogl.DEPTH_TEST);

	do {
		
		Ogl.Color(1);
		Ogl.CallList(fractalCube);
//		Axis(1);
	} while ( !(yield) );

}



var Scene4 = new function() {

	var tId = Ogl.GenTexture();
	Ogl.BindTexture(TEXTURE_2D, tId);

//	var tb = Ogl.CreateTextureBuffer();
//	Ogl.Test(tb);

	do {
		
		Ogl.Color(1);
		Ogl.Rotate(90, 0,1,0);
		Ogl.BindTexture(vTEXTURE_2D, tId);
		Quad();
	} while ( !(yield) );

}


/*
function HWConvolution( width, height, matrix ) {
	
	with (Ogl) {

		DrawBuffer(BACK);
		Viewport(0, 0, width, height);

		Trace( 'VIEWPORT', GetInteger( VIEWPORT, 4 ) );

		MatrixMode(PROJECTION);
		LoadIdentity();
		Ortho(0, width, 0, height, -1, 1);
		MatrixMode(MODELVIEW);
		LoadIdentity();


		Disable(DEPTH_TEST);
		Enable(TEXTURE_2D);
		Enable(BLEND);
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
//		BlendFunc(ONE, ONE);
		TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR); // NEAREST
		TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
		TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	
		ClearColor(0,0,0,1);
		Clear(COLOR_BUFFER_BIT);
		ShadeModel(FLAT);

		for ( var [y, line] in Iterator(matrix) ) {
			for ( var [x, ratio] in Iterator(line) ) {
				
				Color(1,1,1, ratio);
				Begin(QUADS);
				TexCoord(0, 0); Vertex(x, y);
				TexCoord(1, 0); Vertex(width+x, y);
				TexCoord(1, 1); Vertex(width+x, height+y);
				TexCoord(0, 1); Vertex(x, height+y);
				End();
			}
		}
		
		
		Halt();
		
		var tId = GenTexture();
		BindTexture(TEXTURE_2D, tId);
		ReadBuffer(BACK);
		CopyTexImage2D(TEXTURE_2D, 0, RGBA, 0, 0,width, height, 0);
		
		BindFramebuffer(FRAMEBUFFER, 0);
		DeleteRenderbuffer(renderbuffer);
		return tId;
	}
}
*/



function HWBlur() {
	
	with (Ogl) {
	
		// DBD chack display height and width.

		var [,,width, height] = GetInteger(VIEWPORT, 4);
		
		var tId = GenTexture();
		BindTexture(TEXTURE_2D, tId);
		CopyTexImage2D(TEXTURE_2D, 0, RGB, 0, 0, width, height, 0);
		
		MatrixMode(PROJECTION);
		LoadIdentity();
		Ortho(0, width, 0, height, -1, 1);
		MatrixMode(MODELVIEW);
		LoadIdentity();
		
		Disable(DEPTH_TEST);
		Enable(TEXTURE_2D);
		Enable(BLEND);
		BlendFunc(ONE, ONE);
		TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR); // NEAREST
		TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
		TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
		ShadeModel(FLAT);
		
		ClearColor(0,0,0,1);
		Clear(COLOR_BUFFER_BIT);
		
		var factors = [5,12,15,12,5];
		
	
		var y = 0;
		for ( var x = -2; x <= +2; x++ ) {
			
			Color(factors[x+2]/49);
			Begin(QUADS);
			TexCoord(0, 0); Vertex(x, y);
			TexCoord(1, 0); Vertex(width+x, y);
			TexCoord(1, 1); Vertex(width+x, height+y);
			TexCoord(0, 1); Vertex(x, height+y);
			End();
		}	

		var x = 0;
		for ( var y = -y; y <= +2; y++ ) {
			
			Color(factors[x+2]/49);
			Begin(QUADS);
			TexCoord(0, 0); Vertex(x, y);
			TexCoord(1, 0); Vertex(width+x, y);
			TexCoord(1, 1); Vertex(width+x, height+y);
			TexCoord(0, 1); Vertex(x, height+y);
			End();
		}	
		
		DeleteTexture(tId);
	}
	
//	new File('myImage.png').content = EncodePngImage(Ogl.RenderToImage());
	
}


/*
var textureId = Ogl.GenTexture();
Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);
var texture = DecodeJpegImage(new File('image.jpg').Open(File.RDONLY))
Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
*/



var sleep = 20;
for (var end = false; !end ;) {
	
	t1 = TimeCounter();
	print( (1000/(t1-t-sleep)).toFixed(), 'fps     \r' );
	t = t1;
	
	isCameraMoving = false;
	while (PollEvent(listeners));

	var tmp = Math.cos(vy/400);
	cx = -Math.cos(vx/400)*Math.abs(tmp) * vz;
	cy = Math.sin(vx/400)*Math.abs(tmp) * vz;
	cz = Math.sin(vy/400) * vz;

	Ogl.MatrixMode(Ogl.MODELVIEW);
	Ogl.LoadIdentity();
	Ogl.LookAt(cx,cy,cz, 0,0,0, 0,0,1);

	Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	

//	Scene1();
//	Scene2.next();
	Scene3.next();
//	Scene4.next();


/*
	Ogl.MatrixMode(Ogl.PROJECTION);
	Ogl.LoadIdentity();
	Ogl.MatrixMode(Ogl.MODELVIEW);
	Ogl.LoadIdentity();

	mat.Load(Ogl);
	mat.Product(perspective);
	mat.Invert();
	

//	Ogl.Enable(Ogl.TEXTURE_2D);
	
//	Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);

	//Color(1,1,1);
	Ogl.Scale(1, -1);
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1, -1);
	Ogl.TexCoord(1, 0); Ogl.Vertex(+1, -1);
	Ogl.TexCoord(1, 1); Ogl.Vertex(+1, +1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1, +1);
	Ogl.End();
	
	HWBlur();
*/

	for each ( var [p1, p2] in lines ) {

		Ogl.Begin(Ogl.LINES);
		Ogl.Color(1,1,1); Ogl.Vertex( p1[0], p1[1], p1[2] ); Ogl.Vertex( p2[0], p2[1], p2[2] );
		Ogl.End();
	}

	GlSwapBuffers();
	Sleep(sleep);
}

























Halt(); //////////////////////////////////////////////////////////////////////

var t = new Transformation(null);
t.LookAt(0,0,0, 1,0,0, 0,0,1);
t.Translate(0, 0, 1);

var v = [0,0,0];
print( [c.toFixed(2) for each ( c in t.TransformVector(v) )].join('  '), '\n' );


Halt(); //////////////////////////////////////////////////////////////////////


var t = new Transformation(null);
t.Translate(0, 0, 0);
t.RotateZ(-90);
t.Translate(2, 0, 0);
t.RotateY(360);
t.RotateZ(45);
t.Rotate(45, 0,0,1);
t.Translate(1, 0, 0);
t.RotateY(90);
t.Translate(1, 0, 0);
t.RotateY(360);
t.Translate(1, 0, 0);
t.RotateX(360);
t.Translate(1, 0, 0);
t.RotateY(123);
t.Translate(5, 6, 7);
t.Translate(-5, -6, -7);

var v = [0,0,0];
print( [c.toFixed(2) for each ( c in t.TransformVector(v) )].join('  '), '\n' );
t.Invert();
print( [c.toFixed(1) for each ( c in t.TransformVector(v) )].join('  '), '\n' );
print( '\n\n' );




Halt(); //////////////////////////////////////////////////////////////////////





for ( var i = 0; i < 16; i++ )
	print( i%4 ? ' ' : '  ', t[i].toFixed(2) );
print( '\n' );





Halt();

var t = new Transformation(undefined);
t.LookAt(-10,-10,10, 0,0,0, 0,0,1);
		
Transformation.Test();
Halt();



var t = new Transformation( undefined );
t.LookAt(1,1,10, 0,0,0);

DumpMatrix(t);

Halt();


/*
function Quad2D(x0, y0, x1, y1) {

	with (Ogl) {
		Begin(QUADS);
		TexCoord( 0, 0 );
		Vertex( x0, y0 );
		TexCoord( 1, 0 );
		Vertex( x1, y0 );
		TexCoord( 1, 1 );
		Vertex( x1, y1 );
		TexCoord( 0, 1 );
		Vertex( x0, y1 );
		End();
	}
}
*/

function Line(x0, y0, z0, x1, y1, z1) {

	with (Ogl) {
	
		Begin(LINES);
		Vertex(x0, y0, z0);
		Vertex(x1, y1, z1);
		End();
	}
}


function Cube() {

	with (Ogl) {
	
		Begin(QUADS);
		TexCoord(0.0, 0.0); Vertex(-1.0, -1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex( 1.0, -1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0,  1.0);

		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(1.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0,  1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0, -1.0);

		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex(-1.0,  1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0, -1.0);

		TexCoord(1.0, 1.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0, -1.0, -1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0,  1.0);
		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0,  1.0);

		TexCoord(1.0, 0.0); Vertex( 1.0, -1.0, -1.0);
		TexCoord(1.0, 1.0); Vertex( 1.0,  1.0, -1.0);
		TexCoord(0.0, 1.0); Vertex( 1.0,  1.0,  1.0);
		TexCoord(0.0, 0.0); Vertex( 1.0, -1.0,  1.0);

		TexCoord(0.0, 0.0); Vertex(-1.0, -1.0, -1.0);
		TexCoord(1.0, 0.0); Vertex(-1.0, -1.0,  1.0);
		TexCoord(1.0, 1.0); Vertex(-1.0,  1.0,  1.0);
		TexCoord(0.0, 1.0); Vertex(-1.0,  1.0, -1.0);
		End();
	}
}

var time0 = new Date().valueOf();
var ax = 0, ay = 0, pos = 0;

var speedX = 0;
var speedY = 0;

var rotateX = 0;
var rotateY = 0;

var tremeshId;
var tremeshTransformation = new Transformation();
tremeshTransformation.Clear();


function Init() {

	var t = new Trimesh();

	var vertexCount = 3000;
	var triangleCount = 2000;
	
	var vertex = [];
	for ( var i = 0; i < vertexCount*3; i++ )
		vertex.push(Math.random());

	var color = [];
	for ( var i = 0; i < vertexCount*4; i++ )
		color.push(Math.random());

	var index = [];
	for ( var i = 0; i < triangleCount*3; i++ )
		index.push(Math.floor( Math.random() * vertexCount ) );

	t.DefineVertexBuffer(vertex);
	t.DefineColorBuffer(color);
	t.DefineIndexBuffer(index);
	
	tremeshId = Ogl.LoadTrimesh(t);
	

	with (Ogl) {
		Enable(DEPTH_TEST);
		Enable(BLEND); //Enable alpha blending
		BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
	  Enable(CULL_FACE);
	  
  	LoadIdentity();
	  PushMatrix(); 
	 }
}

function Draw() {

	with (Ogl) {
	
		var t = new Date().valueOf() - time0;

		LoadMatrix(tremeshTransformation);

		Translate(-0.5, -0.5, -0.5);
		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		DrawTrimesh(tremeshId);
	};

	var rotation = new Transformation();
	rotation.Clear();
	rotation.RotateToVector( -speedX, speedY, 1 );
	tremeshTransformation.Product(rotation);


}

GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
GlSetAttribute( GL_DOUBLEBUFFER, 1 );
GlSetAttribute( GL_DEPTH_SIZE, 16 );

SetVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
Ogl.Perspective( 90, 0.01, 1000 );

showCursor = true;

var done = false;

var listeners = {
	onQuit: function() done = true,
	onKeyDown: function(key, mod) {
		
		switch (key) {
			case K_ESCAPE: 
				done = true;
				break;
			case K_g:
				grabInput = !grabInput;
				showCursor = !showCursor;
				break;
		}
	},
	onVideoResize: function(w,h) {

//		SetVideoMode(w, h);
		Ogl.Viewport(0, 0, w, h);
	},
	onMouseMotion: function(x,y,dx,dy,button) {
		
		if ( button & BUTTON_LMASK ) {

			speedX += dx;
			speedY += dy;
		}
		
		ax += dx;
		ay += dy;
	}

}

Init();

while ( !done ) {

	PollEvent(listeners);
	Draw();	
	GlSwapBuffers();
	Sleep(10);
}




Halt(); //////////////////////////////////////////////////////////////////////////////////////////////////////////

	var camera = new Transformation;
	camera.Clear();

	var objects = [];

	var win = new Window();
	win.title = "Test";
	var x = 800;
	var y = 600;
	var w = 200;
	var h = 200;
	win.rect = [x,y,x+w,y+h];

	var _quit = false;

	win.onkeydown = function( key, l ) {

		switch (key) {
			case vk.ESC:
				_quit = true;
				break;
		}
	}
	
	win.onmousedown = function(button) {
	
		/*
			1/ Object Coordinates are transformed by the ModelView matrix to produce Eye Coordinates.
			2/ Eye Coordinates are transformed by the Projection matrix to produce Clip Coordinates.
			3/ Clip Coordinate X, Y, and Z are divided by Clip Coordinate W to produce Normalized Device Coordinates.
			4/ Normalized Device Coordinates are scaled and translated by the viewport parameters to produce Window Coordinates.
		*/
		
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=480322
		/*
			Expand to 3D homogenous coordinate. That is make a vector (x, y, z, 1). See below for Z-value.
			Map the X- and Y-value into normalized device coordinates, that is, from [0, viewport size] to the range [-1, 1].
			Multiply by the inverse of the projection matrix.
			Multiply by the inverse of the modelview matrix.
			Perform perspective division.
			The result after perspective division is world space coordinates. Skip the multiplication by the modelview matrix and you get the result in eye space instead.
		*/		
		
		var m = new Transformation();

		var p = new Transformation();
		Ogl.MatrixMode(Ogl.PROJECTION);
		p.Load(Ogl);
		p.Invert();
		
		var c = new Transformation();
		c.Load(camera);
		c.Invert();
	
		// 4/
		var clientRect = win.clientRect;
		var width = clientRect[2] - clientRect[0];
		var height = clientRect[3] - clientRect[1];
//		var viewport = Ogl.GetInteger(Ogl.VIEWPORT, 4);
//		var width = viewport[2] - viewport[0];
//		var height = viewport[3] - viewport[1];
		
		var cursorPosition = win.cursorPosition;
		var px = -1 + 2 * cursorPosition[0] / width;
		var py = -1 + 2 * ( height - cursorPosition[1] ) / height;
		
//		print( ' px='+px,' py='+py, '\n' );
	
//		var vect = [px / p[0], py / p[5], -1];
		var vect = [px, py, -1, 1];
		
		// 2/
//		p.TransformVector(vect);

		// 1/
		c.TransformVector(vect);
		
		vect[0] /= p[0];
		vect[1] /= p[5];
		

		print( 'world space coordinates vect '+[ v.toFixed(1) for each (v in vect) ].join(' '), '\n' );

		
		var obj = {};
		obj.createTime = TimeCounter();
		obj.x = vect[0];
		obj.y = vect[1];
		obj.z = vect[2]-1;
		objects.push(obj);
	}

/*
Line3 Viewport::Ray( int winx, int winy ) {

  Vector4 in( -1 + 2 * ( (float)winx / (float)( _width - _offsetWidth ) ), -1 + 2 * (float)( _height - winy - _offsetHeight ) / (float)(_height - _offsetHeight ), -1, 1 );

  in.x /= ProjectionMatrix()[0];
  in.y /= ProjectionMatrix()[5];

  Matrix44 invView = Camera().Transformation();
  invView.Invert();

  Vector4 out = invView( in ); ...

												  Vector4 operator()( const Vector4 &vector ) const {
												    
													 return Vector4( _m[0]*vector.x + _m[4]*vector.y + _m[8]*vector.z  + _m[12]*vector.w,
																		  _m[1]*vector.x + _m[5]*vector.y + _m[9]*vector.z  + _m[13]*vector.w,
																		  _m[2]*vector.x + _m[6]*vector.y + _m[10]*vector.z + _m[14]*vector.w,
																		  _m[3]*vector.x + _m[7]*vector.y + _m[11]*vector.z + _m[15]*vector.w );
												  }
  

//  out.Rescale(); // should be done if needed

  Point3 position( invView.Translation() );
  Vector3 direction( out.x, out.y, out.z );
  direction -= position;
	return Line3( position, direction );
}
*/




/*
function ResizeWindow(w, h) {

	with (Ogl) {
		Viewport(0,0,w,h);
		MatrixMode(PROJECTION);
		LoadIdentity();
		Ortho(0,0,10,10, -10, 10);
//		Perspective( 90, -1000, 1000 );
	}
	Render();
}

win.onsize = ResizeWindow;		
*/

win.Open();
win.CreateOpenGLContext();


with (Ogl) {

//	ShadeModel(SMOOTH);
	ShadeModel(FLAT);
	FrontFace(CCW);

	ClearColor(0.1,0.15,0.2,1);	

	ClearDepth(1);
//	Enable(DEPTH_TEST); // !!!!
	DepthFunc(LESS);

//	DepthRange(1000, -1000);
	
	Hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	Hint(LINE_SMOOTH_HINT, DONT_CARE);		
	Enable(LINE_SMOOTH);
	LineWidth(1);

	Enable( TEXTURE_2D );
	BindTexture( TEXTURE_2D, GenTexture() );
	TexParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	TexParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
	TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [0,0,0,0]);
	

//	var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load();
	var texture = new Texture(128, 128, 1);
	texture.Set([0]);
//	var curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
	texture.AddGradiantRadial( curveGaussian( 0.3 ), false );
	texture.AddNoise(0.1,0,0,0);

//	Enable(ALPHA_TEST);
//	AlphaFunc( GREATER, 0.5 );

	Enable(BLEND);
//	BlendFunc(ONE, ONE);
	BlendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
	
	DefineTextureImage( TEXTURE_2D, ALPHA, texture );

// http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
//		PointParameter( POINT_SIZE_MIN, 0 );
//		PointParameter( POINT_SIZE_MAX, 1 );
	PointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01] ); // 1/(a + b*d + c *d^2)
	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);

	MatrixMode(PROJECTION);
	LoadIdentity();
	Perspective(60, 0.5, 100);
}


const degToRad = 360/Math.PI;

camera.Translate(0, 0, -20);
//camera.Rotate(45, 1,0,0);

function Render(imgIndex) {
	with (Ogl) {

		Clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
	
		MatrixMode(MODELVIEW);
		LoadMatrix(camera);
		Quad();
	
		Enable( TEXTURE_2D );
		Color(1,1,1,1);
		for each ( var obj in objects ) {
			
			var t = TimeCounter() - obj.createTime;
/*
			PointSize((-Math.cos(t/1000)+1)*10);
			Begin(POINTS);
			Vertex(obj.x, obj.y);
			End();	
*/

			PushMatrix();
			Translate( obj.x, obj.y, obj.z );
			Axis(1);
			PopMatrix();
			

		}
	}
}


for (var i=0; !_quit; i++) {

	MaybeCollectGarbage();
	Ogl.Viewport(0,0,w,h);
	
	var t0 = TimeCounter();
	Render(i);
//	print( (1000/(TimeCounter() - t0)).toFixed(), ' fps\n' );
	win.SwapBuffers();
	win.ProcessEvents();
	Sleep(10);
}

