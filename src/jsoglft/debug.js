LoadModule('jsstd');
LoadModule('jsdebug');
LoadModule('jsstd');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsfont');
LoadModule('jsoglft');

Exec('../common/tools.js');

var f3d = new Font3D(new Font('c:\\windows\\fonts\\arial.ttf'), Font3D.FILLED, 9);
//f3d.SetColor([1,0,0]);

function TextEdit(text) {

	text = text || '';
	var pos = text.length;
	
	this.onKeyDown = function(sym, mod, chr) {
		
		if ( chr >= ' ' )
			text = text.substr(0, pos) + chr + text.substr(pos++);
		else if ( sym == K_RETURN || sym == K_KP_ENTER )
			text = text.substr(0, pos) + '\n' + text.substr(pos++);
		else if ( sym == K_LEFT && pos > 0 )
			pos--;
		else if ( sym == K_RIGHT && pos < text.length )
			pos++;
		else if ( sym == K_END ) {
			var next = text.indexOf('\n', pos);
			if ( next == -1 )
				pos = text.length;
			else
				pos = next;
		} else if ( sym == K_HOME ) {
			var prev = text.lastIndexOf('\n', pos-1);
			if ( prev == -1 )
				pos = 0;
			else
				pos = prev+1;
		} else if ( sym == K_BACKSPACE && pos > 0 )
			text = text.substr(0, --pos) + text.substr(pos+1);
		else if ( sym == K_DELETE && pos <= text.length )
			text = text.substr(0, pos) + text.substr(pos+1);
		else if ( sym == K_UP ) {
	
			var prev = text.lastIndexOf('\n', pos-1);
			if ( prev == -1 )
				return;
			var offset = pos - prev;
			var min = text.lastIndexOf('\n', prev-1);
			if ( offset > prev - min )
				pos = prev;
			else
				if ( min < 0 )
					pos = offset-1;
				else
					pos = min + offset;
					
		} else if ( sym == K_DOWN ) { // abc\ndefgh\nijk

			var next = text.indexOf('\n', pos) + 1;
			if ( next == 0 )
				return;
			var p = text.lastIndexOf('\n', pos-1) + 1;
			var offset = pos - p;
			var max = text.indexOf('\n', next);
			if ( max == -1 )
				max = text.length;
			if ( next + offset > max )
				pos = max;
			else
				pos = next + offset;
		}
	}
	
	this.Draw = function(frame) {

		Ogl.LineWidth(2);
		
		var lines = text.split('\n');
		var y = 0, x = 0;
		
		var len = text.length;
		for ( var i = 0; i < len; i++ ) {
		
			Ogl.PushMatrix();
			var rnd = Math.random();
			Ogl.Translate(x, y);

			Ogl.Translate(Math.sin((frame/10+i)/20)*20, 0);
			Ogl.Rotate(Math.sin((frame+i)/10)*10,0,0,1);
			Ogl.Scale(1+Math.sin((frame+i)/20)/5);
		
			if ( i == pos ) {

				Ogl.Begin(Ogl.LINES);
				Ogl.Color(1,1,1);
				Ogl.Vertex(x,y);
				Ogl.Vertex(x,y+f3d.height);
				Ogl.End();
			}
			
			var chr = text[i];
			if ( chr == '\n' ) {

				y -= f3d.height;
				x = 0;
			} else if ( chr == ' ' ) {
			
				x += f3d.height / 3;
			} else {
			
				f3d.Draw(chr);
				x += f3d.Width(chr);
			}

			Ogl.PopMatrix();
		}
	}
}


var ui = new UI();

var te = new TextEdit('abc\ndefgh\nijk');
ui.AddEventListener(te);


ui.Draw = function(frame) {

	Ogl.Enable(Ogl.CULL_FACE);
	Ogl.CullFace(Ogl.FRONT);

	Ogl.Translate(-5, 5, -15);
	Ogl.Scale(0.1, 0.1, 1);
	te.Draw(frame);
}

ui.Loop();
