LoadModule('jsode');

/// crash 1

  var world = new World;
  var ball = new GeomSphere(world.space);
  ball.body = new Body(world);
