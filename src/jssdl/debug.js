LoadModule('jsstd');
Exec('../common/tools.js');

var ui = new UI();


ui.key.return = function(down) {

	Print('enter' )
	delete ui.key.return;
};

var world = new World();
world.quickStepNumIterations = 20;
world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
world.defaultSurfaceParameters.softERP = 1;
world.defaultSurfaceParameters.softCFM = 0.000001;
world.defaultSurfaceParameters.bounce = 0.5;
world.defaultSurfaceParameters.bounceVel = 2;

function Floor() {
	
	var self = this;
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.Draw = function() {

		with (Ogl) {
			
			Disable(TEXTURE_2D);
			Normal(0,0,1);
			Translate(-10,-10,0);
			Scale(2,2,0);
			var cx=10, cy=10;
			Begin(QUADS);
			for ( var x = 0; x < cx; x++ )
				for ( var y = 0; y < cy; y++ ) {
					if ( (x + y) % 2 )
						Color(0,0,0.5);
					else
						Color(0.9,0.8,0.7);
					Vertex(x,y);
					Vertex(x+1,y);
					Vertex(x+1,y+1);
					Vertex(x, y+1);
				}
			End();
		}
	}	
}


function Ball() {
	
	var geom = new GeomSphere(world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = [3,0,5];

	this.Draw = function() {

		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		Ogl.Color(1,0,0);
		Ogl.DrawSphere(1, 10, 10);
		Ogl.PopMatrix();		
	}
}

function Box() {
	
	var geom = new GeomBox(world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = [0,0,5];

	this.Draw = function() {

		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		Ogl.Color(0,1,0);
		Ogl.DrawCylinder(0.5, 0.5, 4, 1);
		Ogl.Translate(0,0,1);
		Ogl.DrawDisk(0.5, 4, 1);
		Ogl.PopMatrix();		
	}
}


var ball = new Ball();
var box = new Box();
var floor = new Floor();


ui.Draw = function(frame) {

	
	Ogl.LookAt(10,0,20, 0,0,0, 0,0,1);

	ui.DrawGrid();
	
//	floor.Draw();
	ball.Draw();
	box.Draw();
	world.Step(10);


}

ui.Loop();
