LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');


function Program( vertexShaderSource, fragmentShaderSource ) {

	Assert( Ogl.HasExtensionName('fragment_shader', 'shading_language_100', 'shader_objects') );

	var program = Ogl.CreateProgramObject();
	var shader = Ogl.CreateShaderObject(Ogl.VERTEX_SHADER);
	Ogl.ShaderSource(shader, vertexShaderSource);
	Ogl.CompileShader(shader);
	if ( !Ogl.GetObjectParameter(shader, Ogl.OBJECT_COMPILE_STATUS) ) {

		Print( 'Compile Vertex Shader log:\n', Ogl.GetInfoLog(shader), '\n' );
		throw 0;
	}
	Ogl.AttachObject(program, shader);
	Ogl.DeleteObject(shader);
	Ogl.LinkProgram(program);
	if ( !Ogl.GetObjectParameter(program, Ogl.OBJECT_LINK_STATUS) ) {

		Print( 'LinkProgram log:\n', Ogl.GetInfoLog(program), '\n' );
		throw 0;
	}
	var shader = Ogl.CreateShaderObject(Ogl.FRAGMENT_SHADER);
	Ogl.ShaderSource(shader, fragmentShaderSource);
	Ogl.CompileShader(shader);
	if ( !Ogl.GetObjectParameter(shader, Ogl.OBJECT_COMPILE_STATUS) ) {

		Print( 'Compile Fragment Shader log:\n', Ogl.GetInfoLog(shader), '\n' );
		throw 0;
	}
	Ogl.AttachObject(program, shader);
	Ogl.DeleteObject(shader);
	Ogl.LinkProgram(program);
	if ( !Ogl.GetObjectParameter(program, Ogl.OBJECT_LINK_STATUS) ) {

		Print( 'LinkProgram log:\n', Ogl.GetInfoLog(program), '\n' );
		throw 0;
	}
	
	this.uniform = Proxy.create({
		set: function(rcvf, name, val) {
		
			Ogl.UseProgramObject(program);
			Ogl.Uniform(Ogl.GetUniformLocation(program, name), val);
//			Ogl.Uniform(uniformInfo[name].location, val);
			return true;
		}
	});
	
	this.valueOf = function() {

		return program;
	}
}



var size = 64;

SetVideoMode(size, size, 32, OPENGL);
Ogl.Viewport(0,0,size, size);
Ogl.MatrixMode(Ogl.PROJECTION);
Ogl.LoadIdentity();
Ogl.Perspective(60, undefined, 0.1, 500);
Ogl.MatrixMode(Ogl.MODELVIEW);

//Print( Ogl.HasExtensionProc('glGetActiveUniformARB'), '\n' ); Halt();


// doc. http://www.opengl.org/registry/doc/GLSLangSpec.Full.1.20.8.pdf
// nehe. http://nehe.gamedev.net/data/articles/article.asp?article=21


var vertexShaderSource = ''+<><![CDATA[

	varying vec3 position;
	void main(void) {

		position = vec3(gl_MultiTexCoord0 - 0.5) * 5.0;
		gl_Position = ftransform();
	}
]]></>;


var fragmentShaderSource = Expand(<><![CDATA[

	// see. http://www.opengl.org/sdk/docs/tutorials/TyphoonLabs/Chapter_4.pdf
	varying vec3 position; 
	uniform int maxIterations; 
	uniform vec2 center; 
	uniform vec3 outerColor1; 
	uniform vec3 outerColor2; 
	uniform float zoom; 
	void main() {

		float real  = position.x * (1.0/zoom) + center.x;
		float imag  = position.y * (1.0/zoom) + center.y;
		float cReal = real;    
		float cImag = imag;    
		float r2 = 0.0;
		int iter;
		for (iter = 0; iter < maxIterations && r2 < 4.0; ++iter) {

			float tempreal = real;        
			real = (tempreal * tempreal) - (imag * imag) + cReal; 
			imag = 2.0 * tempreal * imag + cImag; 
		}
	  // Base the color on the number of iterations. 
		vec3 color; 
		if (r2 < 4.0) 
			color = vec3(0.0); 
		else     
			color = mix(outerColor1, outerColor2, fract(float(iter)*0.05)); 
		gl_FragColor = vec4 (clamp(color, 0.0, 1.0), 1.0); 
	}
]]></>, {size:size});

var program = new Program( vertexShaderSource, fragmentShaderSource );

//	var uniformInfo = Ogl.GetUniformInfo(program);
//	Print(uneval(uniformInfo)); throw 0;

//Ogl.UniformVector(Ogl.GetUniformLocation(program, 'outerColor1'), [1,0,1]);

program.uniform.outerColor1 = [1,0,1];
program.uniform.outerColor1 = [0,1,0];

program.uniform.zoom = 1;
program.uniform.maxIterations = 2;
program.uniform.center = [0,0];


var texture = Ogl.GenTexture();
Ogl.BindTexture(Ogl.TEXTURE_2D, texture);

Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MIN_FILTER, Ogl.LINEAR);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_MAG_FILTER, Ogl.LINEAR);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_S, Ogl.CLAMP);
Ogl.TexParameter(Ogl.TEXTURE_2D, Ogl.TEXTURE_WRAP_T, Ogl.CLAMP);

var randomTextureSize = 32;
var arr = new Float32Array(randomTextureSize*randomTextureSize);
for ( var i = 0; i < randomTextureSize*randomTextureSize; ++i )
	arr[i] = Math.random();
Ogl.DefineTextureImage(Ogl.TEXTURE_2D, undefined, arr, randomTextureSize, randomTextureSize, 1);
Ogl.BindTexture(Ogl.TEXTURE_2D, 0);



Ogl.UseProgramObject(program);

var t = 0;
Ogl.ActiveTexture(Ogl.TEXTURE0 + t); // Bind the texture to texture unit t.
Ogl.BindTexture(Ogl.TEXTURE_2D, texture);
Ogl.UniformInteger(Ogl.GetUniformLocation(program, 'randomTexture'), t);
Ogl.UseProgramObject(0);

//Ogl.Enable(Ogl.TEXTURE_2D);




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
	
	Ogl.UseProgramObject(program);
	
	Ogl.Color(0,0,1);

	Ogl.Begin(Ogl.QUADS);
	Ogl.TexCoord(0, 0); Ogl.Vertex(-1, 1);
	Ogl.TexCoord(1, 0); Ogl.Vertex( 1, 1);
	Ogl.TexCoord(1, 1); Ogl.Vertex( 1,-1);
	Ogl.TexCoord(0, 1); Ogl.Vertex(-1,-1);
	Ogl.End();

	
//	Ogl.DrawSphere(1,20,10,true);
	
	Ogl.UseProgramObject(0);
	
	GlSwapBuffers();
}

var end = false;
var listeners = {

	onKeyDown:function(sym, mod, chr) {
	
		end = true;
	}
}

//while ( !end )
	ProcessEvents(SDLEvents(listeners), SurfaceReadyEvents(SurfaceReady));
