LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsfont');
LoadModule('jsoglft');
//LoadModule('jsimage');
LoadModule('jsprotex');
//LoadModule('jstrimesh');
LoadModule('jssvg');


function FrustumPlanes() {

	// see http://www.gamedev.net/community/forums/topic.asp?topic_id=350620&whichpage=1&#2294920
	
	var t = new Transformation(Ogl.GetDouble(Ogl.MODELVIEW_MATRIX, 16));
	t.Product(Ogl.GetDouble(Ogl.PROJECTION_MATRIX, 16));
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

function DrawText(text, infrontFct) {

	Ogl.PushAttrib(Ogl.ENABLE_BIT | Ogl.POLYGON_BIT);
	if ( infrontFct ) {

		Ogl.Disable(Ogl.DEPTH_TEST, Ogl.LIGHTING);
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

function DrawGrid() {

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

function DrawFullQuad() {
	
	if ( arguments.callee.FullQuad ) {

		Ogl.CallList(arguments.callee.FullQuad);
		return;
	}
	arguments.callee.FullQuad = Ogl.NewList();
	Ogl.FullQuad()
	Ogl.EndList();
}

function DrawCube(edged, exNorm) {

	function Face() {

		Ogl.Begin(Ogl.QUADS);
		exNorm || Ogl.Normal(0, 0, -1);
		exNorm && Ogl.Normal(1, 1, -1);
		Ogl.TexCoord(0, 1);
		Ogl.Vertex( .5, .5, -0.5);
		exNorm && Ogl.Normal(1, -1, -1);
		Ogl.TexCoord(0, 0);
		Ogl.Vertex( .5,-.5, -0.5);
		exNorm && Ogl.Normal(-1, -1, -1);
		Ogl.TexCoord(1, 0);
		Ogl.Vertex(-.5,-.5, -0.5);
		exNorm && Ogl.Normal(-1, 1, -1);
		Ogl.TexCoord(1, 1);
		Ogl.Vertex(-.5, .5, -0.5);
		Ogl.End();
	}
	
	function Edge() {
		if ( !edged )
			return;
		Ogl.Begin(Ogl.QUADS);
		Ogl.Normal(1, 0, 0); Ogl.Vertex(.5, .5, -.5);
		Ogl.Normal(1, 0, 0); Ogl.Vertex(.5,-.5, -.5);
		Ogl.Normal(0, 0,-1); Ogl.Vertex(.5,-.5, -.5);
		Ogl.Normal(0, 0,-1); Ogl.Vertex(.5, .5, -.5);
		Ogl.End();
	}
	
	Ogl.Rotate(90, 0,1,0); Face(); Edge();
	Ogl.Rotate(90, 0,1,0); Face(); Edge();
	Ogl.Rotate(90, 0,1,0); Face(); Edge();
	Ogl.Rotate(90, 0,1,0); Face(); Edge();
	Ogl.Rotate(90, 1,0,0); Face(); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
	Ogl.Rotate(180, 1,0,0); Face(); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
	Ogl.Rotate(90, 0,0,1); Edge();
}


var ShaderProgramProto = {

	AddShader:function( source, type ) {
		
		if ( type == Ogl.FRAGMENT_SHADER_ARB ) {
			Assert( Ogl.HasExtensionName('GL_ARB_fragment_shader') );
			this._hasFragmentShader = true;
		} else if ( type == Ogl.VERTEX_SHADER_ARB ) {
			Assert( Ogl.HasExtensionName('GL_ARB_vertex_shader') );
		}

		if ( !this.program ) {
			
			Assert( Ogl.HasExtensionName('GL_ARB_shading_language_100', 'GL_ARB_shader_objects') );
			this.program = Ogl.CreateProgramObjectARB();
		}
		
		var shader = Ogl.CreateShaderObjectARB(type);
		Ogl.ShaderSourceARB(shader, source);
		Ogl.CompileShaderARB(shader);
		if ( !Ogl.GetObjectParameterARB(shader, Ogl.OBJECT_COMPILE_STATUS_ARB) ) {

			Print( 'CompileShaderARB log:\n', Ogl.GetInfoLogARB(shader), '\n' );
			throw 0;
		}
		Ogl.AttachObjectARB(this.program, shader);
		Ogl.DeleteObjectARB(shader);
		if ( !Ogl.GetObjectParameterARB(shader, Ogl.OBJECT_DELETE_STATUS_ARB) ) {

			Print( 'DeleteObjectARB log:\n', Ogl.GetInfoLogARB(this.program), '\n' );
			throw 0;
		}
	},
	
	AddFragmentShader:function( source ) {

		this.AddShader(source, Ogl.FRAGMENT_SHADER_ARB);
	},

	AddVertexShader:function( source ) {

		this.AddShader(source, Ogl.VERTEX_SHADER_ARB);
	},
	
	Link:function() {
	
		if ( !this._hasFragmentShader && Ogl.HasExtensionName('GL_ARB_fragment_shader') )
			this.AddFragmentShader('void main(void) {gl_FragColor=gl_Color;}');

		Ogl.LinkProgramARB(this.program);
		if ( !Ogl.GetObjectParameterARB(this.program, Ogl.OBJECT_LINK_STATUS_ARB) ) {

			Print( 'LinkProgramARB log:\n', Ogl.GetInfoLogARB(this.program), '\n' );
			throw 0;
		}
	},
	
	_uniformLocationCache:{},
	
	Set:function(name, value) {
	
		var loc = this._uniformLocationCache[name];
		if ( !loc )
			this._uniformLocationCache[name] = loc = Ogl.GetUniformLocationARB(this.program, name);
		Ogl.UniformARB(loc, value);
	},
	
	SetUniformMatrix:function( name, value ) {
	
		var loc = this._uniformLocationCache[name];
		if ( !loc )
			this._uniformLocationCache[name] = loc = Ogl.GetUniformLocationARB(this.program, name);
		Ogl.UniformMatrixARB(loc, value);
	},

	On:function() {
	
		Ogl.UseProgramObjectARB(this.program);
	},
	
	Off:function() {
	
		Ogl.UseProgramObjectARB(0);
	}
}


function OglTexture2D() {

	this.texture = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, this.texture);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);

	this.LoadImage = function(image, format) {
	
		Ogl.BindTexture(Ogl.TEXTURE_2D, this.texture);
		Ogl.DefineTextureImage(Ogl.TEXTURE_2D, format, image);
	}

	this.LoadSVG = function(svgData, width, height) {

		var svg = new SVG();	
		svg.Write(svgData);
		var image = svg.RenderImage(width, height, 4, true);
		Ogl.BindTexture(Ogl.TEXTURE_2D, this.texture);
		Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, image);
		image.Free();
	}
	
	this.valueOf = function() this.texture;
}


function Light( oglLight ) {
	
	this.position = [0,0,1,1];
	this.aim = [0,0,0];
	this.cutoff = 30;
	
	var tmp = [];
	
	this.Update = function() {

		Ogl.Light(oglLight, Ogl.POSITION, this.position);
		Vec3Sub(this.aim, this.position, tmp);
		Ogl.Light(oglLight, Ogl.SPOT_DIRECTION, tmp);
	}
	
	this.SetPosition = function(x,y,z) {

		this.position[0] = x;
		this.position[1] = y;
		this.position[2] = z;
		this.Update();
	}
	
	this.SetAim = function(x,y,z) {
		
		this.aim[0] = x;
		this.aim[1] = y;
		this.aim[2] = z;
		this.Update();
	}
	
	this.EnableProjectorTextureCoordinates = function() {

		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		Ogl.Translate(0.5, 0.5, 0.5);
		Ogl.Scale(0.5);
		Ogl.Perspective(this.cutoff * 2, 1, 1, 100); // Ogl.GetLight(oglLight, Ogl.SPOT_CUTOFF);
		Ogl.LookAt( this.position[0], this.position[1], this.position[2],  this.aim[0], this.aim[1], this.aim[2],  0, 0, 1 );
		var mat = Ogl.Get(Ogl.MODELVIEW_MATRIX);
		Ogl.PopMatrix();

		//Set up texture coordinate generation.
		Ogl.TexGen(Ogl.S, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[4], mat[8], mat[12]);
		Ogl.TexGen(Ogl.T, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.T, Ogl.EYE_PLANE, mat[1], mat[5], mat[9], mat[13]);
		//Ogl.TexGen(Ogl.R, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		//Ogl.TexGen(Ogl.R, Ogl.EYE_PLANE, mat[2], mat[6], mat[10], mat[14]);
		Ogl.TexGen(Ogl.Q, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.Q, Ogl.EYE_PLANE, mat[3], mat[7], mat[11], mat[15]);

		Ogl.Enable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_Q);
		//Ogl.Enable(Ogl.TEXTURE_GEN_R);
	}

	this.DisableProjectorTextureCoordinates = function() {
	
		Ogl.Disable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_Q);
		//Ogl.Disable(Ogl.TEXTURE_GEN_R);
	}	
	
	Ogl.Light(oglLight, Ogl.DIFFUSE, 1, 1, 1, 1);
	Ogl.Light(oglLight, Ogl.AMBIENT, 0, 0, 0, 0.95);
	//Ogl.Light(oglLight, Ogl.CONSTANT_ATTENUATION, 0.1);
	//Ogl.Light(oglLight, Ogl.LINEAR_ATTENUATION, 0.003);
	//Ogl.Light(oglLight, Ogl.SPOT_EXPONENT, 0);
	Ogl.Light(oglLight, Ogl.SPOT_CUTOFF, this.cutoff);
	Ogl.Enable(oglLight);
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

	GlSetAttribute( GL_DOUBLEBUFFER, 1 );
	GlSetAttribute( GL_SWAP_CONTROL, 1 ); // vsync
	GlSetAttribute( GL_DEPTH_SIZE, 32);
	GlSetAttribute( GL_STENCIL_SIZE, 8 );
