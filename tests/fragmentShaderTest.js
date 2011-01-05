LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');

var size = 128;

SetVideoMode( size, size, 32, OPENGL );

Assert( Ogl.HasExtensionName('GL_ARB_fragment_shader', 'GL_ARB_shading_language_100', 'GL_ARB_shader_objects') );

// doc. http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf
// nehe. http://nehe.gamedev.net/data/articles/article.asp?article=21


var vertexShaderSource = <><![CDATA[

	varying vec2 texture_coordinate;	
	void main(void) {

		gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
		texture_coordinate = gl_MultiTexCoord0.xy;
	}
]]></>.toString();


var fragmentShaderSource = Expand(<><![CDATA[

	uniform sampler2D randomTexture;
	varying vec2 texture_coordinate;
	
	void main(void) {

//		gl_FragColor = texture2D(randomTexture, texture_coordinate);
		
		float x = texture_coordinate.x - 0.5;
		float y = texture_coordinate.y - 0.5;
		float dist = sqrt(x*x+y*y);
		
		float col = texture2D(randomTexture, texture_coordinate).x;
		
		if ( dist > col/10. + 0.4 )
			discard;
		gl_FragColor = vec4(0);

	}
]]></>, {size:size});


var program = Ogl.CreateProgramObjectARB();

var shader = Ogl.CreateShaderObjectARB(Ogl.VERTEX_SHADER_ARB);
Ogl.ShaderSourceARB(shader, vertexShaderSource);
Ogl.CompileShaderARB(shader);
if ( !Ogl.GetObjectParameterARB(shader, Ogl.OBJECT_COMPILE_STATUS_ARB) ) {

	Print( 'CompileShaderARB log:\n', Ogl.GetInfoLogARB(shader), '\n' );
	throw 0;
}
Ogl.AttachObjectARB(program, shader);
Ogl.DeleteObjectARB(shader);
Ogl.LinkProgramARB(program);
if ( !Ogl.GetObjectParameterARB(program, Ogl.OBJECT_LINK_STATUS_ARB) ) {

	Print( 'LinkProgramARB log:\n', Ogl.GetInfoLogARB(program), '\n' );
	throw 0;
}

var shader = Ogl.CreateShaderObjectARB(Ogl.FRAGMENT_SHADER_ARB);
Ogl.ShaderSourceARB(shader, fragmentShaderSource);
Ogl.CompileShaderARB(shader);
if ( !Ogl.GetObjectParameterARB(shader, Ogl.OBJECT_COMPILE_STATUS_ARB) ) {

	Print( 'CompileShaderARB log:\n', Ogl.GetInfoLogARB(shader), '\n' );
	throw 0;
}
Ogl.AttachObjectARB(program, shader);
Ogl.DeleteObjectARB(shader);
Ogl.LinkProgramARB(program);
if ( !Ogl.GetObjectParameterARB(program, Ogl.OBJECT_LINK_STATUS_ARB) ) {

	Print( 'LinkProgramARB log:\n', Ogl.GetInfoLogARB(program), '\n' );
	throw 0;
}


var texture = Ogl.GenTexture();
Ogl.BindTexture(Ogl.TEXTURE_2D, texture);

Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);

var randomTextureSize = 128;
var arr = new Float32Array(randomTextureSize*randomTextureSize);
for ( var i = 0; i < randomTextureSize*randomTextureSize; ++i )
	arr[i] = Math.random();
Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, arr, randomTextureSize, randomTextureSize, 1);
Ogl.BindTexture(Ogl.TEXTURE_2D, 0);


Ogl.UseProgramObjectARB(program);

var t = 0;
Ogl.ActiveTexture(Ogl.TEXTURE0 + t); // Bind the texture to texture unit t.
Ogl.BindTexture(Ogl.TEXTURE_2D, texture);
Ogl.UniformIntegerARB(Ogl.GetUniformLocationARB(program, 'randomTexture'), t);
Ogl.UseProgramObjectARB(0);

//Ogl.Enable(Ogl.TEXTURE_2D);


Ogl.Viewport(0,0,size, size);

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.LoadIdentity();
Ogl.Perspective(60, undefined, 0.1, 500);
Ogl.MatrixMode(Ogl.MODELVIEW);


var r = 0.0;
function SurfaceReady() {

	Ogl.LoadIdentity();
	
	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);

	Ogl.PushMatrix();
	Ogl.Translate(0, 0, -10);
	Ogl.Scale(10, 10, 1);
	Ogl.Begin(Ogl.QUADS);
	Ogl.Color(1,0,0);	Ogl.TexCoord(0, 0); Ogl.Vertex(-1, 1);
	Ogl.Color(0,1,0);	Ogl.TexCoord(1, 0); Ogl.Vertex( 1, 1);
	Ogl.Color(0,0,1);	Ogl.TexCoord(1, 1); Ogl.Vertex( 1,-1);
	Ogl.Color(1,1,0);	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();
	Ogl.PopMatrix();

	Ogl.Translate(0, 0, -2);
	Ogl.Rotate(r, 0, 1, 0);

	r += 0.03;
	
	Ogl.UseProgramObjectARB(program);
	
	Ogl.Color(0,0,1);
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1, 1);
	Ogl.TexCoord(1, 0); Ogl.Vertex( 1, 1);
	Ogl.TexCoord(1, 1); Ogl.Vertex( 1,-1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();

	Ogl.UseProgramObjectARB(0);
	
	GlSwapBuffers();
}

var end = false;
var listeners = {

	onKeyDown:function(sym, mod, chr) {
	
		end = true;
	}
}

while ( !end )
	ProcessEvents(SDLEvents(listeners), SurfaceReadyEvents(SurfaceReady));
