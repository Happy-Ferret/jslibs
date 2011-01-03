LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');

var size = 512;

SetVideoMode( size, size, 32, OPENGL );

Assert( Ogl.HasExtensionName('GL_ARB_fragment_shader', 'GL_ARB_shading_language_100', 'GL_ARB_shader_objects') );

var vertexShaderSource = <><![CDATA[
	void main(void) {

		gl_Position = ftransform();
	}
]]></>.toString();

var fragmentShaderSource = <><![CDATA[
	void main(void) {

		gl_FragColor = vec4(gl_FragCoord.x / 1000, 0, 0, 1);
	}
]]></>.toString();



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
if ( !Ogl.GetObjectParameterARB(this.program, Ogl.OBJECT_LINK_STATUS_ARB) ) {

	Print( 'LinkProgramARB log:\n', Ogl.GetInfoLogARB(this.program), '\n' );
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
if ( !Ogl.GetObjectParameterARB(this.program, Ogl.OBJECT_LINK_STATUS_ARB) ) {

	Print( 'LinkProgramARB log:\n', Ogl.GetInfoLogARB(this.program), '\n' );
	throw 0;
}


Ogl.Viewport(0,0,size, size);

Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.LoadIdentity();
Ogl.Perspective(60, undefined, 0.1, 500);
Ogl.MatrixMode(Ogl.MODELVIEW);

var r = 0.0;
function SurfaceReady() {

	Ogl.LoadIdentity();
	
	Ogl.Clear(Ogl.COLOR_BUFFER_BIT);

	Ogl.UseProgramObjectARB(program);

	Ogl.Color(1,1,1);
	
	Ogl.Translate(0, 0, -5);

	Ogl.Rotate(r, 0, 1, 0);

	r += 0.1
	
	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1, 1);
	Ogl.TexCoord(1, 0); Ogl.Vertex( 1, 1);
	Ogl.TexCoord(1, 1); Ogl.Vertex( 1,-1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();
	
	GlSwapBuffers();
}

var end = false;
var listeners = {

	onKeyDown:function(sym, mod, chr) {
	
		end = true;
	},
	
}

while ( !end )
	ProcessEvents(SDLEvents(listeners), SurfaceReadyEvents(SurfaceReady));