//	GlSetAttribute( GL_ACCELERATED_VISUAL, 1 );
	
	SetVideoMode(currentWidth, currentHeight, undefined, defaultVideoMode);
	
	Print( 'OpenGL v', Ogl.GetString(Ogl.VERSION), '\n' );
	
	//	Assert( Ogl.HasExtensionName('GL_EXT_stencil_two_side') );
	Assert( Ogl.HasExtensionName('GL_ARB_texture_env_combine') );

	Ogl.Hint(Ogl.PERSPECTIVE_CORRECTION_HINT, Ogl.NICEST);
	Ogl.Hint(Ogl.POINT_SMOOTH_HINT, Ogl.NICEST);
	Ogl.Hint(Ogl.POLYGON_SMOOTH_HINT, Ogl.NICEST);

	Ogl.LightModel(Ogl.LIGHT_MODEL_LOCAL_VIEWER, 1); // see. http://gregs-blog.com/2007/12/21/theres-nothing-wrong-with-opengls-specular-lighting/

	Ogl.PixelStore(Ogl.UNPACK_ALIGNMENT, 1);

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

	ShadowVolumeProgram.prototype = ShaderProgramProto;
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
		
		this.AddVertexShader(Expand(source, { lightIndex: light-Ogl.LIGHT0 }));
		this.Link();
	}


	LightConeProgram.prototype = ShaderProgramProto;
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

		this.AddVertexShader(Expand(source, { lightIndex: light-Ogl.LIGHT0 }));
		this.Link();
	}

	var lightConeProgram = new LightConeProgram(Ogl.LIGHT0);
	var shadowVolumeProgram = new ShadowVolumeProgram(Ogl.LIGHT0);
	
	var useTwoSideStencil = Ogl.HasExtensionName('GL_EXT_stencil_two_side', 'GL_EXT_stencil_wrap');
	var useSeparateStencil = Ogl.HasExtensionProc('glStencilOpSeparate');

	this.light = new Light(Ogl.LIGHT0);
	Ogl.Enable(Ogl.LIGHTING);
		
	this.RenderWithShadows = function( renderCallback ) {
	
/*
			Ogl.PolygonMode(Ogl.FRONT_AND_BACK, Ogl.LINE);
			Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
			Ogl.Color(1,1,1, 0.1)
			Ogl.Enable(Ogl.CULL_FACE);
			Ogl.CullFace(Ogl.BACK);
			Ogl.Enable(Ogl.BLEND);
			rays.On();
			renderCallback(3);
			rays.Off();
			Ogl.Disable(Ogl.BLEND);
		return;
*/		

		// see http://www.opengl.org/resources/code/samples/glut_examples/advanced/shadowvol.c	// http://www.angelfire.com/games5/duktroa/RealTimeShadowTutorial.htm
		// http://www.gamedev.net/columns/hardcore/cgshadow/page2.asp // http://joshbeam.com/articles/stenciled_shadow_volumes_in_opengl/
	
		Ogl.Enable(Ogl.POLYGON_OFFSET_FILL);
		Ogl.PolygonOffset(0, -32);
			renderCallback(6);
		Ogl.Disable(Ogl.POLYGON_OFFSET_FILL);

		Ogl.Disable(Ogl.LIGHTING);
		Ogl.DepthMask(false);

		var hasFog = Ogl.IsEnabled(Ogl.FOG);

		if ( hasFog ) {
		
			Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
			Ogl.Enable(Ogl.BLEND);
			Ogl.Enable(Ogl.CULL_FACE);
			Ogl.CullFace(Ogl.BACK);
		} else {

			Ogl.ColorMask(false);
		}
		
		Ogl.StencilFunc(Ogl.ALWAYS, 0, 0);
		Ogl.Enable(Ogl.STENCIL_TEST);
		Ogl.Clear(Ogl.STENCIL_BUFFER_BIT);

		shadowVolumeProgram.On();
		shadowVolumeProgram.Set('hasFog', hasFog);
		
		Ogl.DepthFunc(Ogl.LESS); // needed ???

		// see http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=149515
		if ( useSeparateStencil ) {

			Ogl.Disable(Ogl.CULL_FACE);
			Ogl.StencilOpSeparate(Ogl.BACK, Ogl.KEEP, Ogl.INCR_WRAP_EXT, Ogl.KEEP);
			Ogl.StencilOpSeparate(Ogl.FRONT, Ogl.KEEP, Ogl.DECR_WRAP_EXT, Ogl.KEEP);
				renderCallback(3); // render occluders shape only
			Ogl.Enable(Ogl.CULL_FACE);
		} else if ( useTwoSideStencil ) {

			Ogl.Disable(Ogl.CULL_FACE);
			Ogl.Enable(Ogl.STENCIL_TEST_TWO_SIDE_EXT);
			Ogl.ActiveStencilFaceEXT(Ogl.BACK);
			Ogl.StencilOp(Ogl.KEEP, Ogl.INCR_WRAP_EXT, Ogl.KEEP);
			Ogl.ActiveStencilFaceEXT(Ogl.FRONT);
			Ogl.StencilOp(Ogl.KEEP, Ogl.DECR_WRAP_EXT, Ogl.KEEP);
				renderCallback(3); // render occluders shape only
			Ogl.Disable(Ogl.STENCIL_TEST_TWO_SIDE_EXT);
			Ogl.Enable(Ogl.CULL_FACE);
		} else {

			Ogl.Enable(Ogl.CULL_FACE);
			Ogl.StencilOp(Ogl.KEEP, Ogl.INCR, Ogl.KEEP);
			Ogl.CullFace(Ogl.FRONT);
			var list = Ogl.NewList(false);
				renderCallback(3); // render occluders shape only
			Ogl.EndList()
			Ogl.StencilOp(Ogl.KEEP, Ogl.DECR, Ogl.KEEP);
			Ogl.CullFace(Ogl.BACK);
				Ogl.CallList(list);
			Ogl.DeleteList(list);
		}

		shadowVolumeProgram.Off();

		Ogl.DepthFunc(Ogl.ALWAYS);
		Ogl.StencilFunc(Ogl.NOTEQUAL, 0, -1);
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.KEEP);

		Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.Enable(Ogl.BLEND);
		
		Ogl.Color(Ogl.GetLight(Ogl.LIGHT0, Ogl.AMBIENT)); // use light's ambiant color as shadow color
		Ogl.ColorMask(true);

