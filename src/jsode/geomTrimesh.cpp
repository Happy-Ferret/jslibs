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
	ode::dSpaceID space = 0;
	if ( argc >= 1 ) // place it in a space ?
		J_CHK( ValToSpaceID(cx, argv[0], &space) );


	ode::dTriMeshDataID triMeshDataID = ode::dGeomTriMeshDataCreate();

//	ode::dGeomTriMeshDataBuildSimple(triMeshDataID, 

	ode::dGeomTriMeshDataPreprocess( triMeshDataID );

	ode::dGeomID geomId = ode::dCreateTriMesh(space, triMeshDataID, NULL, NULL, NULL); // default radius is 1


/*
	ode::dGeomID geomId = ode::dCreateSphere(space, 1); // default radius is 1
	JS_SetPrivate(cx, obj, geomId);
	SetupReadMatrix(cx, obj); // (TBD) check return status
*/

	ode::dGeomSetData(geomId, obj); // 'obj' do not need to be rooted because Goem's data is reset to NULL when 'obj' is finalized.

	return JS_TRUE;
}

CONFIGURE_CLASS

	HAS_FINALIZE

	HAS_PROTOTYPE( prototypeGeom )
	HAS_CONSTRUCTOR

	BEGIN_PROPERTY_SPEC
	END_PROPERTY_SPEC

	HAS_PRIVATE

END_CLASS