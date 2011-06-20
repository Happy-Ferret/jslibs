LoadModule('jsstd');
LoadModule('jsode');
Exec('../common/tools.js');


var currentTime = Date.now();
var timeoutList = [];

function AddTimeout( time, fct ) {

	timeoutList.push([currentTime + time, fct]);
	timeoutList.sort(function(a,b) a[0]-b[0]);
}

function ProcessTimeout() {

	var len = timeoutList.length;
	for ( var i = 0; i < len; ++i )
		if ( timeoutList[i][0] > currentTime )
			break;
	var exList = timeoutList.splice(0, i);
	var len = exList.length;
	for ( var i = 0; i < len; ++i )
		exList[i][1]();
	if ( timeoutList.length )
		return timeoutList[0][0]-currentTime;
	else
		return Infinity;
}


function ProcWait(time) function(callback) AddTimeout(time, callback);



function ProcEnd(proc) {
	
	var atExit = proc.atExit;
	if ( atExit ) {
	
		var len = atExit.length;
		while ( len-- )
			atExit[len]();
	}
}


function StartAsyncProc( proc ) {
	
	function inner(result) {
		try {
			proc.send(result)(inner);
		} catch (ex if ex === StopIteration) { ProcEnd(proc) }
	}
	try {	
		proc.next()(inner);
	} catch (ex if ex === StopIteration) { ProcEnd(proc) }
}



var taskList = [];

function Step() {
	
	taskList.length && taskList.pop()();
}

function ScheduleProc(proc) {
	
	taskList.push(function() StartAsyncProc(proc));
	return proc;
}

function WaitNext(callback) {

	 taskList.push(callback);
}

function WaitProc(proc) function(callback) {
		
	if ( !proc.atExit )
		proc.atExit = [];
	proc.atExit.push(function() taskList.push(callback));
}

function KillProc(proc) {
	
	proc.close();
	ProcEnd(proc);
}






var w = new World();
w.gravity = [0,0,-9.809];
w.gravity = [0,0,0];

function Pod() {
	
	var _this = this;

	var m1 = new JointLMotor(w);
	m1.body1 = new Body(w);
	m1.body1.position = [-2,0,0];
	m1.SetAxis(0, 1, [0,0,1]);
	m1.maxForce = 0;
	m1.velocity = Infinity;

	var m2 = new JointLMotor(w);
	m2.body1 = new Body(w);
	m2.body1.position = [2,0,0];
	m2.SetAxis(0, 1, [0,0,1]);
	m2.maxForce = 0;
	m2.velocity = Infinity;

	var center = new Body(w);
	center.position = [0,0,0];

	var j1 = new JointFixed(w);
	j1.body1 = center;
	j1.body2 = m1.body1;
	j1.Set();

	var j2 = new JointFixed(w);
	j2.body1 = center;
	j2.body2 = m2.body1;
	j2.Set();
	
	function M1Force(ratio) {
		
		if ( ratio < 0 )
			ratio = 0;
		else if ( ratio > 1 )
			ratio = 1;
		m1.maxForce = ratio * 10;
	}

	function M2Force(ratio) {
		
		if ( ratio < 0 )
			ratio = 0;
		else if ( ratio > 1 )
			ratio = 1;
		m2.maxForce = ratio * 10;
	}
	
	function MForce(m1ratio, m2ratio) {

		M1Force(m1ratio);
		M2Force(m2ratio);
	}
	
	
	this.Draw = function(env3d) {

	//	env3d.AddTrail(1, m1.body1.position);
	//	env3d.AddTrail(2, m2.body1.position);
		env3d.AddTrail(1, center.position);

		env3d.Begin();
		env3d.DrawGrid();
		
		env3d.DrawTrail(1);
	//	env3d.DrawTrail(2);
		
		Ogl.PushMatrix();
		Ogl.MultMatrix(m1.body1);
		env3d.Draw3DArrow();
		Ogl.LineWidth(2);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,1,1, 1 ); Ogl.Vertex(0,0,-1); Ogl.Vertex(0,0,-1-m1.maxForce);
		Ogl.End();
		Ogl.LineWidth(1);

		Ogl.PopMatrix();

		Ogl.PushMatrix();
		Ogl.MultMatrix(m2.body1);
		env3d.Draw3DArrow();

		Ogl.LineWidth(2);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,1,1, 1 ); Ogl.Vertex(0,0,-1); Ogl.Vertex(0,0,-1-m2.maxForce);
		Ogl.End();
		Ogl.LineWidth(1);
		
		Ogl.PopMatrix();
