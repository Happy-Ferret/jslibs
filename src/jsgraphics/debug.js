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

glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
glSetAttribute( GL_DOUBLEBUFFER, 1 );
glSetAttribute( GL_DEPTH_SIZE, 16 );
glSetAttribute( GL_ACCELERATED_VISUAL, 1 );

glSetAttribute( GL_MULTISAMPLEBUFFERS, 1 );
glSetAttribute( GL_MULTISAMPLESAMPLES, 1 );

setVideoMode( 64, 64, 32, OPENGL );

print( 'LINE_WIDTH_RANGE: ', Ogl.getDouble(Ogl.LINE_WIDTH_RANGE, 2).toString(), '\n' );



halt(); //////////////////////////////////////////////////////////////////////////




/*

loadModule('jsstd');
loadModule('jsio');
loadModule('jsimage');
loadModule('jsode');
loadModule('jsprotex');
loadModule('jstrimesh');
loadModule('jssdl');
loadModule('jsgraphics');

var e = quaternionToEuler(eulerToQuaternion([-0.1, -0.2, -0.3]));
var e = axisAngleToQuaternion(quaternionToAxisAngle([.1,.2,.3,.4]) );


e = vector3Length([2,2,2], [1,1,1]);

print( e, '\n' );

halt();




//halt(); //////////////////////////////////////////////////////////////////////


function trace() {
	
	for ( var i=0; i<arguments.length; i++ )
		print( arguments[i], ' ' );
	print( '\n' );
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


function minMax(val, min, max) val < min ? min : val > max ? max : val;

function range(min, max) ({ __iterator__:function() { for (var i = min; i <= max; i++) yield i }});

function count(n) ({ __iterator__:function() { for (var i = 0; i < n; i++) yield i }});


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
	onVideoResize: function(w,h) { Ogl.viewport(0, 0, w, h) },
	onMouseButtonDown: function(button, x, y) {
		
		if ( button == BUTTON_LEFT ) {
			
			x = 2*x/videoWidth - 1;
			y = -(2*y/videoHeight - 1);

			var p1 = mat.transformVector([x,y,0,1]);
			p1[0] /= p1[3];
			p1[1] /= p1[3];
			p1[2] /= p1[3];

			var p2 = mat.transformVector([x,y,1,1]);
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
function condNewList(name, invalidate) {
	
	if ( name in listList && !invalidate ) {
		
		Ogl.callList(listList[name]);
		return false;
	} else {
		
		listList[name] = Ogl.newList();
		return true;
	}
}

function axis(size) {

	with (Ogl) {
	
		lineWidth(1);
		begin(LINES);
		color( 1,0,0, 0.5 ); vertex( 0,0,0 ); vertex( size,0,0 );
		color( 0,1,0, 0.5 ); vertex( 0,0,0 ); vertex( 0,size,0 );
		color( 0,0,1, 0.5 ); vertex( 0,0,0 ); vertex( 0,0,size );
		end();
	}
}

function quad() {
	
	with (Ogl) {
		
		begin(QUADS);
		texCoord(0, 0); vertex(-1, -1);
		texCoord(1, 0); vertex(+1, -1);
		texCoord(1, 1); vertex(+1, +1);
		texCoord(0, 1); vertex(-1, +1);
		end();
	}
}


function cloud( size, amp ) {

	var octaves = Math.log(size) / Math.LN2;
	var a = 1, s = 1;
	var cloud = new Texture(s, s, 1);
	cloud.clearChannel();
	while ( octaves-- > 0 ) {
		
		cloud.addNoise(a);
		a *= amp;
		s *= 2;
		cloud.resize(s, s, false);
		cloud.boxBlur(3, 3);
	}
	cloud.normalizeLevels();
	return cloud;
}


function createCloudTextureLayer() {
	
	var texture = cloud(32, 1);
	var gaussian = new Texture(texture.width, texture.height, 1);
	gaussian.Set(0);
	gaussian.addGradiantRadial(curveGaussian(0.5), true);
//	gaussian.NormalizeLevels();
	gaussian.add(-gaussian.getBorderLevelRange()[1]);

	texture.mult(gaussian);
	var tid = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, tid);
	Ogl.defineTextureImage(Ogl.TEXTURE_2D, Ogl.ALPHA, texture);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	return tid;
}





randSeed(1235);
// var cloudLayerList = [ [ (RandReal()-0.5)*5, (RandReal()-0.5)*5, (RandReal()-0.5)*5, 5, CreateCloudTextureLayer() ] for (i in Count(5)) ];

function draw3DCloud() {
	
	// http://graphicsrunner.blogspot.com/2008/03/volumetric-clouds.html / http://www.inframez.com/events_volclouds_slide18.htm
	// volumetric cloud and particules: http://www.inframez.com/events_volclouds_slide01.htm

	with (Ogl) {

		pushAttrib(ENABLE_BIT | DEPTH_BUFFER_BIT | LIGHTING_BIT | TEXTURE_BIT) // http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/pushattrib.html

		shadeModel(FLAT);
//		DepthFunc(ALWAYS);
		disable(DEPTH_TEST);
		
		// color = polygon * src + screen * dst
		enable(BLEND); 
		// http://www.opengl.org/sdk/docs/man/xhtml/glBlendFunc.xml

// with LUMINANCE only		
		//blendFunc(ONE, ONE); // radioactive cloud
		//blendFunc(DST_COLOR, ONE); // less radioactive cloud
		//blendFunc(ONE_MINUS_DST_COLOR, ONE); // normal cloud
		//blendFunc(ONE_MINUS_SRC_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 1
		//blendFunc(ONE_MINUS_DST_COLOR, ONE_MINUS_SRC_COLOR); // normal strange 2
		//blendFunc(ZERO, ONE_MINUS_SRC_COLOR); // dark cloud
		//blendFunc(SRC_COLOR, ONE_MINUS_SRC_COLOR); // less dark cloud
		
// with ALPHA only
		blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
		
		
		enable(TEXTURE_2D);

		for each ( var [x,y,z, scale, tid] in cloudLayerList ) {

			bindTexture(TEXTURE_2D, tid);

			pushMatrix();
			translate(x, y, z);
			keepTranslation();

			var dis = vector3Length(cx-x, cy-y, cz-z);
			var fx = dis > 2 ? 1 : dis/2;
//			Color(0.7 * fx, 0.6 * fx, 0.4 * fx);
			color(0.6, 0.5, 0, 1);

			begin(QUADS);
			texCoord(0, 0); vertex(-scale, -scale);
			texCoord(1, 0); vertex(+scale, -scale);
			texCoord(1, 1); vertex(+scale, +scale);
			texCoord(0, 1); vertex(-scale, +scale);
			end();
			//drawDisk(scale, 6);
			popMatrix();
		}
		popAttrib();
	}
}




with (Ogl) {

	hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	hint(LINE_SMOOTH_HINT, NICEST);
	hint(POINT_SMOOTH_HINT, NICEST);

//	PointParameter( POINT_SIZE_MIN, 0 );
//	PointParameter( POINT_SIZE_MAX, 1 );
//	PointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01/PixelWidthFactor(), 0] ); // 1/(a + b*d + c *d^2)
//	Enable(POINT_SPRITE); // http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
//	TexEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
// Enable(POINT_SPRITE);

	texEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
//	TexEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [1,1,1,0]);

// see. http://jerome.jouvie.free.fr/OpenGl/Tutorials/Tutorial9.php

//	Enable(LINE_SMOOTH);

	clearColor(0.2, 0.1, 0.4, 1);
	enable(DEPTH_TEST);

	matrixMode(PROJECTION);
	perspective(60, 0.1, 10000);
	perspective.load(Ogl);
	matrixMode(MODELVIEW);
}


function scene1() {

	with (Ogl) {

		lineWidth(2);
		begin(LINES);
		color(1,1,1, 1);
		vertex( -10, 0 );
		vertex( 10, 0 );
		color(0,0,0, 1);
		vertex( -10, 0.2 );
		vertex( 10, 0.2 );
		color(1,0,0, 1);
		vertex( -10, 0.4 );
		vertex( 10, 0.4 );
		color(0,1,0, 1);
		vertex( -10, 0.6 );
		vertex( 10, 0.6 );
		color(0,0,1, 1);
		vertex( -10, 0.8 );
		vertex( 10, 0.8 );
		end();

		if ( condNewList('clouds', isCameraMoving) ) { // clouds are static.
			
			draw3DCloud();
			endList();
		}

		enable(BLEND);
		blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
		
		lineWidth(4);
		begin(LINES);
		color(1,1,1, 1);
		vertex( -10, 1 );
		vertex( 10, 1 );
		color(0,0,0, 1);
		vertex( -10, 1.2 );
		vertex( 10, 1.2 );
		color(1,0,0, 1);
		vertex( -10, 1.4 );
		vertex( 10, 1.4 );
		color(0,1,0, 1);
		vertex( -10, 1.6 );
		vertex( 10, 1.6 );
		color(0,0,1, 1);
		vertex( -10, 1.8 );
		vertex( 10, 1.8 );
		end();

		disable(TEXTURE_2D);
		axis(1);
	}
}


var scene2 = new function() {
	
	var vertexList = [];
	var colorList = [];
	
	for ( var i = 0; i < 10000; i++ ) {

//	for ( var x = -2; x <= 2; x += 0.1 )
//		for ( var y = -2; y <= 2; y += 0.1 )
//			for ( var z = -2; z <= 2; z += 0.1 ) {
				
//					var ox = (PerlinNoise( 1.6, 0.5, 5, x )-0.5)*20;
//					var oy = (PerlinNoise( 1.6, 0.5, 5, y )-0.5)*20;
//					var oz = (PerlinNoise( 1.6, 0.5, 5, z )-0.5)*20;
				
				var ox = randReal()-0.5;
				var oy = randReal()-0.5;
				var oz = randReal()-0.5;
				
				var v = (perlinNoise( 1.3, 0.3, 3, ox/2, oy/2, oz/2 ))*1.5;
			
				colorList.push( v, 1-v, 0.5, v/2 );
				vertexList.push( ox,oy,oz );

			}

	print( vertexList.length / 3, '\n' );
	var tm = new Trimesh();
	tm.defineVertexBuffer(vertexList);
	tm.defineColorBuffer(colorList);
	var tmid = Ogl.loadTrimesh(tm);
	
	var tex = new Texture(16,16, 1).Set(0).addGradiantRadial( curveGaussian( 0.5 ) );
	
	with (Ogl) {
	
		var textureId = genTexture();
		bindTexture(TEXTURE_2D, textureId);
		defineTextureImage(TEXTURE_2D, ALPHA, tex);
		texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR);
		texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
//		PointParameter(POINT_DISTANCE_ATTENUATION, [0, 0, 0.01]); // 1/(a + b*d + c *d^2)
		enable(POINT_SPRITE);
		texEnv(POINT_SPRITE, COORD_REPLACE, TRUE);
//		TexEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	}
	
	do {

//		Ogl.Color(0, 0, 1, 0.5);
//		Ogl.Enable(Ogl.POINT_SMOOTH);
		Ogl.disable(Ogl.DEPTH_TEST);
		Ogl.enable(Ogl.BLEND);
		Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.pointSize(10);
		Ogl.enable(Ogl.TEXTURE_2D);
		Ogl.bindTexture(Ogl.TEXTURE_2D, textureId);
		Ogl.drawTrimesh( tmid, Ogl.POINTS );
//		Quad();
	
	} while ( !(yield) );

}



var scene3 = new function() {
	
	function fractalCubeFace( px, py, pz,  x1, y1, z1,  x2, y2, z2 ) {

		var face = new Texture(128,128,1).Set(0);
		for ( var scale = 2; scale <= 64; scale *= 2 )
			face.addPerlin2([px*scale, py*scale, pz*scale], [x1*scale, y1*scale, z1*scale], [x2*scale, y2*scale, z2*scale], 1/scale);
		face.add(0.25);
		face.mult(1.5);
		return face;
	}	

	function genFaceTexture( px, py, pz,  x1, y1, z1,  x2, y2, z2 ) {

		var tid = Ogl.genTexture();
		Ogl.bindTexture(Ogl.TEXTURE_2D, tid);
		Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
		Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
		Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP );
		Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP );
		Ogl.defineTextureImage(Ogl.TEXTURE_2D, Ogl.ALPHA, fractalCubeFace(px, py, pz,  x1, y1, z1,  x2, y2, z2));
		return tid;
	}
	
	var fractalCube = Ogl.newList(true);
	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(-1,-1,1, 2,0,0, 0,2,0));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0,  1.0);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0,  1.0);
	Ogl.end();

	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(-1,1,1, 2,0,0, 0,0,-2));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0, -1.0);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0,  1.0,  1.0);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0,  1.0,  1.0);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0, -1.0);
	Ogl.end();

	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(-1,1,-1, 2,0,0, 0,-2,0));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0, -1.0, -1.0);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0,  1.0, -1.0);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0,  1.0, -1.0);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0, -1.0, -1.0);
	Ogl.end();

	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(1,-1,1, -2,0,0, 0,0,-2));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex(-1.0, -1.0, -1.0);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex( 1.0, -1.0, -1.0);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
	Ogl.end();

	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(1,-1,1, 0,0,-2, 0,2,0));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex( 1.0, -1.0, -1.0);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex( 1.0,  1.0, -1.0);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex( 1.0,  1.0,  1.0);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex( 1.0, -1.0,  1.0);
	Ogl.end();
	
	Ogl.bindTexture(Ogl.TEXTURE_2D, genFaceTexture(-1,-1,-1, 0,0,2, 0,2,0));
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0.0, 0.0); Ogl.vertex(-1.0, -1.0, -1.0);
	Ogl.texCoord(1.0, 0.0); Ogl.vertex(-1.0, -1.0,  1.0);
	Ogl.texCoord(1.0, 1.0); Ogl.vertex(-1.0,  1.0,  1.0);
	Ogl.texCoord(0.0, 1.0); Ogl.vertex(-1.0,  1.0, -1.0);
	Ogl.end();


	Ogl.endList();

	Ogl.enable(Ogl.TEXTURE_2D);
	Ogl.enable(Ogl.BLEND);
	Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
	Ogl.texEnv(Ogl.TEXTURE_ENV, Ogl.TEXTURE_ENV_MODE, Ogl.MODULATE);
	Ogl.disable(Ogl.DEPTH_TEST);

	do {
		
		Ogl.color(1);
		Ogl.callList(fractalCube);
//		Axis(1);
	} while ( !(yield) );

}



var scene4 = new function() {

	var tId = Ogl.genTexture();
	Ogl.bindTexture(TEXTURE_2D, tId);

//	var tb = Ogl.CreateTextureBuffer();
//	Ogl.Test(tb);

	do {
		
		Ogl.color(1);
		Ogl.rotate(90, 0,1,0);
		Ogl.bindTexture(vTEXTURE_2D, tId);
		quad();
	} while ( !(yield) );

}


/*
function HWConvolution( width, height, matrix ) {
	
	with (Ogl) {

		drawBuffer(BACK);
		viewport(0, 0, width, height);

		trace( 'VIEWPORT', getInteger( VIEWPORT, 4 ) );

		matrixMode(PROJECTION);
		loadIdentity();
		ortho(0, width, 0, height, -1, 1);
		matrixMode(MODELVIEW);
		loadIdentity();


		disable(DEPTH_TEST);
		enable(TEXTURE_2D);
		enable(BLEND);
		blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
//		BlendFunc(ONE, ONE);
		texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR); // NEAREST
		texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
		texEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	
		clearColor(0,0,0,1);
		clear(COLOR_BUFFER_BIT);
		shadeModel(FLAT);

		for ( var [y, line] in Iterator(matrix) ) {
			for ( var [x, ratio] in Iterator(line) ) {
				
				color(1,1,1, ratio);
				begin(QUADS);
				texCoord(0, 0); vertex(x, y);
				texCoord(1, 0); vertex(width+x, y);
				texCoord(1, 1); vertex(width+x, height+y);
				texCoord(0, 1); vertex(x, height+y);
				end();
			}
		}
		
		
		halt();
		
		var tId = genTexture();
		bindTexture(TEXTURE_2D, tId);
		readBuffer(BACK);
		copyTexImage2D(TEXTURE_2D, 0, RGBA, 0, 0,width, height, 0);
		
		bindFramebuffer(FRAMEBUFFER, 0);
		deleteRenderbuffer(renderbuffer);
		return tId;
	}
}
*/



