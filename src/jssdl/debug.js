LoadModule('jsstd');
Exec('../common/tools.js');

var ui = new UI();

ui.key.return = function(down) {

	Print('enter' )
	delete ui.key.return;
};

var world = new World();
world.quickStepNumIterations = 30;
world.gravity = [0,0,-9.809];
world.linearDamping = 0.001;
world.angularDamping = 0.001;
world.defaultSurfaceParameters.softERP = 0.19;
world.defaultSurfaceParameters.softCFM = 0.01;
world.defaultSurfaceParameters.bounce = 0.75;
world.defaultSurfaceParameters.bounceVel = 2;

function Floor() {
	
	var self = this;
	var geom = new GeomPlane(world.space);
	geom.params = [0,0,1,0];
	
	this.Draw = function() {

		Ogl.PushMatrix();
		with (Ogl) {
			
			Disable(TEXTURE_2D);
			Normal(0,0,1);
			Translate(-10,-10,0);
			Scale(2,2,1);
			var cx=10, cy=10;
			Begin(QUADS);
			for ( var x = 0; x < cx; x++ )
				for ( var y = 0; y < cy; y++ ) {
					if ( (x + y) % 2 )
						Color(0,0,0.5);
					else
						Color(0.9,0.8,0.7);
					Vertex(x,y);  Vertex(x+1,y);  Vertex(x+1,y+1);  Vertex(x, y+1);
				}
			End();
		}
		Ogl.PopMatrix();				
	}	
}


function Ball(pos) {
	
	var geom = new GeomSphere(world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 100;
	body.position = pos;
	
	this.body = body;
	
	var clist;
	this.Draw = function() {
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		if ( clist ) {
			Ogl.CallList(clist);
		} else {
			clist = Ogl.NewList();
			Ogl.Color(1,0,0);
			Ogl.DrawSphere(1, 15, 15);
			Ogl.EndList();
		}
		Ogl.PopMatrix();
	}
}

function Box(pos) {
	
	var geom = new GeomBox(world.space);
	var body = new Body(world);
	geom.body = body;
	body.mass.value = 1;
	body.position = pos;
	geom.lengths = [1,1,1];

	this.body = body;

	var clist;
	this.Draw = function() {

		Ogl.PushMatrix();
		Ogl.MultMatrix(geom);
		if ( clist ) {
			Ogl.CallList(clist);
		} else {
			clist = Ogl.NewList();
			Ogl.Color(0,1,0);
			Ogl.DrawBox(1,1,1);
			Ogl.EndList();
		}
		Ogl.PopMatrix();		
	}
}

var scene = [];

var ball = new Ball([0, 0, 10]);
scene.push( ball );

for ( var i = -10; i < 10; ++i )
	for ( var j = -10; j < 10; ++j )
		scene.push( new Box([i*1.0, j*1.0, 0.5]) );
		
scene.push( new Floor() );


ui.Draw = function(frame) {

	Ogl.LookAt(-15,15,15, 0,0,0, 0,0,1);

	ui.SetLight([10,10,10]);
//	ui.SetLight([Math.cos(frame/10)*10,Math.sin(frame/10)*10,0]);
//	ui.SetLight(ball.body.position);

	for each ( var obj in scene )
		obj.Draw();

	world.Step(20);
}

ui.Loop();