/*
		if ( hasFog )
			renderCallback(5); ???
		else
			DrawFullQuad();
*/
		DrawFullQuad();

		Ogl.Disable(Ogl.BLEND, Ogl.STENCIL_TEST);
		
		Ogl.DepthMask(true);
		Ogl.DepthFunc(Ogl.LEQUAL);
		
		if ( hasFog ) {
		
			Ogl.Disable(Ogl.TEXTURE_2D);
			
			Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE);
			Ogl.Color(1,1,1, 1);
			lightConeProgram.On();

			Ogl.Enable(Ogl.CULL_FACE);
			Ogl.CullFace(Ogl.BACK);
			
			Ogl.Enable(Ogl.BLEND);

			Ogl.Translate(this.light.position[0], this.light.position[1], this.light.position[2]);
			Ogl.AimAt(this.light.aim[0]-this.light.position[0], this.light.aim[1]-this.light.position[1], this.light.aim[2]-this.light.position[2]);
			Ogl.DepthMask(false);
			
			var h = 100;
			var r = Math.tan(Ogl.GetLight(Ogl.LIGHT0, Ogl.SPOT_CUTOFF) / 180 * Math.PI ) * h;
			Ogl.DrawCylinder(0, r, h, 64, 32);
			Ogl.DepthMask(true);

			Ogl.Disable(Ogl.BLEND);
			Ogl.Enable(Ogl.CULL_FACE);
			Ogl.CullFace(Ogl.BACK);

			lightConeProgram.Off();
			
			Ogl.Color(0,0,0,1);
			Ogl.Translate(0,0,-2);
			Ogl.DrawCylinder(0, 2, 3, 16, 1);
		}
		
		Ogl.Enable(Ogl.LIGHTING);
	}

	//////////

	this.status = 'status';
	
	this.Projection = function() {
		
		Ogl.Perspective(60, undefined, 0.5, 500);
		
//		// http://www.songho.ca/opengl/gl_projectionmatrix.html
//		DumpMatrix(Ogl.Get(Ogl.PROJECTION_MATRIX)); Halt();
	}

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
	
	var fps = 0.;
	var fpsArray = [];
	
	var isInit = false;

	function SurfaceReady() {
	
		if ( !isInit ) {

			this.Init && this.Init();
			isInit = true;
		}

		var t0 = TimeCounter();
		Ogl.Viewport(0, 0, videoWidth, videoHeight);
		Ogl.ClearColor(0.15, 0.2, 0.4, 0);
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);

		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.LoadIdentity();
		this.Projection();
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.LoadIdentity();
	
		this.Draw(this.frame);

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
		
		fps = 0.;
		var len = fpsArray.length;
		for ( var i = 0; i < len; ++i )
			fps += fpsArray[i];
		fps = (fps / len).toFixed(1);
		
		
		var str = fps+'fps\t'+this.status;
		for ( var [i,chunk] in Iterator(str.split('\t')) )
			f2d.Draw(chunk, 2 + i * 150, 2);
		Ogl.PopAttrib();

		Ogl.Finish();
		GlSwapBuffers(true);
		this.Idle();
		
		fps = 1000/(TimeCounter()-t0);
		if ( fpsArray.length > 2 )
			fpsArray.shift();
		fpsArray.push(fps);
		
		this.frame++;
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

	this.Loop = function( additionalEventCallback ) {
		
		while ( !endSignal ) {

			var events = [ EndSignalEvents(), SDLEvents(listeners), SurfaceReadyEvents.call(this, SurfaceReady) ];
			additionalEventCallback && additionalEventCallback(events);
			ProcessEvents.apply(this, events);
		}
	}
}





