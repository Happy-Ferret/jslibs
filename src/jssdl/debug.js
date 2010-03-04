LoadModule('jsstd');
Exec('../common/tools.js');


var ui = new UI();

ui.Draw = function() {

	Ogl.LookAt(10,10,10, 0,0,0, 0,0,1);

	ui.DrawGrid();

}

ui.Loop();