/*
		Ogl.PushMatrix();
		Ogl.Translate(aim[0], aim[1], aim[2]);
		env3d.Draw3DAxis();
		Ogl.PopMatrix();
*/		
	}

	
	
	this.PushLeft = function(force) {
		
		m1.maxForce = force;
		yield WaitNext;
	}

	this.PushRight = function(force) {
	
		m2.maxForce = force;
		yield WaitNext;
	}
	

	this.TurnUntil = function(direction, predicate) taskList.unshift(new function() {
	});
	
	function GetRotationSpeed() {
	
		var front = center.Vector3ToWorld([0,0,1]);
		var vel = center.linearVel;
		return Vec3Dot( Vec3Sub(m1.body1.linearVel, vel), front );
	}

	function GetStraightRatio() {
	
		var vel = center.linearVel;
		return Vec3Dot( center.Vector3ToWorld([0,0,1]), Vec3Normalize(vel) );
	}

	this.ReduceRotate = function(rot) {
		
		for (;;) {

			var r1 = GetRotationSpeed();
						
			if ( Math.abs(r1) < rot ) {
				
				MForce(0,0);
				return;
			}
			
			if ( r1 > 0 ) {
			
				MForce(0,1);
			} else {

				MForce(1,0);
			}
			yield WaitNext;
		}
	};

	
	this.GoStraight = function() {

		for (;;) {
		
			var newRot = GetRotationSpeed();

//			Print( ' RotationAccel:', (newRot-rot).toFixed(10) );

			var str = GetStraightRatio();
			var rot = newRot;
			
			
			Print( ' StraightRatio:', str.toFixed(2) );
			Print( ' RotationSpeed:', rot.toFixed(2) );
			Print( StringRepeat(' ', 30), '\r');
			
			if ( str < 0.99 || Math.abs(rot) > 0.5 ) {
				
				if ( rot > 0 ) {
	
					MForce(0,1);
				} else {

					MForce(1,0);
				}
			} else {
				
				MForce(0,0);
			}
			
			
			
/*						
			if ( err > 0.9 && Math.abs(GetRotationSpeed()) < 0.1 ) {

				M1Force(0);
				M2Force(0);
				return;
			}

			yield WaitProc( ScheduleProc( pod1.ReduceRotate(1) ) );			
 
			if ( err < 0 ) {
			
				M1Force(0);
				M2Force(err);
			} else {

				M1Force(err);
				M2Force(0);
			}
*/			
			yield WaitNext;
		}
	
	};
	


	this.Stop = function() {
	
		yield WaitProc( ScheduleProc( pod1.ReduceRotate(1) ) );
	
		var vel = center.linearVel;

		var err = Vec3Dot( center.Vector3ToWorld([0,0,-1]), Vec3Normalize(vel) );

		for (;;) {

			var front = center.Vector3ToWorld([0,0,1]);
			var ok = -Vec3Dot( front, Vec3Normalize(vel) );
			
			var d1 = Vec3Dot( vel, center.Vector3ToWorld([-1,0,0]) );
			var d2 = Vec3Dot( vel, center.Vector3ToWorld([1,0,0]) );
			
			var r1 = Vec3Dot( Vec3Sub(m1.body1.linearVel, vel), front );
			var r2 = Vec3Dot( Vec3Sub(m2.body1.linearVel, vel), front );


			Print( ' d1:', d1.toFixed(2) );
			Print( ' d2:', d2.toFixed(2) );
			Print( ' ok:', ok.toFixed(2) );
			Print( ' r1:', r1.toFixed(2) );
			Print( ' r2:', r2.toFixed(2) );
			
			var f1 = 0, f2 = 0;
			
			if ( ok < 0 && r1 < 1 && r2 < 1 ) {

				if ( d1 > 0 )
					f1 = 1;
				if ( d2 > 0 )
					f2 = 1;
			}

			if ( ok > 0 && r1 < 1 && r2 < 1 ) {
			
			}

			var speed = Vec3Length(vel);
			Print( ' speed:', speed.toFixed(2) );
			
			if ( speed < 0.1 && speed > -0.1 ) {

				M1Force(0);
				M2Force(0);
				break;
			}

			Print( ' f1:', f1.toFixed(2) );
			Print( ' f2:', f2.toFixed(2) );
			
			M1Force(f1);
			M2Force(f2);

			yield WaitNext;
			
			Print( StringRepeat(' ', 30), '\r');
		}
		
		Print( StringRepeat(' ', 50), '\r');
		M1Force(0);
		M2Force(0);
	};




	this.UTurn = function() taskList.unshift(new function() {
		
		var aimdir = center.Vector3ToWorld([0,0,-1]);
		
		for (;;) {

			var curdir = center.Vector3ToWorld([0,0,1]);
			var ok = Vec3Dot( aimdir, curdir );
			
			var d1 = Vec3Dot( aimdir, center.Vector3ToWorld([-1,0,0]) );
			var d2 = Vec3Dot( aimdir, center.Vector3ToWorld([1,0,0]) );
			
			var m1ok = Vec3Dot( m1.body1.linearVel, curdir );
			var m2ok = Vec3Dot( m2.body1.linearVel, curdir );

//			Print( m1vel.toFixed(2), '  ', m2vel.toFixed(2), ' ok:', ok.toFixed(2), '            \r');
			
			var f1 = 0, f2 = 0;
			
			if ( d1 < 0 )
				f1 = 1 - ok;
			if ( d2 < 0 )
				f2 = 1 - ok;
			
			if ( ok > 0 ) {

				// stabilization

				if ( m1ok < 0 ) {

					f2 = 0;
					f1 = -m1ok;
				}
				if ( m2ok < 0 ) {
					
					f1 = 0;
					f2 = -m2ok;
				}
					
				if ( m1ok > 0 && m2ok > 0 ) {
				
					if ( m1ok > m2ok )
						f2 += (m1ok - m2ok);
					else
						f1 += (m2ok - m1ok);
				}

			}

			M1Force(f1);
			M2Force(f2);

			yield WaitNext;
		}
	});
	
	
	this.Aim = function(aim) taskList.unshift(new function() {
		
		for (;;) {

			var len1 = Vec3Length(Vec3Sub(m1.body1.position, aim));
			var len2 = Vec3Length(Vec3Sub(m2.body1.position, aim));
			
			var dist = Vec3Length(Vec3Sub(center.position, aim));
			var dif = (len2 - len1) / 4;
			var speed = Vec3Length(center.linearVel);
			var relSpeed = center.GetRelativeVelocity(aim, []);
			var veldir = -Vec3Dot(Vec3Normalize(center.linearVel), Vec3Normalize(Vec3Sub(center.position, aim)));
			var dir = -Vec3Dot(center.Vector3ToWorld([0,0,1]), Vec3Normalize(Vec3Sub(center.position, aim)));
			var yvel = center.angularVel[1];
			var absyvel = Math.abs(yvel);
			var attn = 1-3/(dist+3); // curves: http://fooplot.com/
			var hdir = Vec3Dot(center.linearVel, center.Vector3ToWorld([1,0,0]));

			var f = 0;
			var p = 0;

			// orient to aim
			if ( dir < 0.95 && absyvel < 1 )
				f += (dif < 0 ? -1 : 1) * 2;
			
		//	Print( 'v:', relSpeed.toFixed(1), '\n' );

			// approach
			if ( dir > 0.9 && absyvel < 0.1 && relSpeed < 2 ) {

				p += 1;
			}
			
			// stabilization
			if ( dir > 0.5 || absyvel > 1 )
				f += (yvel < 0 ? -1 : 1) * absyvel*3;

			// anti-spiral
		//	if ( veldir > -0.2 && veldir < 0.2 && dir > 0.7 )
		//		f += (hdir < 0 ? -1 : 1) * 3;

			if ( relSpeed < 1 && speed > 3 ) {
			
				f += (hdir < 0 ? -1 : 1) * 2;	
			}
			
			m1.maxForce = p + (f < 0 ? -f : 0);
			m2.maxForce = p + (f > 0 ? f : 0);
			
			yield;
		}
	});


	
}

