<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsode/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsode/qa.js) -
# jsode module #

> jsode is a module that manages support to ODE.
> ODE is an open source, high performance library for simulating rigid body dynamics.
> It is fully featured, stable, mature and platform independent with an easy to use C/C++ API.
> It has advanced joint types and integrated collision detection with friction.
> ODE is useful for simulating vehicles, objects in virtual reality environments and virtual creatures.
> It is currently used in many computer games, 3D authoring tools and simulation tools.
> ##### note: #####
> > In the following API description, <sub>vec3</sub> type is a js 3 dimentions <sub>Array</sub> like `[1,3,5]`



---

## class jsode::World ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/world.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>()
> > dWorldCreate
> > ##### note: #####
> > > This function creates also a Space (space) object and a SurfaceParameters object (defaultSurfaceParameters).

### Methods ###

#### <font color='white' size='1'><b>Destroy</b></font> ####

> <b>Destroy</b>()
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>Step</b></font> ####

> <b>Step</b>( time [[.md](.md), iterations] )
> > ##### note: #####
> > > If the _iterations_ argument is given, this uses an iterative method that takes time on the order of m\*N and memory on the order of m, where m is the total number of constraint rows and N is the number of iterations.

### Properties ###

#### <font color='white' size='1'><b>gravity</b></font> ####

> <sub>vec3</sub> <b>gravity</b>
> > Gets or sets the gravity vector for a given world.

#### <font color='white' size='1'><b>real</b></font> ####

> <sub>real</sub> **ERP**
> > dWorldGetERP

  * <sub>real</sub> **CFM**
> > dWorldGetCFM

  * <sub>real</sub> **contactSurfaceLayer**
> > dWorldGetContactSurfaceLayer

#### <font color='white' size='1'><b>env</b></font> ####

> <sub>Body</sub> <b>env</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Returns the environment object that is the fixed body of this world (like the ground).

