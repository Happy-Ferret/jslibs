LoadModule('jsstd');
LoadModule('jsio');
LoadModule('jsimage');
LoadModule('jsode');
LoadModule('jsprotex');
LoadModule('jstrimesh');
LoadModule('jssdl');
LoadModule('jsgraphics');
LoadModule('jsfont');
LoadModule('jsoglft');

Exec('../common/tools.js');


var f3d = new Font3D(new Font('c:\\windows\\fonts\\consola.ttf'), Font3D.FILLED, 9);

function DrawText(text, infrontFct) {

	Ogl.PushAttrib(Ogl.ENABLE_BIT | Ogl.POLYGON_BIT);
	if ( infrontFct ) {

		Ogl.Disable(Ogl.DEPTH_TEST, Ogl.LIGHTING);
		Ogl.PushMatrix();
		Ogl.LoadIdentity();
		infrontFct();
	}
	Ogl.Disable(Ogl.CULL_FACE);
	f3d.SetColor();
	f3d.Draw(text, -f3d.Width(text)/2, f3d.height);
	if ( infrontFct )
		Ogl.PopMatrix();
	Ogl.PopAttrib();
}




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


//w.linearDamping = 0.001;
//w.angularDamping = 0.001;

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
	center.mass.value = 1;

	m1.body1.mass.value = 1;
	m2.body1.mass.value = 1;

	

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
	
	var info = '';
	
	
	this.Draw = function(env3d) {

	//	env3d.AddTrail(1, center.position);

		env3d.AddTrail(1, m1.body1.position);
		env3d.AddTrail(2, m2.body1.position);

		env3d.Begin();

		var pos = center.position;
		Ogl.Translate(-pos[0], -pos[1], -pos[2]);

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


		DrawText(info, function() {
			Ogl.Translate(-0.5, 0.5, -1);
			Ogl.Scale(0.0025);
			Ogl.Color(1,1,1);
		});
	}

		
	this.__defineGetter__('m1', function() { return m1.maxForce / 10 });
	this.__defineSetter__('m1', function(ratio) { m1.maxForce = Math.max( Math.min(ratio, 1), 0 ) * 10 });

	this.__defineGetter__('m2', function() { return m2.maxForce / 10 });
	this.__defineSetter__('m2', function(ratio) { m2.maxForce = Math.max( Math.min(ratio, 1), 0 ) * 10 });
	

	this.__defineGetter__('angularVel', function() {
		
		return center.angularVel[1]
	});
	
	this.__defineGetter__('straightAngle', function() {

		var vel = center.linearVel;
		var str = Vec3Dot( center.Vector3ToWorld([0,0,1]), Vec3Normalize(vel) );
		var dir = Vec3Dot( center.Vector3ToWorld([-1,0,0]), vel );
		var radError = Math.PI/2 - Math.asin(str);
		if ( dir < 0 )
			radError = -radError;
		return radError;
	});

	this.__defineGetter__('angle', function() {

		var str = Vec3Dot( center.Vector3ToWorld([0,0,1]), [0,0,1] );
		var dir = Vec3Dot( center.Vector3ToWorld([-1,0,0]), [0,0,1] );
		var radError = Math.PI/2 - Math.asin(str);
		if ( dir < 0 )
			radError = -radError;
		return radError;
	});

	
	this.maxAngularAccel = 0;
	this.ProbeMaxAngularAccel = function() {
	
		MForce(1,0);
		var tmp = this.angularVel;
		yield PSleep(500);
		this.maxAngularAccel = (this.angularVel - tmp)/500 * 1000;
		MForce(0,0);
	}

	this.DisplayInfo = function() {
	
		function PrintValue(name, num) {
			
			var s = num.toFixed(3);
			Print( name, ': ', StringRepeat(' ', 6-s.length)+s, '  ' );
		}

		var vel, prevVel;
		for (;;) {

			PrintValue( 'angle', this.angle );
			PrintValue( 'straightAngle', this.straightAngle );
			PrintValue( 'maxAngularAccel', this.maxAngularAccel );
			vel = this.angularVel;
			PrintValue( 'angularVel', vel );
			PrintValue( 'accel', (prevVel - vel) );
			prevVel = vel;
			PrintValue( 'linearVel', Vec3Length(center.linearVel) );
			Print( StringRepeat(' ', 20), '\r');
			yield PSleep(Math.abs(50));
		}
	}

	this.Init = function() {

		StartProc(this.DisplayInfo());
		StartProc(this.ProbeMaxAngularAccel());
		yield PIdle;
	}
	
	
	this.PushLeft = function(force) {
		
		this.m1 = force;
		return;
		
		if ( force == 0 )
			StartProc(this.GoStraight());
		else
			this.m1 += force;
		yield PIdle;
	}

	this.PushRight = function(force) {

		this.m2 = force;
		return;

		if ( force == 0 )
			StartProc(this.GoStraight());
		else
			this.m2 += force;
		yield PIdle;
	}
	
	this.Thrust = function(rad, pow) {
		
		var dir = Math.sin(rad) * pow;
		this.m1 = dir;
		this.m2 = -dir;
	}
			
	this.StopRotation = function() {
	
		var time = this.angularVel / this.maxAngularAccel;
		
		this.Thrust( Math.PI/2 * (time < 0 ? -1 : 1) );
//		if ( time < 0 )
//			MForce(1,0);
//		else 
//			MForce(0,1);
		yield PSleep(Math.abs(time)*1000);
		MForce(0,0);
	}


	this.GoStraight = function() {
	
		var a = this.maxAngularAccel;
		var d = this.straightAngle;
		var v = this.angularVel;

		var time = v / a;
		this.Thrust( Math.PI/2 * (time < 0 ? 1 : -1), 1 );
		yield PSleep(Math.abs(time)*1000);

		var d = this.straightAngle;
		var time = Math.sqrt(a*Math.abs(d))/a;

		this.Thrust( Math.PI/2 * (d < 0 ? 1 : -1), 1 );
		yield PSleep(time * 1000);

		this.Thrust( Math.PI/2 * (d < 0 ? -1 : 1), 1 );
		yield PSleep(time * 1000);
		
		MForce(0,0);
	}
	
	function ClampAngle(rad) {
		
		return (rad + Math.PI) % (2*Math.PI) - Math.PI;
	}

	this.GoBack = function() {
	
		var a = this.maxAngularAccel;
		var v = this.angularVel;

		var time = v / a;
		this.Thrust( Math.PI/2 * (time < 0 ? 1 : -1), 1 );
		yield PSleep(Math.abs(time)*1000);

		var d = this.straightAngle;
		var d = ClampAngle(d + Math.PI);
		
		var time = Math.sqrt(a*Math.abs(d))/a;

		this.Thrust( Math.PI/2 * (d < 0 ? 1 : -1), 1 );
		yield PSleep(time * 1000);

		this.Thrust( Math.PI/2 * (d < 0 ? -1 : 1), 1 );
		yield PSleep(time * 1000);
		
		MForce(0,0);
	};
	

/*
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
*/



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

env3d.AddKeyListener(K_UP, function(polarity) { 

	if ( !polarity )
		return;
	StartProc( pod1.GoStraight() )
});

env3d.AddKeyListener(K_DOWN, function(polarity) { 

	if ( !polarity )
		return;
	StartProc( pod1.GoBack() )
});


//env3d.AddKeyListener(K_HOME, function(polarity) { StartProc( pod1.GoHome() ) });


env3d.AddKeyListener(K_RIGHT, function(polarity) { StartProc( pod1.PushLeft(polarity ? 1 : 0) ) });

env3d.AddKeyListener(K_LEFT, function(polarity) { StartProc( pod1.PushRight(polarity ? 1 : 0) ) });

env3d.AddKeyListener(K_u, function(polarity) { StartProc( pod1.UTurn() ) });


StartProc( pod1.Init() );



while ( !endSignal ) {
	
	pause || w.Step(10);
	pod1.Draw(env3d);
	env3d.End();
	
	Step(10, 10);
	
}