/* **************************************************************************************************** */
/* UNUSED CODE :


	var shadowMapSize = 128;
	var shadowMapTexture = Ogl.GenTexture();
	Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);
	Ogl.TexImage2D(Ogl.TEXTURE_2D, 0, Ogl.DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0, Ogl.DEPTH_COMPONENT, Ogl.UNSIGNED_BYTE, null);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR); // NEAREST
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_MODE, Ogl.COMPARE_R_TO_TEXTURE); //Enable shadow comparison
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_COMPARE_FUNC, Ogl.LEQUAL); //Shadow comparison should be true (ie not in shadow) if r<=texture
	Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.DEPTH_TEXTURE_MODE, Ogl.INTENSITY); //Shadow comparison should generate an INTENSITY result


	this.RenderWithShadows1 = function( renderCallback ) {
		
		// http://www.paulsprojects.net/tutorials/smt/smt.html ( and http://dalab.se.sjtu.edu.cn/~jietan/shadowMappingTutorial.html )
		// http://www.google.com/codesearch/p?hl=en#4FSOSMZ6Pxc/distfiles/MesaDemos-5.0.tar.bz2|w8yTKn7FXYw/Mesa-5.0/demos/shadowtex.c

		var mat = new Transformation(0.5, 0.0, 0.0, 0.0,  0.0, 0.5, 0.0, 0.0,  0.0, 0.0, 0.5, 0.0,  0.5, 0.5, 0.5, 1.0);

		Ogl.Enable(Ogl.NORMALIZE);

// First pass - from light's point of view

		Ogl.PushAttrib( Ogl.VIEWPORT_BIT | Ogl.LIGHTING_BIT ); // | Ogl.ENABLE_BIT
		Ogl.Viewport(0, 0, shadowMapSize, shadowMapSize);

		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		Ogl.Perspective(60, 1, 30, 50); //var lightFov = Ogl.GetLight(Ogl.LIGHT0, Ogl.SPOT_CUTOFF) * 2;
		mat.Product(Ogl);
		
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		Ogl.LookAt( lightPos[0], lightPos[1], lightPos[2],  lightDir[0], lightDir[1], lightDir[2],  0, 0, 1 );
		mat.Product(Ogl);

		Ogl.Disable(Ogl.TEXTURE_2D);
		Ogl.Disable(Ogl.LIGHTING);
		Ogl.CullFace(Ogl.FRONT);
		Ogl.ShadeModel(Ogl.FLAT);
		Ogl.ColorMask(false, false, false, false);
//		Ogl.ClearDepth(1); // default is 1
		Ogl.DepthFunc(Ogl.LEQUAL);
		Ogl.Enable(Ogl.DEPTH_TEST);
		Ogl.Clear(Ogl.DEPTH_BUFFER_BIT);

		Ogl.Enable(Ogl.POLYGON_OFFSET_FILL);
//		Ogl.PolygonOffset(1, 2); // set the scale and units used to calculate depth values.
		
//		Ogl.DepthRange(0.5, 1);

		renderCallback(3); // render occluders + shape only
//		new File('zbuffer.png').content = EncodePngImage(Ogl.ReadImage(true, Ogl.DEPTH_COMPONENT));  throw 0;		
		
		Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);

//		Ogl.PixelTransfer(Ogl.DEPTH_BIAS, 0);
//		Ogl.PixelTransfer(Ogl.DEPTH_SCALE, 1);
		
		Ogl.CopyTexSubImage2D(Ogl.TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSize, shadowMapSize);

		Ogl.ColorMask(true, true, true, true);
		Ogl.ShadeModel(Ogl.SMOOTH);
		Ogl.CullFace(Ogl.BACK);
		
		Ogl.PopMatrix();
		Ogl.MatrixMode(Ogl.PROJECTION);
		Ogl.PopMatrix();
		Ogl.MatrixMode(Ogl.MODELVIEW);
		Ogl.PopAttrib();
		
//2nd pass - Draw from camera's point of view

		//Use dim light to represent shadowed areas
		Ogl.Light(Ogl.LIGHT0, Ogl.POSITION, lightPos); // needed ?
		Ogl.Light(Ogl.LIGHT0, Ogl.AMBIENT, 1.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1.5, 0.5, 0.5, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 0, 0, 0, 1);
		
		Ogl.Enable(Ogl.LIGHTING);
		Ogl.Clear(Ogl.COLOR_BUFFER_BIT | Ogl.DEPTH_BUFFER_BIT);
		
		renderCallback(4); // render objects that receive shadow

//3rd pass - Draw with bright light

		Ogl.Light(Ogl.LIGHT0, Ogl.DIFFUSE, 1,1,1, 1);
		Ogl.Light(Ogl.LIGHT0, Ogl.SPECULAR, 1,1,1, 1);

		//Set up texture coordinate generation.
		Ogl.TexGen(Ogl.S, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.S, Ogl.EYE_PLANE, mat[0], mat[4], mat[8], mat[12]);
		Ogl.TexGen(Ogl.T, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.T, Ogl.EYE_PLANE, mat[1], mat[5], mat[9], mat[13]);
		Ogl.TexGen(Ogl.R, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.R, Ogl.EYE_PLANE, mat[2], mat[6], mat[10], mat[14]);
		Ogl.TexGen(Ogl.Q, Ogl.TEXTURE_GEN_MODE, Ogl.EYE_LINEAR);
		Ogl.TexGen(Ogl.Q, Ogl.EYE_PLANE, mat[3], mat[7], mat[11], mat[15]);

		Ogl.Enable(Ogl.TEXTURE_GEN_S, Ogl.TEXTURE_GEN_T, Ogl.TEXTURE_GEN_R, Ogl.TEXTURE_GEN_Q);

		//Bind & enable shadow map texture
		Ogl.BindTexture(Ogl.TEXTURE_2D, shadowMapTexture);  // Ogl.BindTexture(Ogl.TEXTURE_2D, testTexture);

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





	var tmpShadowMatrix = [];
	this.RenderWithShadows = function( renderCallback, plane ) {
		
		// see http://www.opengl.org/resources/code/samples/mjktips/TexShadowReflectLight.html

		Ogl.Clear(Ogl.STENCIL_BUFFER_BIT);
		
		// Draw the floor with stencil value 3.  This helps us only draw the shadow once per floor pixel (and only on the floor pixels).
		Ogl.Enable(Ogl.STENCIL_TEST);
		Ogl.StencilFunc(Ogl.ALWAYS, 3, -1);
		Ogl.StencilOp(Ogl.KEEP, Ogl.KEEP, Ogl.REPLACE);

		renderCallback(4);
		
		// Render the projected shadow.
		Ogl.StencilFunc(Ogl.LESS, 2, -1);  // draw if ==1
		Ogl.StencilOp(Ogl.REPLACE, Ogl.REPLACE, Ogl.REPLACE);
		
		// To eliminate depth buffer artifacts, we use polygon offset to raise the depth of the projected shadow slightly so that it does not depth buffer alias with the floor.
		Ogl.PolygonOffset(-2, -1); // set the scale and units used to calculate depth values.
      // Render 50% black shadow color on top of whatever the floor appareance is.
      Ogl.Enable(Ogl.POLYGON_OFFSET_FILL, Ogl.BLEND);
      Ogl.BlendFunc(Ogl.SRC_ALPHA, Ogl.ONE_MINUS_SRC_ALPHA);
		Ogl.Disable(Ogl.LIGHTING); // Force the 50% black.
      Ogl.Color(0.0, 0.0, 0.0, 0.5);
      Ogl.PushMatrix();
		Ogl.MultMatrix(ShadowMatrix(plane, lightPos, tmpShadowMatrix));

		renderCallback(3);
//		new File('tmp.png').content = EncodePngImage(Ogl.ReadImage(true, Ogl.RGB));  throw 0;

		Ogl.PopMatrix();
      Ogl.Enable(Ogl.LIGHTING);
      Ogl.Disable(Ogl.BLEND, Ogl.POLYGON_OFFSET_FILL, Ogl.STENCIL_TEST);
	}



*/


