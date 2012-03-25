loadModule('jssdl');
loadModule('jsgraphics');
loadModule('jsfont');
loadModule('jsoglft');
//loadModule('jsimage');
loadModule('jsprotex');
//loadModule('jstrimesh');
loadModule('jssvg');


function frustumPlanes() {

	// see http://www.gamedev.net/community/forums/topic.asp?topic_id=350620&whichpage=1&#2294920
	
	var t = new Transformation(Ogl.getDouble(Ogl.MODELVIEW_MATRIX, 16));
	t.product(Ogl.getDouble(Ogl.PROJECTION_MATRIX, 16));
	return [
		[ t[3] + t[0], t[7] + t[4], t[11] + t[ 8], t[15] + t[12] ], // left
		[ t[3] - t[0], t[7] - t[4], t[11] - t[ 8], t[15] - t[12] ], // right
		[ t[3] + t[1], t[7] + t[5], t[11] + t[ 9], t[15] + t[13] ], // bottom
		[ t[3] - t[1], t[7] - t[5], t[11] - t[ 9], t[15] - t[13] ], // top
		[ t[3] + t[2], t[7] + t[6], t[11] + t[10], t[15] + t[14] ], // near
		[ t[3] - t[2], t[7] - t[6], t[11] - t[10], t[15] - t[14] ]  // far
	];
}

var f3d = new Font3D(new Font('c:\\windows\\fonts\\arial.ttf'), Font3D.OUTLINE, 9);
var f2d = new Font3D(new Font('c:\\windows\\fonts\\arial.ttf'), Font3D.GRAYSCALE, 9); // TRANSLUCENT

function drawText(text, infrontFct) {

	Ogl.pushAttrib(Ogl.ENABLE_BIT | Ogl.POLYGON_BIT);
	if ( infrontFct ) {

		Ogl.disable(Ogl.DEPTH_TEST, Ogl.LIGHTING);
		Ogl.pushMatrix();
		Ogl.loadIdentity();
		infrontFct();
	}
	Ogl.disable(Ogl.CULL_FACE);
	var str = text;
	f3d.setColor();
	f3d.draw(text, -f3d.width(text)/2, f3d.height);
	if ( infrontFct )
		Ogl.popMatrix();
	Ogl.popAttrib();
}

