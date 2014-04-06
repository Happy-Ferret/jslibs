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

/*

check this:
	void dSpaceSetCleanup (dSpaceID space, int mode);
	int dSpaceGetCleanup (dSpaceID space);
*/

#include "stdafx.h"
#include "space.h"



bool ReconstructSpace(JSContext *cx, ode::dSpaceID spaceId, JSObject **obj) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	/*
	if (unlikely( bodyId == (ode::dBodyID)0 )) { // bodyId may be null if body is world.env

		*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(Body), JL_CLASS_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
	} else {

		JL_ASSERT( ode::dBodyGetData(bodyId) == NULL, "Invalid case (object not finalized)." );
		JL_ASSERT( bodyId != NULL, "Invalid ode object." );

		*obj = jl::newObjectWithGivenProto(cx, JL_CLASS(Body), JL_CLASS_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
//		BodyPrivate *bodypv = (BodyPrivate*)jl_malloc(sizeof(BodyPrivate));
//		JL_ASSERT_ALLOC( bodypv );
//		bodypv->obj = *obj;
//		ode::dBodySetData(bodyId, bodypv);
		ode::dBodySetData(bodyId, *obj);
	}

	JL_CHK( SetMatrix44GetInterface(cx, *obj, ReadMatrix) );
	JL_SetPrivate( *obj, bodyId);
	*/

	return true;
	JL_BAD;
}



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3533 $
**/
BEGIN_CLASS( Space )

// Finalize should not destroy the item, but simply unwrap it.
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(obj);
	if ( spaceId == NULL )
		return;

//	dSpaceDestroy(spaceId); // (TBD) ???
}


/**doc
$TOC_MEMBER $INAME
 $INAME( [parentSpace] )
**/
DEFINE_CONSTRUCTOR() {

	ode::dSpaceID spaceId = NULL;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	ode::dSpaceID parentSpace;

	if ( JL_ARG_ISDEF(1) )
		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &parentSpace) );
	else
		parentSpace = 0;

	spaceId = ode::dHashSpaceCreate(parentSpace); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
	
	// doc:  If the clean-up mode is 1, then the contained geoms will be destroyed when the space is destroyed. If the clean-up mode is 0 this does not happen. The default clean-up mode for new spaces is 1.
	ode::dSpaceSetCleanup(spaceId, 0);

	//ode::dHashSpaceSetLevels(spaceId, -3, 10);

	JL_SetPrivate(obj, spaceId);
	return true;

bad:
	if ( spaceId )
		ode::dSpaceDestroy(spaceId);
	return false;

}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
**/
DEFINE_FUNCTION( destroy ) {
	
	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(spaceId);
	if ( spaceId != NULL ) {

		while ( dSpaceGetNumGeoms(spaceId) )
			dSpaceRemove(spaceId, dSpaceGetGeom(spaceId, 0));

//		ode::dSpaceClean(spaceId);
//		ode::dSpaceDestroy(spaceId);
//		JL_SetPrivate( obj, NULL);
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3533 $"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( destroy )
	END_FUNCTION_SPEC

END_CLASS
