LoadModule('jsstd');
LoadModule('jsode');

var world = new World;
world.gravity = [0,0,-9.809];
var floor = new GeomPlane(world.space);
floor.body = world.env;

var ball = new GeomSphere(world.space);
ball.body = new Body(world);
ball.impact = function(geom1, geom2) { 
	
	if ( geom1.body.linearVel[2] < 0 )
		Print('impact velocity: '+-geom1.body.linearVel[2].toFixed(2)+'\n' );
}

world.defaultSurfaceParameters.bounce = 0.8;
world.defaultSurfaceParameters.bounceVel = 0;

ball.body.position = [0,0,5];

Print('Gravity is '+(-world.gravity[2])+' m/s^2\n');
Print('Placing the ball at '+ball.body.position[2]+' m high.\n');
Print('Starting the simulation (press ctrl-c to abort)...\n');

while ( !endSignal ) {

	Sleep(10);
	world.Step(10);
}