#### <font color='white' size='1'>???</font> ####
  * efaultSurfaceParameters**![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > This defines the default properties of the colliding grometries surfaces.
> >**<br />
> > The property is read-only but not i'ts content.

  * **space**  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > This is the default space object that is bound to the world.


---

## class jsode::Body ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/body.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( _world_ )
> > <b><font color='red'>TBD</font></b>

### Methods ###

#### <font color='white' size='1'><b>Destroy</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Destroy</b>()
> > <b><font color='red'>TBD</font></b> dBodyDestroy

#### <font color='white' size='1'><b>IsConnectedTo</b></font> ####

> <sub>boolean</sub> <b>IsConnectedTo</b>( _body_ )
> > <b><font color='red'>TBD</font></b> dAreConnected

### Properties ###

#### <font color='white' size='1'><b>vector</b></font> ####

> <sub>vec3</sub> **position**
> > dBodySetPosition

  * <sub>vec4</sub> **quaternion**
> > dBodySetQuaternion

  * <sub>vec3</sub> **linearVel**
> > dBodySetLinearVel

  * <sub>vec3</sub> **angularVel**
> > dBodySetAngularVel

  * <sub>vec3</sub> **force**
> > dBodySetForce

  * <sub>vec3</sub> **torque**
> > dBodySetTorque

#### <font color='white' size='1'><b>mass</b></font> ####

> <b>mass</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the mass object (jsode::Mass) of the body.

### Native Interface ###
  * **NIMatrix44Read**
> > Is the current body's position.


---

## class jsode::Geom ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geom.cpp?r=2555) -

### Methods ###

#### <font color='white' size='1'><b>Destroy</b></font> ####
  * estroy**()
> > dGeomSetData NULL, dGeomDestroy**

### Properties ###

#### <font color='white' size='1'><b>enable</b></font> ####

> <sub>boolean</sub> **enable**
> > Is the status of the geometry.

#### <font color='white' size='1'><b>body</b></font> ####

> <sub>body</sub> **body**
> > Bind the current geometry to the given body object.

#### <font color='white' size='1'><b>offset</b></font> ####
  * ffset**> > Sets the position and rotation of the geometry to its center of mass.
> >**<br />
> > Use _undefined_ value to reset the geometry offset.

#### <font color='white' size='1'><b>tansformation</b></font> ####
  * ansformation**> > Sets the position and rotation of the geometry to its center of mass.**

#### <font color='white' size='1'><b>position</b></font> ####

> <sub>vec3</sub> **position**
> > Is the current position of the geometry.

### Callback functions ###
  * **impact**(index, thisGeom, againstGeom, position);
> > This function is called each time two geometries collide together.
> > _index_ is the index of the collision between step and step+1.
> > <br />
> > _thisGeom_ is the geometry that is colliding (usualy, `this` object).
> > <br />
> > _againstGeom_ is the geometry against with this geometry is colliding (the other Geom).
> > <br />
> > <sub>vec3</sub> _position_ is the position of the impact point in world position.

### Native Interface ###
  * **NIMatrix44Read**
> > Is the current geometry's position.


---

## class jsode::GeomBox<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomBox.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>lengths</b></font> ####

> <sub>vec3</sub> <b>lengths</b>
> > Is the x, y, z size of the box.


---

## class jsode::GeomCapsule<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomCapsule.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>radius</b></font> ####

> <sub>real</sub> <b>radius</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>length</b></font> ####

> <sub>real</sub> <b>length</b>
> > Is the length of the capsule.


---

## class jsode::GeomPlane<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomPlane.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>


---

## class jsode::GeomRay<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomRay.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>length</b></font> ####

> <sub>real</sub> <b>length</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>start</b></font> ####

> <sub>vec3</sub> <b>start</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>direction</b></font> ####

> <sub>vec3</sub> <b>direction</b>
> > <b><font color='red'>TBD</font></b>


---

## class jsode::GeomSphere<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomSphere.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>radius</b></font> ####

> <sub>real</sub> <b>radius</b>


---

## class jsode::GeomTrimesh<sup>jsode::Geom</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/geomTrimesh.cpp?r=2557) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( space )
> > <b><font color='red'>TBD</font></b>


---

## class jsode::Joint ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/joint.cpp?r=2556) -

### Methods ###

#### <font color='white' size='1'><b>Destroy</b></font> ####

> <b>Destroy</b>()
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>body1</b></font> ####

> <sub>Body</sub> <b>body1</b>
> > Set the first body of the joint.

#### <font color='white' size='1'><b>body2</b></font> ####

> <sub>Body</sub> <b>body2</b>
> > Set the second body of the joint.

#### <font color='white' size='1'><b>useFeedback</b></font> ####

> <sub>boolean</sub> <b>useFeedback</b>
> > Set to _true_ activates the feedback, and `ALSE` to desactivates it.
> > <br />
> > Using feedback will allows body1Force, body1Torque, body2Force and body2Torque to be used.

#### <font color='white' size='1'><b>feedbackVector</b></font> ####

> <sub>vec3</sub> **body1Force**
> > Is the current force vector that applies to the body1 if feedback is activated.

  * <sub>vec3</sub> **body1Torque**
> > Is the current torque vector that applies to the body1 if feedback is activated.

  * <sub>vec3</sub> **body2Force**

> Is the current force vector that applies to the body2 if feedback is activated.

  * <sub>vec3</sub> **body2Torque**
> > Is the current torque vector that applies to the body2 if feedback is activated.

#### <font color='white' size='1'><b>jointParam</b></font> ####

> <sub>real</sub> **loStop**

  * <sub>real</sub> **hiStop**

  * <sub>real</sub> **bounce**

  * <sub>real</sub> **CFM**

  * <sub>real</sub> **stopERP**

  * <sub>real</sub> **stopCFM**

  * <sub>real</sub> **velocity**

  * <sub>real</sub> **maxForce**


---

## class jsode::JointBall<sup>jsode::Joint</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/jointBall.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( world )

### Properties ###

#### <font color='white' size='1'><b>anchor</b></font> ####
> <sub>vec3</sub> <b>anchor</b>

#### <font color='white' size='1'><b>anchor2</b></font> ####
> <sub>vec3</sub> <b>anchor2</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)


---

## class jsode::JointFixed<sup>jsode::Joint</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/jointFixed.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( world )

### Methods ###

#### <font color='white' size='1'><b>Set</b></font> ####
> <b>Set</b>()
> > Set the current position of body1 and body2 as fixed.


---

## class jsode::JointHinge<sup>jsode::Joint</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/jointHinge.cpp?r=2555) -

### Methods ###

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( world )
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>AddTorque</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>AddTorque</b>( torque )
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>anchor</b></font> ####

> <sub>vec3</sub> <b>anchor</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>anchor2</b></font> ####

