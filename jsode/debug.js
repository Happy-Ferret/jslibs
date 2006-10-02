LoadModule('jsstd');
LoadModule('jsode');

var world = new World;
world.gravity = [0,0,-9.81];

var body = new Body(world);
var body1 = new Body(world);

var joint = new JointBall(world);

body.toto = 123;
joint.Attach(body, body1);

Print( joint.body1.toto );

joint.Destroy();

//body.mass = new Mass(10);

Print( body.mass );

//body.linearVel = [0,0,15];
	body.force = [ 0,1,1 ];




for ( var i = 0; i<20; i++ ) {
//	Print( 'Step:', i,  '\n' );
	Print( ' Position:', body.position , '\n' );
//	Print( ' Force:', body.force , '\n' );
	world.Step(1,true);
}