var env3d = new Env3D();

var pod1 = new Pod();

//pod1.Aim([10,0,20]);
//pod1.Stay(2);	// radius

var pause = false;

env3d.AddKeyListener(K_BACKSPACE, function(polarity) { pod1.CancelTask(); });

env3d.AddKeyListener(K_PAUSE, function(polarity) { if ( polarity ) pause = !pause });

env3d.AddKeyListener(K_SPACE, function(polarity) { 

	ScheduleProc( pod1.GoStraight() )

});

env3d.AddKeyListener(K_LEFT, function(polarity) { ScheduleProc( pod1.PushLeft(polarity ? 10 : 0) ) });

env3d.AddKeyListener(K_RIGHT, function(polarity) { ScheduleProc( pod1.PushRight(polarity ? 10 : 0) ) });

env3d.AddKeyListener(K_DOWN, function(polarity) { if ( polarity ) ScheduleProc( pod1.Stop() ) });
env3d.AddKeyListener(K_u, function(polarity) { ScheduleProc( pod1.UTurn() ) });


while ( !endSignal ) {
	
	currentTime = Date.now();
	Step();
	pause || w.Step(10);
	pod1.Draw(env3d);
	env3d.End();
	
	var n = ProcessTimeout();
	if ( n > 10 )
		n = 10;
	Sleep(n);
}