function HWBlur() {
	
	with (Ogl) {
	
		// DBD chack display height and width.

		var [,,width, height] = getInteger(VIEWPORT, 4);
		
		var tId = genTexture();
		bindTexture(TEXTURE_2D, tId);
		copyTexImage2D(TEXTURE_2D, 0, RGB, 0, 0, width, height, 0);
		
		matrixMode(PROJECTION);
		loadIdentity();
		ortho(0, width, 0, height, -1, 1);
		matrixMode(MODELVIEW);
		loadIdentity();
		
		disable(DEPTH_TEST);
		enable(TEXTURE_2D);
		enable(BLEND);
		blendFunc(ONE, ONE);
		texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, LINEAR); // NEAREST
		texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, LINEAR);
		texEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
		shadeModel(FLAT);
		
		clearColor(0,0,0,1);
		clear(COLOR_BUFFER_BIT);
		
		var factors = [5,12,15,12,5];
		
	
		var y = 0;
		for ( var x = -2; x <= +2; x++ ) {
			
			color(factors[x+2]/49);
			begin(QUADS);
			texCoord(0, 0); vertex(x, y);
			texCoord(1, 0); vertex(width+x, y);
			texCoord(1, 1); vertex(width+x, height+y);
			texCoord(0, 1); vertex(x, height+y);
			end();
		}	

		var x = 0;
		for ( var y = -y; y <= +2; y++ ) {
			
			color(factors[x+2]/49);
			begin(QUADS);
			texCoord(0, 0); vertex(x, y);
			texCoord(1, 0); vertex(width+x, y);
			texCoord(1, 1); vertex(width+x, height+y);
			texCoord(0, 1); vertex(x, height+y);
			end();
		}	
		
		deleteTexture(tId);
	}
	
