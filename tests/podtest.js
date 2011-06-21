LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');

Exec('../common/tools.js');



var taskList = [];

function _ProcEnd(proc) {
	
	var atExit = proc.atExit;
	if ( atExit ) {
	
		var len = atExit.length;
		for ( var i = 0; i < len; ++i )
			taskList.push(atExit[i]);
	}
}

function StartProc(proc) {
	
	taskList.push(function inner(result) {
	
		try {
			proc.send(result)(inner);
		} catch (ex if ex === StopIteration) {
			_ProcEnd(proc);
		}
	});
	return proc;
}

function PWaitProc(proc) function(callback) {
		
	if ( !proc.atExit )
		proc.atExit = [];
	proc.atExit.push(function() taskList.push(callback));
}

function PIdle(callback) {

	 taskList.push(callback);
}

function PKill(proc) {
	
	if ( proc ) {
		
		proc.close();
		_ProcEnd(proc);
	} else {
		
		throw StopIteration;
	}
}


// timer

var currentTime = Date.now();
var timeoutList = [];

function AddTimeout(time, fct) {

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
		taskList.push(exList[i][1]);
}

function PSleep(time) function(callback) AddTimeout(time, callback);


// event

function PCreateEvent(manualReset, initialState) {

	return [[], initialState, manualReset];
}

function PResetEvent(event) {
	
	event[1] = false;
}

function PFireEvent(event) {

	var len = event[0].length;
	for ( var i = 0; i < len; ++i )
		taskList.push(event[0][i]);
	event[0].length = 0;
	event[1] = event[2];
}

function PWaitEvent(event) function(callback) {

	if ( event[1] )
		taskList.push(callback);
	else
		event[0].push(callback);
}


// Semaphore

function PCreateSemaphore(initialCount) {
	
	return [[], initialCount];
}

function PReleaseSemaphore(semaphore) {
	
	if ( semaphore[0].length )
		taskList.push(semaphore[0].shift());
	else
		++semaphore[1];
}

function PAcquireSemaphore(semaphore) function(callback) {

	if ( semaphore[1] > 0 ) {
	
		--semaphore[1];
		taskList.push(callback);
	} else {
	
		semaphore[0].push(callback);
	}
}


// Step

function Step(elapsed, maxWait) {

	currentTime += elapsed;
	ProcessTimeout();
	
	var wait;
	if ( timeoutList.length )
		wait = Math.min(timeoutList[0][0] - currentTime, maxWait);
	else
		wait = maxWait;

	var len = taskList.length;
	if ( len ) {
		
		var tmp = taskList;
		taskList = [];
		for ( var i = 0; i < len; ++i )
			tmp[i]();
	}
	
	Sleep(Math.floor(wait));
}


////////////////////////////////////



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
		
		m1.maxForce = Math.max( Math.min(ratio, 1), 0 ) * 10;
	}

	function M2Force(ratio) {

		m2.maxForce = Math.max( Math.min(ratio, 1), 0 ) * 10;
	}
	
	function MForce(m1ratio, m2ratio) {

		if ( m1ratio != undefined )
			m1.maxForce = Math.max( Math.min(m1ratio, 1), 0 ) * 10;
		
		if ( m2ratio != undefined )
			m2.maxForce = Math.max( Math.min(m2ratio, 1), 0 ) * 10;
	}
	
	
	this.Draw = function(env3d) {

	//	env3d.AddTrail(1, center.position);

		env3d.AddTrail(1, m1.body1.position);
		env3d.AddTrail(2, m2.body1.position);

		env3d.Begin();
		env3d.DrawGrid();
		
		env3d.DrawTrail(1);
		env3d.DrawTrail(2);
		
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
		Ogl.MultMatrix(center);
		Ogl.Scale(5);
		Ogl.LineWidth(1);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,1,1, 1 ); Ogl.Vertex(0,0,0); Ogl.Vertex(0,0,1);
		Ogl.End();
		Ogl.PopMatrix();
		
		Ogl.PushMatrix();
//		Ogl.MultMatrix(center);
		var pos = center.position;
		Ogl.Translate(pos[0], pos[1], pos[2]);
		Ogl.Scale(1);
		Ogl.LineWidth(1);
		Ogl.Begin(Ogl.LINES);
		Ogl.Color( 1,0.2,0.2, 0.5 ); Ogl.Vertex(0,0,0); Ogl.Vertex(/*Vec3Normalize*/(center.linearVel));
		Ogl.End();
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
		yield PIdle;
	}

	this.PushRight = function(force) {
	
		m2.maxForce = force;
		yield PIdle;
	}
	

	this.TurnUntil = function(direction, predicate) taskList.unshift(new function() {
	});
	
	function GetRotationSpeed() {
	
		//var front = center.Vector3ToWorld([0,0,1]);
		//var vel = center.linearVel;
		//return Vec3Dot( Vec3Sub(m1.body1.linearVel, vel), front );
		return center.angularVel[1];
	}

	function GetStraightRatio() {
	
		var vel = center.linearVel;
		return Vec3Dot( center.Vector3ToWorld([0,0,1]), Vec3Normalize(vel) );
	}
	
	function GetStraightRad() {
	
		var vel = center.linearVel;
		var str = Vec3Dot( center.Vector3ToWorld([0,0,1]), Vec3Normalize(vel) );
		var dir = Vec3Dot( center.Vector3ToWorld([1,0,0]), vel );
		var radError = Math.PI/2 - Math.asin(str);
		if ( dir < 0 )
			radError = -radError;
		return radError;
	}
	
	