function drawGrid() {

	if ( !arguments.callee.geometry ) {
		
		arguments.callee.geometry = Ogl.newList(false);

		Ogl.enable(Ogl.DEPTH_TEST);
		Ogl.enable(Ogl.BLEND);
		Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		
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

function drawFullQuad() {
	
	if ( arguments.callee.fullQuad ) {

		Ogl.callList(arguments.callee.fullQuad);
		return;
	}
	arguments.callee.fullQuad = Ogl.newList();
	Ogl.fullQuad()
	Ogl.endList();
}

function drawCube(edged, exNorm) {

	function face() {

		Ogl.begin(Ogl.QUADS);
		exNorm || Ogl.normal(0, 0, -1);
		exNorm && Ogl.normal(1, 1, -1);
		Ogl.texCoord(0, 1);
		Ogl.vertex( .5, .5, -0.5);
		exNorm && Ogl.normal(1, -1, -1);
		Ogl.texCoord(0, 0);
		Ogl.vertex( .5,-.5, -0.5);
		exNorm && Ogl.normal(-1, -1, -1);
		Ogl.texCoord(1, 0);
		Ogl.vertex(-.5,-.5, -0.5);
		exNorm && Ogl.normal(-1, 1, -1);
		Ogl.texCoord(1, 1);
		Ogl.vertex(-.5, .5, -0.5);
		Ogl.end();
	}
	
	function edge() {
		if ( !edged )
			return;
		Ogl.begin(Ogl.QUADS);
		Ogl.normal(1, 0, 0); Ogl.vertex(.5, .5, -.5);
		Ogl.normal(1, 0, 0); Ogl.vertex(.5,-.5, -.5);
		Ogl.normal(0, 0,-1); Ogl.vertex(.5,-.5, -.5);
		Ogl.normal(0, 0,-1); Ogl.vertex(.5, .5, -.5);
		Ogl.end();
	}
	
	Ogl.rotate(90, 0,1,0); face(); edge();
	Ogl.rotate(90, 0,1,0); face(); edge();
	Ogl.rotate(90, 0,1,0); face(); edge();
	Ogl.rotate(90, 0,1,0); face(); edge();
	Ogl.rotate(90, 1,0,0); face(); edge();
	Ogl.rotate(90, 0,0,1); edge();
	Ogl.rotate(90, 0,0,1); edge();
	Ogl.rotate(90, 0,0,1); edge();
	Ogl.rotate(180, 1,0,0); face(); edge();
	Ogl.rotate(90, 0,0,1); edge();
	Ogl.rotate(90, 0,0,1); edge();
	Ogl.rotate(90, 0,0,1); edge();
}


var shaderProgramProto = {

	addShader:function( source, type ) {
		
		if ( type == Ogl.FRAGMENT_SHADER ) {
			assert( Ogl.hasExtensionName('GL_ARB_fragment_shader') );
			this._hasFragmentShader = true;
		} else if ( type == Ogl.VERTEX_SHADER ) {
			assert( Ogl.hasExtensionName('GL_ARB_vertex_shader') );
		}

		if ( !this.program ) {
			
			assert( Ogl.hasExtensionName('GL_ARB_shading_language_100', 'GL_ARB_shader_objects') );
			this.program = Ogl.createProgramObject();
		}
		
		var shader = Ogl.createShaderObject(type);
		Ogl.shaderSource(shader, source);
		Ogl.compileShader(shader);
		if ( !Ogl.getObjectParameter(shader, Ogl.OBJECT_COMPILE_STATUS) ) {

			print( 'CompileShader log:\n', Ogl.getInfoLog(shader), '\n' );
			throw 0;
		}
		Ogl.attachObject(this.program, shader);
		Ogl.deleteObject(shader);
		if ( !Ogl.getObjectParameter(shader, Ogl.OBJECT_DELETE_STATUS) ) {

			print( 'DeleteObject log:\n', Ogl.getInfoLog(this.program), '\n' );
			throw 0;
		}
	},
	
	addFragmentShader:function( source ) {

		this.addShader(source, Ogl.FRAGMENT_SHADER);
	},

	addVertexShader:function( source ) {

		this.addShader(source, Ogl.VERTEX_SHADER);
	},
	
	link:function() {
	
		if ( !this._hasFragmentShader && Ogl.hasExtensionName('GL_ARB_fragment_shader') )
			this.addFragmentShader('void main(void) {gl_FragColor=gl_Color;}');

		Ogl.linkProgram(this.program);
		if ( !Ogl.getObjectParameter(this.program, Ogl.OBJECT_LINK_STATUS) ) {

			print( 'LinkProgram log:\n', Ogl.getInfoLog(this.program), '\n' );
			throw 0;
		}
	},
	
	_uniformLocationCache:{},
	
	Set:function(name, value) {
	
		var loc = this._uniformLocationCache[name];
		if ( !loc )
			this._uniformLocationCache[name] = loc = Ogl.getUniformLocation(this.program, name);
		Ogl.uniform(loc, value);
	},
	
	setUniformMatrix:function( name, value ) {
	
		var loc = this._uniformLocationCache[name];
		if ( !loc )
			this._uniformLocationCache[name] = loc = Ogl.getUniformLocation(this.program, name);
		Ogl.uniformMatrix(loc, value);
	},

	on:function() {
	
		Ogl.useProgramObject(this.program);
	},
	
	off:function() {
	
		Ogl.useProgramObject(0);
	}
}


function oglTexture2D() {

	this.texture = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, this.texture);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);

	this.loadImage = function(image, format) {
	
		Ogl.bindTexture(Ogl.TEXTURE_2D, this.texture);
		Ogl.defineTextureImage(Ogl.TEXTURE_2D, format, image);
	}

	this.loadSVG = function(svgData, width, height) {

		var svg = new SVG();	
		svg.write(svgData);
		var image = svg.renderImage(width, height, 4, true);
		Ogl.bindTexture(Ogl.TEXTURE_2D, this.texture);
		Ogl.defineTextureImage(Ogl.TEXTURE_2D, undefined, image);
		image.free();
	}
	
	this.valueOf = function() this.texture;
}