//	new File('myImage.png').content = EncodePngImage(Ogl.RenderToImage());
	
}


/*
var textureId = Ogl.genTexture();
Ogl.bindTexture(Ogl.TEXTURE_2D, textureId);
var texture = decodeJpegImage(new File('image.jpg').open(File.RDONLY))
Ogl.defineTextureImage(Ogl.TEXTURE_2D, undefined, texture);
Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
*/



var sleep = 20;
for (var end = false; !end ;) {
	
	t1 = timeCounter();
	print( (1000/(t1-t-sleep)).toFixed(), 'fps     \r' );
	t = t1;
	
	isCameraMoving = false;
	while (pollEvent(listeners));

	var tmp = Math.cos(vy/400);
	cx = -Math.cos(vx/400)*Math.abs(tmp) * vz;
	cy = Math.sin(vx/400)*Math.abs(tmp) * vz;
	cz = Math.sin(vy/400) * vz;

	Ogl.matrixMode(Ogl.MODELVIEW);
	Ogl.loadIdentity();
	Ogl.lookAt(cx,cy,cz, 0,0,0, 0,0,1);

	Ogl.clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
	

//	Scene1();
//	Scene2.next();
	scene3.next();
//	Scene4.next();


/*
	Ogl.matrixMode(Ogl.PROJECTION);
	Ogl.loadIdentity();
	Ogl.matrixMode(Ogl.MODELVIEW);
	Ogl.loadIdentity();

	mat.load(Ogl);
	mat.product(perspective);
	mat.invert();
	

//	Ogl.Enable(Ogl.TEXTURE_2D);
	
//	Ogl.BindTexture(Ogl.TEXTURE_2D, textureId);

	//color(1,1,1);
	Ogl.scale(1, -1);
	Ogl.begin(Ogl.QUADS);
	Ogl.texCoord(0, 0); Ogl.vertex(-1, -1);
	Ogl.texCoord(1, 0); Ogl.vertex(+1, -1);
	Ogl.texCoord(1, 1); Ogl.vertex(+1, +1);
	Ogl.texCoord(0, 1); Ogl.vertex(-1, +1);
	Ogl.end();
	
	HWBlur();
*/

	for each ( var [p1, p2] in lines ) {

		Ogl.begin(Ogl.LINES);
		Ogl.color(1,1,1); Ogl.vertex( p1[0], p1[1], p1[2] ); Ogl.vertex( p2[0], p2[1], p2[2] );
		Ogl.end();
	}

	glSwapBuffers();
	sleep(sleep);
}

