/*
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
			yield PIdle;
		}
	};
*/

	var maxRotationAccelSpeed;

	var TestMaxRotationSpeed = function() {
	
		MForce(1,0);
		yield PSleep(10);
		var tmp = GetRotationSpeed();
		yield PSleep(100);
		maxRotationAccelSpeed = (GetRotationSpeed() - tmp)/100;
		MForce(0,0);
		
//		Print('max rotation speed: ', maxRotationAccelSpeed, '\n');
	}
	
	this.StopRotation = function() {
	
		if ( maxRotationAccelSpeed == undefined )
			yield PWaitProc(StartProc(TestMaxRotationSpeed()));

		var currentRotationSpeed = GetRotationSpeed();
//		Print('rotation speed: ', currentRotationSpeed, '\n');
		
		var time = currentRotationSpeed / maxRotationAccelSpeed;

		if ( time < 0 )
			MForce(1,0);
		else 
			MForce(0,1);
		
		yield PSleep(Math.abs(time));
		
		MForce(0,0);
	}
	
	
	this.DisplayInfo = function() {

		for (;;) {
			
			var str = GetStraightRatio();
			Print( ' StraightRatio:', str.toFixed(2) );

			var radError = GetStraightRad();
			Print( ' radError:', radError.toFixed(2) );
			
			Print( ' maxRotationAccelSpeed:', maxRotationAccelSpeed ? maxRotationAccelSpeed.toFixed(2) : '      ' );

			Print( ' angularVel:', center.angularVel[1].toFixed(2) );
			
			var vel = center.linearVel;
			Print( ' vel:', vel[0].toFixed(2), ',', vel[1].toFixed(2), ',', vel[2].toFixed(2) );
			
			Print( StringRepeat(' ', 30), '\r');
			yield PSleep(Math.abs(50));
		}
	}
	

	this.GoStraight = function() {

		if ( maxRotationAccelSpeed == undefined )
			yield PWaitProc(StartProc(TestMaxRotationSpeed()));

		yield PWaitProc(StartProc(this.StopRotation()));
		
	
		for (;;) {

			var radError = GetStraightRad();
			var time = radError / maxRotationAccelSpeed;
			
			// d = 0.5 * a * t^2
			// t = sqrt( d / 0.5 * a )
			
			time = Math.sqrt( Math.abs(radError * 2 * maxRotationAccelSpeed) );
			
			Print( '\n', time, '\n' );

			if ( time < 0 )
				MForce(1,0);
			else 
				MForce(0,1);
				
			yield PSleep(Math.abs(time));
				
			if ( time < 0 )
				MForce(0,1);
			else 
				MForce(1,0);
				
			yield PSleep(Math.abs(time));
			
			MForce(0,0);
			
			return;
	
			
			yield PIdle;

		continue;
			
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

			yield WaitProc( StartProc( pod1.ReduceRotate(1) ) );			
 
			if ( err < 0 ) {
			
				M1Force(0);
				M2Force(err);
			} else {

				M1Force(err);
				M2Force(0);
			}
*/			
			yield PIdle;
		}
	
	};
	


	this.Stop = function() {
	
		yield WaitProc( StartProc( pod1.ReduceRotate(1) ) );
	
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

			yield PIdle;
			
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

			yield PIdle;
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

// env3d.AddKeyListener(K_BACKSPACE, function(polarity) { pod1.CancelTask(); });

env3d.AddKeyListener(K_PAUSE, function(polarity) { if ( polarity ) pause = !pause });

env3d.AddKeyListener(K_SPACE, function(polarity) { 

	if ( !polarity )
		return;
	StartProc( pod1.GoStraight() )
//	StartProc( pod1.StopRotation() )

});

env3d.AddKeyListener(K_LEFT, function(polarity) { StartProc( pod1.PushLeft(polarity ? 10 : 0) ) });

env3d.AddKeyListener(K_RIGHT, function(polarity) { StartProc( pod1.PushRight(polarity ? 10 : 0) ) });

env3d.AddKeyListener(K_DOWN, function(polarity) { if ( polarity ) StartProc( pod1.Stop() ) });
env3d.AddKeyListener(K_u, function(polarity) { StartProc( pod1.UTurn() ) });


StartProc( pod1.DisplayInfo() )


while ( !endSignal ) {
	
	pause || w.Step(10);
	pod1.Draw(env3d);
	env3d.End();
	
	Step(10, 10);
	
}
