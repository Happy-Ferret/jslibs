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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( GeomTrimesh )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	if ( !geomId )
		return;
	ode::dGeomSetData(geomId, NULL);
	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshGetData(geomId);
	if ( !ode::dGeomGetBody(geomId) || _odeFinalization ) // geom is lost
		ode::dGeomDestroy(geomId);
	ode::dGeomTriMeshDataDestroy(triMeshDataID);
}


/**doc
$TOC_MEMBER $INAME
 $INAME( trimeshObject [, space ] [, preprocess] )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_RANGE(1, 3);
	JL_S_ASSERT_OBJECT(JL_ARG(1));

	ode::dSpaceID space;
	if ( JL_ARG_ISDEF(2) ) // place it in a space ?
		JL_CHK( JsvalToSpaceID(cx, JL_ARG(2), &space) );
	else
		space = 0;

	jsval trimeshVal = JL_ARG(1);
	JL_S_ASSERT( JsvalIsTrimesh(cx, trimeshVal), "Invalid Trimesh object." );
	JSObject *trimesh = JSVAL_TO_OBJECT(trimeshVal);
	Surface *srf = GetTrimeshSurface(cx, trimesh);
	JL_S_ASSERT_RESOURCE( srf );
	JL_S_ASSERT( srf->vertex && srf->vertexCount && srf->index && srf->indexCount, "No enough data." );

	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshDataCreate();
	ode::dGeomTriMeshDataBuildSingle(triMeshDataID, srf->vertex, 3 * sizeof(SURFACE_REAL_TYPE), srf->vertexCount, srf->index, srf->indexCount, 3 * sizeof(SURFACE_INDEX_TYPE));
	if ( JL_ARG_ISDEF(3) ) {
		
		bool b;
		JL_CHK( JsvalToBool(cx, JL_ARG(3), &b) );
		if ( b )
			ode::dGeomTriMeshDataPreprocess(triMeshDataID);
	}

	ode::dGeomID geomId = ode::dCreateTriMesh(space, triMeshDataID, NULL, NULL, NULL);

	JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_TRIMESH_TRIMESH, trimeshVal) );

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
DEFINE_PROPERTY( triangleCount ) {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( geomId );
	int count = ode::dGeomTriMeshGetTriangleCount(geomId);
	JL_CHK( IntToJsval(cx, count, vp) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(3)

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ( triangleCount )
	END_PROPERTY_SPEC

END_CLASS