function Light( oglLight ) {
	
	this.position = [0,0,1,1];
	this.aim = [0,0,0];
	this.cutoff = 30;
	
	var tmp = [];
	
	this.update = function() {

		Ogl.Light(oglLight, Ogl.POSITION, this.position);
		vec3Sub(this.aim, this.position, tmp);
		Ogl.Light(oglLight, Ogl.SPOT_DIRECTION, tmp);
	}
	
	this.setPosition = function(x,y,z) {

		this.position[0] = x;
		this.position[1] = y;
		this.position[2] = z;
		this.update();
	}
	
	this.setAim = function(x,y,z) {
		
		this.aim[0] = x;
		this.aim[1] = y;
		this.aim[2] = z;
		this.update();
	}
	
	this.enableProjectorTextureCoordinates = function() {

		Ogl.pushMatrix();
		Ogl.loadIdentity();
		Ogl.translate(0.5, 0.5, 0.5);
		Ogl.scale(0.5);
		Ogl.perspective(this.cutoff * 2, 1, 1, 100); // Ogl.GetLight(oglLight, Ogl.SPOT_CUTOFF);
		Ogl.lookAt( this.position[0], this.position[1], this.position[2],  this.aim[0], this.aim[1], this.aim[2],  0, 0, 1 );
		var mat = Ogl.get(Ogl.MODELVIEW_MATRIX);
		Ogl.popMatrix();

		//Set up texture coordinate generation.
		Ogl.texGen(Ogl.S, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[4], mat[8], mat[12]);
		Ogl.texGen(Ogl.T, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.T, Ogl.EYE_PLANE, mat[1], mat[5], mat[9], mat[13]);
		//Ogl.TexGen(Ogl.R, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		//Ogl.TexGen(Ogl.R, Ogl.EYE_PLANE, mat[2], mat[6], mat[10], mat[14]);
		Ogl.texGen(Ogl.Q, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.Q, Ogl.EYE_PLANE, mat[3], mat[7], mat[11], mat[15]);

		Ogl.enable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_Q);
		//Ogl.Enable(Ogl.TEXTURE_GEN_R);
	}

	this.disableProjectorTextureCoordinates = function() {
	
		Ogl.disable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_Q);
		//Ogl.Disable(Ogl.TEXTURE_GEN_R);
	}	
	
	Ogl.Light(oglLight, Ogl.DIFFUSE, 1, 1, 1, 1);
	Ogl.Light(oglLight, Ogl.AMBIENT, 0, 0, 0, 0.95);
	//Ogl.Light(oglLight, Ogl.CONSTANT_ATTENUATION, 0.1);
	//Ogl.Light(oglLight, Ogl.LINEAR_ATTENUATION, 0.003);
	//Ogl.Light(oglLight, Ogl.SPOT_EXPONENT, 0);
	Ogl.Light(oglLight, Ogl.SPOT_CUTOFF, this.cutoff);
	Ogl.enable(oglLight);
}


////

