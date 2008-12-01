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

#define SLOT_TRIMESH 0

/**doc
$CLASS_HEADER Geom
**/
BEGIN_CLASS( GeomTrimesh )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JS_GetPrivate(cx, obj);
	if ( geomId != NULL ) {

		ode::dGeomTriMeshDataDestroy(ode::dGeomTriMeshGetData(geomId));
		ode::dGeomSetData(geomId, NULL);
	}
}


/**doc
 * $INAME( space )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();

	J_S_ASSERT_ARG_MIN(2);
	J_S_ASSERT_OBJECT(J_ARG(2));

	ode::dSpaceID space = 0;
	if ( J_ARG_ISDEF(1) ) // place it in a space ?
		J_CHK( ValToSpaceID(cx, J_ARG(1), &space) );

	jsval trimeshVal = J_ARG(2);
	JSObject *trimesh = JSVAL_TO_OBJECT(trimeshVal);
	J_S_ASSERT( IsTrimeshObject(cx, trimesh), "Invalid Trimesh object." );
	Surface *srf = GetTrimeshSurface(cx, trimesh);
	J_S_ASSERT_RESOURCE( srf );
	J_S_ASSERT( srf->vertex && srf->vertexCount && srf->index && srf->indexCount, "No enough data" );

	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshDataCreate();

	ode::dGeomTriMeshDataBuildSingle(triMeshDataID, srf->vertex, 3*sizeof(SURFACE_REAL_TYPE), srf->vertexCount, srf->index, srf->indexCount, sizeof(SURFACE_INDEX_TYPE));
	ode::dGeomTriMeshDataPreprocess(triMeshDataID);

	ode::dGeomID geomId = ode::dCreateTriMesh(space, triMeshDataID, NULL, NULL, NULL);

	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_TRIMESH, trimeshVal) ); // keep e reference to the trimesh object because dGeomTriMeshDataBuildSingle do not make a copy of the data.
	J_CHK( JS_SetPrivate(cx, obj, geomId) );
	J_CHK( SetupReadMatrix(cx, obj) ); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.

	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))
	HAS_FINALIZE

	HAS_RESERVED_SLOTS(1)

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS
