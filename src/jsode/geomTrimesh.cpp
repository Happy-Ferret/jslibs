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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( GeomTrimesh )

DEFINE_FINALIZE() {

	ode::dGeomID geomId = (ode::dGeomID)JL_GetPrivate(cx, obj);
	if ( geomId != NULL ) {

		ode::dGeomTriMeshDataDestroy(ode::dGeomTriMeshGetData(geomId));
		ode::dGeomSetData(geomId, NULL);
	}
}


/**doc
$TOC_MEMBER $INAME
 $INAME( space )
  TBD
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_MIN(2);
	JL_S_ASSERT_OBJECT(JL_ARG(2));

	ode::dSpaceID space = 0;
	if ( JL_ARG_ISDEF(1) ) // place it in a space ?
		JL_CHK( ValToSpaceID(cx, JL_ARG(1), &space) );

	jsval trimeshVal = JL_ARG(2);
	JL_S_ASSERT( JsvalIsTrimesh(cx, trimeshVal), "Invalid Trimesh object." );
	JSObject *trimesh = JSVAL_TO_OBJECT(trimeshVal);
	Surface *srf = GetTrimeshSurface(cx, trimesh);
	JL_S_ASSERT_RESOURCE( srf );
	JL_S_ASSERT( srf->vertex && srf->vertexCount && srf->index && srf->indexCount, "No enough data" );

	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshDataCreate();

	ode::dGeomTriMeshDataBuildSingle(triMeshDataID, srf->vertex, 3*sizeof(SURFACE_REAL_TYPE), srf->vertexCount, srf->index, srf->indexCount, 3*sizeof(SURFACE_INDEX_TYPE));
	ode::dGeomTriMeshDataPreprocess(triMeshDataID);

	ode::dGeomID geomId = ode::dCreateTriMesh(space, triMeshDataID, NULL, NULL, NULL);

	JL_CHK( JS_SetReservedSlot(cx, obj, SLOT_TRIMESH, trimeshVal) ); // keep e reference to the trimesh object because dGeomTriMeshDataBuildSingle do not make a copy of the data.
	JL_SetPrivate(cx, obj, geomId);
	JL_CHK( SetupReadMatrix(cx, obj) ); // (TBD) check return status
	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.

	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_FINALIZE

	HAS_RESERVED_SLOTS(1)

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