halt(); //////////////////////////////////////////////////////////////////////

var t = new Transformation(null);
t.lookAt(0,0,0, 1,0,0, 0,0,1);
t.translate(0, 0, 1);

var v = [0,0,0];
print( [c.toFixed(2) for each ( c in t.transformVector(v) )].join('  '), '\n' );


halt(); //////////////////////////////////////////////////////////////////////


var t = new Transformation(null);
t.translate(0, 0, 0);
t.rotateZ(-90);
t.translate(2, 0, 0);
t.rotateY(360);
t.rotateZ(45);
t.rotate(45, 0,0,1);
t.translate(1, 0, 0);
t.rotateY(90);
t.translate(1, 0, 0);
t.rotateY(360);
t.translate(1, 0, 0);
t.rotateX(360);
t.translate(1, 0, 0);
t.rotateY(123);
t.translate(5, 6, 7);
t.translate(-5, -6, -7);

var v = [0,0,0];
print( [c.toFixed(2) for each ( c in t.transformVector(v) )].join('  '), '\n' );
t.invert();
print( [c.toFixed(1) for each ( c in t.transformVector(v) )].join('  '), '\n' );
print( '\n\n' );




halt(); //////////////////////////////////////////////////////////////////////





for ( var i = 0; i < 16; i++ )
	print( i%4 ? ' ' : '  ', t[i].toFixed(2) );
print( '\n' );





halt();

var t = new Transformation(undefined);
t.lookAt(-10,-10,10, 0,0,0, 0,0,1);
		
Transformation.test();
halt();



var t = new Transformation( undefined );
t.lookAt(1,1,10, 0,0,0);

dumpMatrix(t);

halt();


/*
function quad2D(x0, y0, x1, y1) {

	with (Ogl) {
		begin(QUADS);
		texCoord( 0, 0 );
		vertex( x0, y0 );
		texCoord( 1, 0 );
		vertex( x1, y0 );
		texCoord( 1, 1 );
		vertex( x1, y1 );
		texCoord( 0, 1 );
		vertex( x0, y1 );
		end();
	}
}
*/

function line(x0, y0, z0, x1, y1, z1) {

	with (Ogl) {
	
		begin(LINES);
		vertex(x0, y0, z0);
		vertex(x1, y1, z1);
		end();
	}
}


function cube() {

	with (Ogl) {
	
		begin(QUADS);
		texCoord(0.0, 0.0); vertex(-1.0, -1.0,  1.0);
		texCoord(1.0, 0.0); vertex( 1.0, -1.0,  1.0);
		texCoord(1.0, 1.0); vertex( 1.0,  1.0,  1.0);
		texCoord(0.0, 1.0); vertex(-1.0,  1.0,  1.0);

		texCoord(1.0, 0.0); vertex(-1.0, -1.0, -1.0);
		texCoord(1.0, 1.0); vertex(-1.0,  1.0, -1.0);
		texCoord(0.0, 1.0); vertex( 1.0,  1.0, -1.0);
		texCoord(0.0, 0.0); vertex( 1.0, -1.0, -1.0);

		texCoord(0.0, 1.0); vertex(-1.0,  1.0, -1.0);
		texCoord(0.0, 0.0); vertex(-1.0,  1.0,  1.0);
		texCoord(1.0, 0.0); vertex( 1.0,  1.0,  1.0);
		texCoord(1.0, 1.0); vertex( 1.0,  1.0, -1.0);

		texCoord(1.0, 1.0); vertex(-1.0, -1.0, -1.0);
		texCoord(0.0, 1.0); vertex( 1.0, -1.0, -1.0);
		texCoord(0.0, 0.0); vertex( 1.0, -1.0,  1.0);
		texCoord(1.0, 0.0); vertex(-1.0, -1.0,  1.0);

		texCoord(1.0, 0.0); vertex( 1.0, -1.0, -1.0);
		texCoord(1.0, 1.0); vertex( 1.0,  1.0, -1.0);
		texCoord(0.0, 1.0); vertex( 1.0,  1.0,  1.0);
		texCoord(0.0, 0.0); vertex( 1.0, -1.0,  1.0);

		texCoord(0.0, 0.0); vertex(-1.0, -1.0, -1.0);
		texCoord(1.0, 0.0); vertex(-1.0, -1.0,  1.0);
		texCoord(1.0, 1.0); vertex(-1.0,  1.0,  1.0);
		texCoord(0.0, 1.0); vertex(-1.0,  1.0, -1.0);
		end();
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
tremeshTransformation.clear();


function init() {

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

	t.defineVertexBuffer(vertex);
	t.defineColorBuffer(color);
	t.defineIndexBuffer(index);
	
	tremeshId = Ogl.loadTrimesh(t);
	

	with (Ogl) {
		enable(DEPTH_TEST);
		enable(BLEND); //enable alpha blending
		blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA); //Set the blend function
	  enable(CULL_FACE);
	  
  	loadIdentity();
	  pushMatrix(); 
	 }
}

function draw() {

	with (Ogl) {
	
		var t = new Date().valueOf() - time0;

		loadMatrix(tremeshTransformation);

		translate(-0.5, -0.5, -0.5);
		clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
		drawTrimesh(tremeshId);
	};

	var rotation = new Transformation();
	rotation.clear();
	rotation.rotateToVector( -speedX, speedY, 1 );
	tremeshTransformation.product(rotation);


}

glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
glSetAttribute( GL_DOUBLEBUFFER, 1 );
glSetAttribute( GL_DEPTH_SIZE, 16 );

setVideoMode( 320, 200, 32, HWSURFACE | OPENGL | RESIZABLE ); // | ASYNCBLIT // RESIZABLE FULLSCREEN
Ogl.perspective( 90, 0.01, 1000 );

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
		Ogl.viewport(0, 0, w, h);
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

init();

while ( !done ) {

	pollEvent(listeners);
	draw();	
	glSwapBuffers();
	sleep(10);
}




halt(); //////////////////////////////////////////////////////////////////////////////////////////////////////////

	var camera = new Transformation;
	camera.clear();

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
			1/ Object Coordinates are transformed by the modelView matrix to produce Eye Coordinates.
			2/ Eye Coordinates are transformed by the projection matrix to produce Clip Coordinates.
			3/ Clip coordinate X, Y, and Z are divided by Clip coordinate W to produce Normalized Device Coordinates.
			4/ Normalized Device Coordinates are scaled and translated by the viewport parameters to produce Window Coordinates.
		*/
		
		// http://www.gamedev.net/community/forums/topic.asp?topic_id=480322
		/*
			expand to 3D homogenous coordinate. That is make a vector (x, y, z, 1). See below for Z-value.
			Map the X- and Y-value into normalized device coordinates, that is, from [0, viewport size] to the range [-1, 1].
			multiply by the inverse of the projection matrix.
			multiply by the inverse of the modelview matrix.
			perform perspective division.
			The result after perspective division is world space coordinates. skip the multiplication by the modelview matrix and you get the result in eye space instead.
		*/		
		
		var m = new Transformation();

		var p = new Transformation();
		Ogl.matrixMode(Ogl.PROJECTION);
		p.load(Ogl);
		p.invert();
		
		var c = new Transformation();
		c.load(camera);
		c.invert();
	
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
		c.transformVector(vect);
		
		vect[0] /= p[0];
		vect[1] /= p[5];
		

		print( 'world space coordinates vect '+[ v.toFixed(1) for each (v in vect) ].join(' '), '\n' );

		
		var obj = {};
		obj.createTime = timeCounter();
		obj.x = vect[0];
		obj.y = vect[1];
		obj.z = vect[2]-1;
		objects.push(obj);
	}

/*
Line3 viewport::ray( int winx, int winy ) {

  vector4 in( -1 + 2 * ( (float)winx / (float)( _width - _offsetWidth ) ), -1 + 2 * (float)( _height - winy - _offsetHeight ) / (float)(_height - _offsetHeight ), -1, 1 );

  in.x /= projectionMatrix()[0];
  in.y /= projectionMatrix()[5];

  matrix44 invView = camera().Transformation();
  invView.invert();

  vector4 out = invView( in ); ...

												  vector4 operator()( const vector4 &vector ) const {
												    
													 return vector4( _m[0]*vector.x + _m[4]*vector.y + _m[8]*vector.z  + _m[12]*vector.w,
																		  _m[1]*vector.x + _m[5]*vector.y + _m[9]*vector.z  + _m[13]*vector.w,
																		  _m[2]*vector.x + _m[6]*vector.y + _m[10]*vector.z + _m[14]*vector.w,
																		  _m[3]*vector.x + _m[7]*vector.y + _m[11]*vector.z + _m[15]*vector.w );
												  }
  

//  out.Rescale(); // should be done if needed

  point3 position( invView.translation() );
  vector3 direction( out.x, out.y, out.z );
  direction -= position;
	return Line3( position, direction );
}
*/




/*
function resizeWindow(w, h) {

	with (Ogl) {
		viewport(0,0,w,h);
		matrixMode(PROJECTION);
		loadIdentity();
		ortho(0,0,10,10, -10, 10);
//		Perspective( 90, -1000, 1000 );
	}
	render();
}

win.onsize = resizeWindow;		
*/

win.open();
win.createOpenGLContext();


with (Ogl) {

//	ShadeModel(SMOOTH);
	shadeModel(FLAT);
	frontFace(CCW);

	clearColor(0.1,0.15,0.2,1);	

	clearDepth(1);
//	Enable(DEPTH_TEST); // !!!!
	depthFunc(LESS);

//	DepthRange(1000, -1000);
	
	hint(PERSPECTIVE_CORRECTION_HINT, NICEST);
	hint(LINE_SMOOTH_HINT, DONT_CARE);		
	enable(LINE_SMOOTH);
	lineWidth(1);

	enable( TEXTURE_2D );
	bindTexture( TEXTURE_2D, genTexture() );
	texParameter(TEXTURE_2D, TEXTURE_MIN_FILTER, NEAREST); // GL_LINEAR
	texParameter(TEXTURE_2D, TEXTURE_MAG_FILTER, NEAREST);
	texEnv(TEXTURE_ENV, TEXTURE_ENV_MODE, MODULATE);
	texEnv(TEXTURE_ENV, TEXTURE_ENV_COLOR, [0,0,0,0]);
	

//	var texture = new Jpeg(new File('R0010235.JPG').Open( File.RDONLY )).Load();
	var texture = new Texture(128, 128, 1);
	texture.Set([0]);
//	var curveGaussian = function(c) { return function(x) { return Math.exp( -(x*x)/(2*c*c) ) } }
	texture.addGradiantRadial( curveGaussian( 0.3 ), false );
	texture.addNoise(0.1,0,0,0);

//	Enable(ALPHA_TEST);
//	AlphaFunc( GREATER, 0.5 );

	enable(BLEND);
//	BlendFunc(ONE, ONE);
	blendFunc(SRC_ALPHA, ONE_MINUS_SRC_ALPHA);
	
	defineTextureImage( TEXTURE_2D, ALPHA, texture );

// http://www.informit.com/articles/article.aspx?p=770639&seqNum=7
//		PointParameter( POINT_SIZE_MIN, 0 );
//		PointParameter( POINT_SIZE_MAX, 1 );
	pointParameter( POINT_DISTANCE_ATTENUATION, [0, 0, 0.01] ); // 1/(a + b*d + c *d^2)
	texEnv(POINT_SPRITE, COORD_REPLACE, TRUE);

	matrixMode(PROJECTION);
	loadIdentity();
	perspective(60, 0.5, 100);
}


const degToRad = 360/Math.PI;

camera.translate(0, 0, -20);
//camera.Rotate(45, 1,0,0);

function render(imgIndex) {
	with (Ogl) {

		clear(COLOR_BUFFER_BIT | DEPTH_BUFFER_BIT);
	
		matrixMode(MODELVIEW);
		loadMatrix(camera);
		quad();
	
		enable( TEXTURE_2D );
		color(1,1,1,1);
		for each ( var obj in objects ) {
			
			var t = timeCounter() - obj.createTime;
/*
			pointSize((-Math.cos(t/1000)+1)*10);
			begin(POINTS);
			vertex(obj.x, obj.y);
			end();	
*/

			pushMatrix();
			translate( obj.x, obj.y, obj.z );
			axis(1);
			popMatrix();
			

		}
	}
}


for (var i=0; !_quit; i++) {

	maybeCollectGarbage();
	Ogl.viewport(0,0,w,h);
	
	var t0 = timeCounter();
	render(i);
//	print( (1000/(TimeCounter() - t0)).toFixed(), ' fps\n' );
	win.swapBuffers();
	win.processEvents();
	sleep(10);
}

