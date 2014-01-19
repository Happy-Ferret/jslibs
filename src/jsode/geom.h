/* ***** BEGIN LICENSE BLOCK *****
 * Version: GNU GPL 2.0
 *
 * The contents of this file are subject to the
 * GNU General Public License Version 2.0; you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 * http://www.gnu.org/licenses/gpl.html
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * ***** END LICENSE BLOCK ***** */


#pragma once

#define SLOT_GEOM_SURFACEPARAMETER 0
#define SLOT_GEOM_CONTACT_FUNCTION 1
#define SLOT_GEOM_SPACEOBJECT 2

#define SLOT_TRIMESH_TRIMESH 3

DECLARE_CLASS( SurfaceParameters )

DECLARE_CLASS( Geom )
DECLARE_CLASS( GeomSphere )
DECLARE_CLASS( GeomBox )
DECLARE_CLASS( GeomPlane )
DECLARE_CLASS( GeomCapsule )
DECLARE_CLASS( GeomCylinder )
DECLARE_CLASS( GeomRay )
DECLARE_CLASS( GeomConvex )
DECLARE_CLASS( GeomTrimesh )

bool SetupReadMatrix(JSContext *cx, JSObject *obj);

void FinalizeGeom(JSObject *obj);

bool ReconstructGeom(JSContext *cx, ode::dGeomID geomId, JSObject **obj);

ALWAYS_INLINE bool JL_JsvalIsGeom( const jsval val ) {

	return !JSVAL_IS_PRIMITIVE(val) && JL_GetClass(JSVAL_TO_OBJECT( val )) == JL_CLASS(Geom);
}

ALWAYS_INLINE bool GeomHasJsObj( ode::dGeomID geomId ) {
	
	return ode::dGeomGetData(geomId) != NULL;
}

ALWAYS_INLINE JSBool GeomToJsval( JSContext *cx, ode::dGeomID geomId, jsval *val ) {

	JSObject *obj = (JSObject*)ode::dGeomGetData(geomId);
	if (unlikely( !obj ))
		JL_CHK( ReconstructGeom(cx, geomId, &obj) );
	JL_ASSERT_INHERITANCE(obj, JL_CLASS(Geom));
	*val = OBJECT_TO_JSVAL( obj );
	return JS_TRUE;
	JL_BAD;
}


ALWAYS_INLINE JSBool JL_JsvalToGeom( JSContext *cx, const jsval val, ode::dGeomID *geom ) {

	JL_ASSERT_IS_OBJECT(val, JL_CLASS_NAME(Geom));
	JL_ASSERT_INSTANCE(JSVAL_TO_OBJECT(val), JL_CLASS(Geom));
	*geom = (ode::dGeomID)JL_GetPrivate(JSVAL_TO_OBJECT(val));
	JL_ASSERT_OBJECT_STATE( *geom, JL_CLASS_NAME(Geom) );
	return JS_TRUE;
	JL_BAD;
}