function UI(currentWidth, currentHeight) {
	
	var _this = this;

	currentWidth = currentWidth || desktopWidth / 4;
	currentHeight = currentHeight || desktopHeight / 4;
	var defaultVideoMode = OPENGL | RESIZABLE;

	unicodeKeyboardTranslation = true;
	keyRepeatDelay = 300;
	keyRepeatInterval = 50;
	maxFPS = 60;
		
	this.frame = 0;

	glSetAttribute( GL_DOUBLEBUFFER, 1 );
	glSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	glSetAttribute( GL_DEPTH_SIZE, 32);
	glSetAttribute( GL_STENCIL_SIZE, 8 );
//	GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
	
	setVideoMode(currentWidth, currentHeight, undefined, defaultVideoMode);
	
	print( 'OpenGL v', Ogl.getString(Ogl.VERSION), '\n' );
	
	//	Assert( Ogl.HasExtensionName('GL_EXT_stencil_two_side') );
	assert( Ogl.hasExtensionName('GL_ARB_texture_env_combine') );

	Ogl.hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
	Ogl.hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
	Ogl.hint(Ogl.POLYGON_SMOOTH_HINT, Ogl.NICEST);

	Ogl.lightModel(Ogl.LIGHT_MODEL_LOCAL_VIEWER, 1); // see. http://gregs-blog.com/2007/12/21/theres-nothing-wrong-with-opengls-specular-lighting/

	Ogl.pixelStore(Ogl.UNPACK_ALIGNMENT, 1);

	Ogl.enable(Ogl.CULL_FACE);  // default: Ogl.FrontFace(Ogl.CCW);  Ogl.CullFace(Ogl.BACK);
	Ogl.enable(Ogl.DEPTH_TEST);

	//	Ogl.Enable(Ogl.TEXTURE_2D);
	//	Ogl.ShadeModel(Ogl.SMOOTH);
	//	Ogl.Material(Ogl.FRONT, Ogl.SPECULAR, [1.0, 1.0, 1.0, 1.0]);
	//	Ogl.Material(Ogl.FRONT, Ogl.SHININESS, 50);
	//	Ogl.ColorMaterial ( Ogl.FRONT_AND_BACK, Ogl.AMBIENT_AND_DIFFUSE ) ;
	//	Ogl.Enable( Ogl.COLOR_MATERIAL );
	//	Ogl.Enable(Ogl.BLEND);
	//	Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);

	ShadowVolumeProgram.prototype = shaderProgramProto;
	function ShadowVolumeProgram( light ) {

		var source = <><![CDATA[
			
			float far = 1000.0;
			uniform bool hasFog;

			void main(void) {
			
				vec3 lightDir = (gl_ModelViewMatrix * gl_Vertex - gl_LightSource[$(lightIndex)].position).xyz;
				if ( dot(lightDir, gl_NormalMatrix * gl_Normal) < 0.001 ) { // if vertex is lit
				
					gl_Position = ftransform();
				} else {

					vec4 fin = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex + vec4(normalize(lightDir) * far, 0.0));
					if ( fin.z > fin.w ) // avoid clipping
						fin.z = fin.w; // move to the far plane
					gl_Position = fin;
				}
				
				if ( hasFog ) {

					float alpha = sin((gl_NormalMatrix * gl_Normal).z * 1.57 ) * gl_Fog.scale * 20.0;
					gl_FrontColor = vec4(gl_Fog.color.rgb, alpha);
				}
			}
		]]></>.toString();
		
		this.addVertexShader(expand(source, { lightIndex: light-Ogl.LIGHT0 }));
		this.link();
	}


	LightConeProgram.prototype = shaderProgramProto;
	function LightConeProgram( light ) {

		var source = <><![CDATA[

			const float pi = 3.14159265;
			uniform mat4 lightMatrix;
			
			gl_LightSourceParameters light = gl_LightSource[$(lightIndex)];
			
			void main(void) {
				// see Intersection of a line and a Cone: http://www.hodge.net.au/sam/blog/?p=61=1

				gl_Position = ftransform();

				vec4 v = gl_ModelViewMatrix * gl_Vertex;

				float d = length(v.xyz - gl_LightSource[$(lightIndex)].position.xyz);
			   float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * d + light.quadraticAttenuation * d * d);
			   
				float spotZ = abs(normalize(v - light.position).z);
			   float lightZ = abs((gl_NormalMatrix * gl_Normal).z);
			   
				gl_FrontColor = vec4(gl_Color.rgb, ((spotZ/4.0) + lightZ/2.0 ) * attenuation / 1.0); // * gl_Color.a
			}
		]]></>.toString();

		this.addVertexShader(expand(source, { lightIndex: light-Ogl.LIGHT0 }));
		this.link();
	}

	var lightConeProgram = new LightConeProgram(Ogl.LIGHT0);
	var shadowVolumeProgram = new ShadowVolumeProgram(Ogl.LIGHT0);
	
	var useTwoSideStencil = Ogl.hasExtensionName('GL_EXT_stencil_two_side', 'GL_EXT_stencil_wrap');
	var useSeparateStencil = Ogl.hasExtensionProc('glStencilOpSeparate');

	this.light = new Light(Ogl.LIGHT0);
	Ogl.enable(Ogl.LIGHTING);
		
	this.renderWithShadows = function( renderCallback ) {
	
/*
			Ogl.polygonMode(Ogl.FRONT_AND_BACK, Ogl.LINE);
			Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
			Ogl.color(1,1,1, 0.1)
			Ogl.enable(Ogl.CULL_FACE);
			Ogl.cullFace(Ogl.BACK);
			Ogl.enable(Ogl.BLEND);
			rays.on();
			renderCallback(3);
			rays.off();
			Ogl.disable(Ogl.BLEND);
		return;
*/		

		// see http://www.opengl.org/resources/code/samples/glut_examples/advanced/shadowvol.c	// http://www.angelfire.com/games5/duktroa/RealTimeShadowTutorial.htm
		// http://www.gamedev.net/columns/hardcore/cgshadow/page2.asp // http://joshbeam.com/articles/stenciled_shadow_volumes_in_opengl/
	
		Ogl.enable(Ogl.POLYGON_OFFSET_FILL);
		Ogl.polygonOffset(0, -32);
			renderCallback(6);
		Ogl.disable(Ogl.POLYGON_OFFSET_FILL);

		Ogl.disable(Ogl.LIGHTING);
		Ogl.depthMask(false);

		var hasFog = Ogl.isEnabled(Ogl.FOG);

		if ( hasFog ) {
		
			Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
			Ogl.enable(Ogl.BLEND);
			Ogl.enable(Ogl.CULL_FACE);
			Ogl.cullFace(Ogl.BACK);
		} else {

			Ogl.colorMask(false);
		}
		
		Ogl.stencilFunc(Ogl.ALWAYS, 0, 0);
		Ogl.enable(Ogl.STENCIL_TEST);
		Ogl.clear(Ogl.STENCIL_BUFFER_BIT);

		shadowVolumeProgram.on();
		shadowVolumeProgram.Set('hasFog', hasFog);
		
		Ogl.depthFunc(Ogl.LESS); // needed ???

		// see http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=149515
		if ( useSeparateStencil ) {

			Ogl.disable(Ogl.CULL_FACE);
			Ogl.stencilOpSeparate(Ogl.BACK, Ogl.KEEP, Ogl.INCR_WRAP, Ogl.KEEP);
			Ogl.stencilOpSeparate(Ogl.FRONT, Ogl.KEEP, Ogl.DECR_WRAP, Ogl.KEEP);
				renderCallback(3); // render occluders shape only
			Ogl.enable(Ogl.CULL_FACE);
		} else if ( useTwoSideStencil ) {

			Ogl.disable(Ogl.CULL_FACE);
			Ogl.enable(Ogl.STENCIL_TEST_TWO_SIDE);
			Ogl.activeStencilFaceEXT(Ogl.BACK);
			Ogl.stencilOp(Ogl.KEEP, Ogl.INCR_WRAP, Ogl.KEEP);
			Ogl.activeStencilFaceEXT(Ogl.FRONT);
			Ogl.stencilOp(Ogl.KEEP, Ogl.DECR_WRAP, Ogl.KEEP);
				renderCallback(3); // render occluders shape only
			Ogl.disable(Ogl.STENCIL_TEST_TWO_SIDE);
			Ogl.enable(Ogl.CULL_FACE);
		} else {

			Ogl.enable(Ogl.CULL_FACE);
			Ogl.stencilOp(Ogl.KEEP, Ogl.INCR, Ogl.KEEP);
			Ogl.cullFace(Ogl.FRONT);
			var list = Ogl.newList(false);
				renderCallback(3); // render occluders shape only
			Ogl.endList()
			Ogl.stencilOp(Ogl.KEEP, Ogl.DECR, Ogl.KEEP);
			Ogl.cullFace(Ogl.BACK);
				Ogl.callList(list);
			Ogl.deleteList(list);
		}

		shadowVolumeProgram.off();

		Ogl.depthFunc(Ogl.ALWAYS);
		Ogl.stencilFunc(Ogl.NOTEQUAL, 0, -1);
		Ogl.stencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.KEEP);

		Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.enable(Ogl.BLEND);
		
		Ogl.color(Ogl.getLight(Ogl.LIGHT0, Ogl.AMBIENT)); // use light's ambiant color as shadow color
		Ogl.colorMask(true);

