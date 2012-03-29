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

#include "stdafx.h"
#include "space.h"
#include "geom.h"

#include "../jstrimesh/trimeshPub.h"

/**doc
$CLASS_HEADER Geom
$SVN_REVISION $Revision: 3528 $
**/
BEGIN_CLASS( GeomTrimesh )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	if ( !geomId )
		return;
	ode::dGeomSetData(geomId, NULL);
	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshGetData(geomId);
	if ( !ode::dGeomGetBody(geomId) ) // geom is lost
		ode::dGeomDestroy(geomId);
	ode::dGeomTriMeshDataDestroy(triMeshDataID);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( trimeshObject [, space ] [, preprocess] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	jsval trimeshVal;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_RANGE(1, 3);
	JL_ASSERT_ARG_IS_OBJECT(1);

	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(2) ) { // place it in a space ?

		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(2), &space) );
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_GEOM_SPACEOBJECT, JL_ARG(2)) );
	} else {

		space = 0;
	}

	trimeshVal = JL_ARG(1);
	JL_ASSERT( JL_JsvalIsTrimesh(cx, trimeshVal), E_ARG, E_NUM(1), E_TYPE, E_STR("Trimesh") );

	JSObject *trimesh = JSVAL_TO_OBJECT(trimeshVal);
	Surface *srf = GetTrimeshSurface(cx, trimesh);
	JL_ASSERT_OBJECT_STATE( srf, JL_GetClassName(trimesh) );

	JL_ASSERT( srf->vertex && srf->vertexCount && srf->index && srf->indexCount, E_ARG, E_NUM(1), E_SEP, E_DATASIZE, E_INVALID );

	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshDataCreate();
	ode::dGeomTriMeshDataBuildSingle(triMeshDataID, srf->vertex, 3 * sizeof(SURFACE_REAL_TYPE), srf->vertexCount, srf->index, srf->indexCount, 3 * sizeof(SURFACE_INDEX_TYPE));
	if ( JL_ARG_ISDEF(3) ) {
		
		bool b;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &b) );
		if ( b )
			ode::dGeomTriMeshDataPreprocess(triMeshDataID);
	}

	ode::dGeomID geomId = ode::dCreateTriMesh(space, triMeshDataID, NULL, NULL, NULL);

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_TRIMESH_TRIMESH, trimeshVal) );

	JL_SetPrivate(cx, obj, geomId);
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.
	JL_CHK( SetupReadMatrix(cx, obj) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  TBD
**/
DEFINE_PROPERTY_GETTER( triangleCount ) {

	JL_IGNORE(id);
	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( geomId );
	int count = ode::dGeomTriMeshGetTriangleCount(geomId);
	JL_CHK( JL_NativeToJsval(cx, count, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3528 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(4)

	HAS_PROTOTYPE( Geom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( triangleCount )
	END_PROPERTY_SPEC

END_CLASS