> <sub>vec3</sub> <b>anchor2</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>axis</b></font> ####

> <sub>vec3</sub> <b>axis</b>
> > Get or set the axis of the joint.

#### <font color='white' size='1'><b>angle</b></font> ####

> <sub>real</sub> <b>angle</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get the current angle.

#### <font color='white' size='1'><b>angleRate</b></font> ####

> <sub>real</sub> <b>angleRate</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Get the current rotation speed.


---

## class jsode::JointPlane<sup>jsode::Joint</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/jointPlane.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( world )
> > <b><font color='red'>TBD</font></b>


---

## class jsode::JointSlider<sup>jsode::Joint</sup> ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/jointSlider.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( world )
> > <b><font color='red'>TBD</font></b>

### Properties ###

#### <font color='white' size='1'><b>axis</b></font> ####

> <sub>vec3</sub> <b>axis</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>position</b></font> ####

> <sub>real</sub> <b>position</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>positionRate</b></font> ####

> <sub>real</sub> <b>positionRate</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > <b><font color='red'>TBD</font></b>


---

## class jsode::Mass ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/mass.cpp?r=2555) -

### Methods ###

#### <font color='white' size='1'><b>Translate</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Translate</b>( vec3 )
> > <b><font color='red'>TBD</font></b> dMassTranslate + dBodySetMass

#### <font color='white' size='1'><b>Adjust</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Adjust</b>( mass )
> > <b><font color='red'>TBD</font></b> dBodyGetMass, dMassAdjust, dBodySetMass

#### <font color='white' size='1'><b>SetZero</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>SetZero</b>()
> > <b><font color='red'>TBD</font></b> dBodyGetMass, dMassSetZero, dBodySetMass

#### <font color='white' size='1'><b>SetBoxTotal</b></font> ####

> <b>SetBoxTotal</b>( mass, vec3 )
> > <b><font color='red'>TBD</font></b> dBodyGetMass, dMassSetBoxTotal, dBodySetMass

### Properties ###

#### <font color='white' size='1'><b>value</b></font> ####

> <sub>real</sub> <b>value</b>
> > <b><font color='red'>TBD</font></b> dBodyGetMass, dBodySetMass

#### <font color='white' size='1'><b>center</b></font> ####

> <sub>vec3</sub> <b>center</b>
> > <b><font color='red'>TBD</font></b> dBodyGetMass, dBodySetMass
> > get/set a _vec3_


---

## class jsode::Space ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/space.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( parentSpace )


---

## class jsode::SurfaceParameters ##
- [top](#jsode_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsode/surfaceParameters.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>()
> > ##### note: #####
> > > by default, mu is set to Infinity.

### Properties ###

#### <font color='white' size='1'><b>surface</b></font> ####
  * <sub>real</sub> **mu**

> > Coulomb friction coefficient. This must be in the range 0 to dInfinity. 0 results in a frictionless contact, and dInfinity results in a contact that never slips.
> > Note that frictionless contacts are less time consuming to compute than ones with friction, and infinite friction contacts can be cheaper than contacts with finite friction.
> > This must always be set.

  * <sub>real</sub> **mu2**
> > Optional Coulomb friction coefficient for friction direction 2 (0..dInfinity).

  * <sub>real</sub> **bounce**
> > Restitution parameter (0..1). 0 means the surfaces are not bouncy at all, 1 is maximum bouncyness.

  * <sub>real</sub> **bounceVel**
> > The minimum incoming velocity necessary for bounce. Incoming velocities below this will effectively have a bounce parameter of 0.

  * <sub>real</sub> **softERP**
> > Contact normal "softness" parameter.

  * <sub>real</sub> **softCFM**
> > Contact normal "softness" parameter.

  * <sub>real</sub> **motion1**, <sub>real</sub> **motion2**, <sub>real</sub> motionN
> > Surface velocity in friction directions 1 and 2 and along the normal.

  * <sub>real</sub> **slip1**, <sub>real</sub> **slip2**
> > The coefficients of force-dependent-slip (FDS) for friction directions 1 and 2.


> ##### note: #####
> > Use _undefined_ as value to reset the property.


> ##### ODE API: #####
> > [dSurfaceParameters](http://opende.sourceforge.net/wiki/index.php/Manual_(Joint_Types_and_Functions)#Contact)


---

- [top](#jsode_module.md) - [main](JSLibs.md) -