/*
		if ( hasFog )
			renderCallback(5); ???
		else
			drawFullQuad();
*/
		drawFullQuad();

		Ogl.disable(Ogl.BLEND, Ogl.STENCIL_TEST);
		
		Ogl.depthMask(true);
		Ogl.depthFunc(Ogl.LEQUAL);
		
		if ( hasFog ) {
		
			Ogl.disable(Ogl.TEXTURE_2D);
			
			Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE);
			Ogl.color(1,1,1, 1);
			lightConeProgram.on();

			Ogl.enable(Ogl.CULL_FACE);
			Ogl.cullFace(Ogl.BACK);
			
			Ogl.enable(Ogl.BLEND);

			Ogl.translate(this.light.position[0], this.light.position[1], this.light.position[2]);
			Ogl.aimAt(this.light.aim[0]-this.light.position[0], this.light.aim[1]-this.light.position[1], this.light.aim[2]-this.light.position[2]);
			Ogl.depthMask(false);
			
			var h = 100;
			var r = Math.tan(Ogl.getLight(Ogl.LIGHT0, Ogl.SPOT_CUTOFF) / 180 * Math.PI ) * h;
			Ogl.drawCylinder(0, r, h, 64, 32);
			Ogl.depthMask(true);

			Ogl.disable(Ogl.BLEND);
			Ogl.enable(Ogl.CULL_FACE);
			Ogl.cullFace(Ogl.BACK);

			lightConeProgram.off();
			
			Ogl.color(0,0,0,1);
			Ogl.translate(0,0,-2);
			Ogl.drawCylinder(0, 2, 3, 16, 1);
		}
		
		Ogl.enable(Ogl.LIGHTING);
	}

	//////////

	this.status = 'status';
	
	this.projection = function() {
		
		Ogl.perspective(60, undefined, 0.5, 500);
		
//		// http://www.songho.ca/opengl/gl_projectionmatrix.html
//		dumpMatrix(Ogl.Get(Ogl.PROJECTION_MATRIX)); halt();
	}

	this.idle = function() { // default function
	}
	
	this.draw = function() { // default function

		Ogl.pushAttrib(Ogl.ENABLE_BIT);
		Ogl.disable(Ogl.CULL_FACE);
		var str = 'nothing to draw';
		Ogl.translate(-f3d.width(str)/2, 0, -80);
		f3d.draw(str);
		Ogl.popAttrib();
	}
	
	var fps = 0.;
	var fpsArray = [];
	
	var isInit = false;

	function surfaceReady() {
	
		if ( !isInit ) {

			this.init && this.init();
			isInit = true;
		}

		var t0 = timeCounter();
		Ogl.viewport(0, 0, videoWidth, videoHeight);
		Ogl.clearColor(0.15, 0.2, 0.4, 0);
		Ogl.clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);

		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.loadIdentity();
		this.projection();
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.loadIdentity();
	
		this.draw(this.frame);

		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.loadIdentity();
		Ogl.ortho(0, videoWidth, 0, videoHeight, 0, 1);
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.loadIdentity();
		Ogl.pushAttrib(Ogl.ENABLE_BIT);
		Ogl.disable(Ogl.LIGHTING);
		Ogl.disable(Ogl.DEPTH_TEST);
		Ogl.color(0);
		Ogl.begin(Ogl.QUADS);
		Ogl.vertex(0,0);  Ogl.vertex(videoWidth,0);  Ogl.vertex(videoWidth,16);  Ogl.vertex(0,16);
		Ogl.end();

		f2d.setBackgroundColor([0,0,0,0]);
		f2d.setColor([1]);
		
		fps = 0.;
		var len = fpsArray.length;
		for ( var i = 0; i < len; ++i )
			fps += fpsArray[i];
		fps = (fps / len).toFixed(1);
		
		
		var str = fps+'fps\t'+this.status;
		for ( var [i,chunk] in Iterator(str.split('\t')) )
			f2d.draw(chunk, 2 + i * 150, 2);
		Ogl.popAttrib();

		Ogl.finish();
		glSwapBuffers(true);
		this.idle();
		
		fps = 1000/(timeCounter()-t0);
		if ( fpsArray.length > 2 )
			fpsArray.shift();
		fpsArray.push(fps);
		
		this.frame++;
	}

	
	var keyObjListeners = {};

	this.keyState = new ObjEx(undefined, undefined, function(name) {
			
		var sym = global['K_'+name.toUpperCase()] || global['K_'+name.toLowerCase()];
		return getKeyState(sym);
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
	this.addEventListener = function(obj) {

		eventListenerList.push(obj);
	}
	this.removeEventListener = function(obj) {

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

				processEvents( surfaceReadyEvents() );
				if ( videoFlags & FULLSCREEN ) {
				
					setVideoMode(currentWidth, currentHeight, undefined, defaultVideoMode);
				} else {
				
					currentWidth = videoWidth;
					currentHeight = videoHeight;
					setVideoMode(desktopWidth, desktopHeight, undefined, defaultVideoMode | FULLSCREEN);
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
			
//			print('resize '+w+'x'+h, '\n')
//			SetVideoMode();
//			Ogl.Viewport(0, 0, w,h);
		},
	};

	this.loop = function( additionalEventCallback ) {
		
		while ( !endSignal ) {

			var events = [ endSignalEvents(), SDLEvents(listeners), surfaceReadyEvents.call(this, surfaceReady) ];
			additionalEventCallback && additionalEventCallback(events);
			processEvents.apply(this, events);
		}
	}
}





/* **************************************************************************************************** */
/* UNUSED CODE :


	var shadowMapSize = 128;
	var shadowMapTexture = Ogl.genTexture();
	Ogl.bindTexture(Ogl.TEXTURE_2D, shadowMapTexture);
	Ogl.texImage2D(Ogl.TEXTURE_2D, 0, Ogl.DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0, Ogl.DEPTH_COMPONENT, Ogl.UNSIGNED_BYTE, null);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR); // NEAREST
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_MODE, Ogl.COMPARE_R_TO_TEXTURE); //enable shadow comparison
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_FUNC, Ogl.LEQUAL); //shadow comparison should be true (ie not in shadow) if r<=texture
	Ogl.texParameter(Ogl.TEXTURE_2D, Ogl.DEPTH_TEXTURE_MODE, Ogl.INTENSITY); //shadow comparison should generate an INTENSITY result


	this.renderWithShadows1 = function( renderCallback ) {
		
		// http://www.paulsprojects.net/tutorials/smt/smt.html ( and http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html )
		// http://www.google.com/codesearch/p?hl=en#4FSOSMZ6Pxc/distfiles/MesaDemos-5.0.tar.bz2|w8yTKn7FXYw/Mesa-5.0/demos/shadowtex.c

		var mat = new Transformation(0.5, 0.0, 0.0, 0.0,  0.0, 0.5, 0.0, 0.0,  0.0, 0.0, 0.5, 0.0,  0.5, 0.5, 0.5, 1.0);

		Ogl.enable(Ogl.NORMALIZE);

// First pass - from light's point of view

		Ogl.pushAttrib( Ogl.VIEWPORT_BIT | Ogl.LIGHTING_BIT ); // | Ogl.ENABLE_BIT
		Ogl.viewport(0, 0, shadowMapSize, shadowMapSize);

		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.pushMatrix();
		Ogl.loadIdentity();
		Ogl.perspective(60, 1, 30, 50); //var lightFov = Ogl.GetLight(Ogl.LIGHT0, Ogl.SPOT_CUTOFF) * 2;
		mat.product(Ogl);
		
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.pushMatrix();
		Ogl.loadIdentity();
		Ogl.lookAt( lightPos[0], lightPos[1], lightPos[2],  lightDir[0], lightDir[1], lightDir[2],  0, 0, 1 );
		mat.product(Ogl);

		Ogl.disable(Ogl.TEXTURE_2D);
		Ogl.disable(Ogl.LIGHTING);
		Ogl.cullFace(Ogl.FRONT);
		Ogl.shadeModel(Ogl.FLAT);
		Ogl.colorMask(false, false, false, false);
//		Ogl.ClearDepth(1); // default is 1
		Ogl.depthFunc(Ogl.LEQUAL);
		Ogl.enable(Ogl.DEPTH_TEST);
		Ogl.clear(Ogl.DEPTH_BUFFER_BIT);

		Ogl.enable(Ogl.POLYGON_OFFSET_FILL);
//		Ogl.PolygonOffset(1, 2); // set the scale and units used to calculate depth values.
		
//		Ogl.DepthRange(0.5, 1);

		renderCallback(3); // render occluders + shape only
//		new File('zbuffer.png').content = EncodePngImage(Ogl.ReadImage(true, Ogl.DEPTH_COMPONENT));  throw 0;		
		
		Ogl.bindTexture(Ogl.TEXTURE_2D, shadowMapTexture);

//		Ogl.PixelTransfer(Ogl.DEPTH_BIAS, 0);
//		Ogl.PixelTransfer(Ogl.DEPTH_SCALE, 1);
		
		Ogl.copyTexSubImage2D(Ogl.TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSize, shadowMapSize);

		Ogl.colorMask(true, true, true, true);
		Ogl.shadeModel(Ogl.SMOOTH);
		Ogl.cullFace(Ogl.BACK);
		
		Ogl.popMatrix();
		Ogl.matrixMode(Ogl.PROJECTION);
		Ogl.popMatrix();
		Ogl.matrixMode(Ogl.MODELVIEW);
		Ogl.popAttrib();
		
//2nd pass - Draw from camera's point of view

		//use dim light to represent shadowed areas
		Ogl.Light(Ogl.LIGHT0, Ogl.POSITION, lightPos); // needed ?
		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 1.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 0, 0, 0, 1);
		
		Ogl.enable(Ogl.LIGHTING);
		Ogl.clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		
		renderCallback(4); // render objects that receive shadow

//3rd pass - Draw with bright light

		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1,1,1, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 1,1,1, 1);

		//Set up texture coordinate generation.
		Ogl.texGen(Ogl.S, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[4], mat[8], mat[12]);
		Ogl.texGen(Ogl.T, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.T, Ogl.EYE_PLANE, mat[1], mat[5], mat[9], mat[13]);
		Ogl.texGen(Ogl.R, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.R, Ogl.EYE_PLANE, mat[2], mat[6], mat[10], mat[14]);
		Ogl.texGen(Ogl.Q, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.texGen(Ogl.Q, Ogl.EYE_PLANE, mat[3], mat[7], mat[11], mat[15]);

		Ogl.enable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_R, Ogl.TEXTURE_GEN_Q);

		//bind & enable shadow map texture
		Ogl.bindTexture(Ogl.TEXTURE_2D, shadowMapTexture);  // Ogl.BindTexture(Ogl.TEXTURE_2D, testTexture);

		// ?? Each component is then multiplied by the signed scale factor GL_c_SCALE, added to the signed bias GL_c_BIAS, and clamped to the range [0,1] (see glPixelTransfer).

		//Set alpha test to discard false comparisons
		Ogl.alphaFunc(Ogl.GEQUAL, 0.99);
		Ogl.enable(Ogl.ALPHA_TEST);
		
		Ogl.enable(Ogl.TEXTURE_2D);

		renderCallback(6); // render all objects

		Ogl.disable(Ogl.TEXTURE_2D);
		Ogl.disable(Ogl.ALPHA_TEST);
		Ogl.disable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_R, Ogl.TEXTURE_GEN_Q);
	}





	var tmpShadowMatrix = [];
	this.renderWithShadows = function( renderCallback, plane ) {
		
		// see http://www.opengl.org/resources/code/samples/mjktips/TexShadowReflectLight.html

		Ogl.clear(Ogl.STENCIL_BUFFER_BIT);
		
		// Draw the floor with stencil value 3.  This helps us only draw the shadow once per floor pixel (and only on the floor pixels).
		Ogl.enable(Ogl.STENCIL_TEST);
		Ogl.stencilFunc(Ogl.ALWAYS, 3, -1);
		Ogl.stencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.REPLACE);

		renderCallback(4);
		
		// Render the projected shadow.
		Ogl.stencilFunc(Ogl.LESS, 2, -1);  // draw if ==1
		Ogl.stencilOp(Ogl.REPLACE, Ogl.REPLACE, Ogl.REPLACE);
		
		// To eliminate depth buffer artifacts, we use polygon offset to raise the depth of the projected shadow slightly so that it does not depth buffer alias with the floor.
		Ogl.polygonOffset(-2, -1); // set the scale and units used to calculate depth values.
      // Render 50% black shadow color on top of whatever the floor appareance is.
      Ogl.enable(Ogl.POLYGON_OFFSET_FILL, Ogl.BLEND);
      Ogl.blendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.disable(Ogl.LIGHTING); // Force the 50% black.
      Ogl.color(0.0, 0.0, 0.0, 0.5);
      Ogl.pushMatrix();
		Ogl.multMatrix(shadowMatrix(plane, lightPos, tmpShadowMatrix));

		renderCallback(3);
//		new File('tmp.png').content = EncodePngImage(Ogl.ReadImage(true, Ogl.RGB));  throw 0;

		Ogl.popMatrix();
      Ogl.enable(Ogl.LIGHTING);
      Ogl.disable(Ogl.BLEND, Ogl.POLYGON_OFFSET_FILL, Ogl.STENCIL_TEST);
	}



*/


