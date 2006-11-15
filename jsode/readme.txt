Note from GeomBox Finalize:

	 // [TBD] manage destruction dependency: geom - space, body - world, but how to ?

	// read the next comment !!
//	ode::dGeomID GeomId = (ode::dGeomID)JS_GetPrivate(cx,obj);
//	if ( GeomId != NULL )
//		ode::dGeomDestroy(GeomId);

	// [TBD] really destroy the geom when finalise ? think that even if there is no more references to this object, 
	//       the geom remain managed by ODE, so what to do ??? ...
	//       in the case we use geom as a property of Body ??


Links
-----

World(X) -- Body(rSlot)
==> Body may be GC !

World(X) -- Joint(X)
==> Joint may be GC !

Joint(rSlot) -- body1(X)
             -- body2(X)
==> Joint may be GC !

Body(prop mass) -- mass(rSlot)
==> no GC risk

Body(X) -- Geom(prop body)
==> Geom may be GC !

Space(X) -- Geom(X)
==> Space may be GC !
==> Geom may be GC !


Everything that may be GC SHOULD not have a Finalize:
- Body, Joint, Space, Geom
But these functions should have a Destroy function