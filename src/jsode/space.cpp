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



JSBool ReconstructSpace(JSContext *cx, ode::dSpaceID spaceId, JSObject **obj) { // (TBD) JSObject** = Conservative Stack Scanning issue ?

	/*
	if (unlikely( bodyId == (ode::dBodyID)0 )) { // bodyId may be null if body is world.env

		*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(Body), JL_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
	} else {

		JL_ASSERT( ode::dBodyGetData(bodyId) == NULL, "Invalid case (object not finalized)." );
		JL_ASSERT( bodyId != NULL, "Invalid ode object." );

		*obj = JS_NewObjectWithGivenProto(cx, JL_CLASS(Body), JL_PROTOTYPE(cx, Body), NULL);
		JL_CHK( *obj );
//		BodyPrivate *bodypv = (BodyPrivate*)jl_malloc(sizeof(BodyPrivate));
//		JL_ASSERT_ALLOC( bodypv );
//		bodypv->obj = *obj;
//		ode::dBodySetData(bodyId, bodypv);
		ode::dBodySetData(bodyId, *obj);
	}

	JL_CHK( SetMatrix44GetInterface(cx, *obj, ReadMatrix) );
	JL_SetPrivate(cx, *obj, bodyId);
	*/

	return JS_TRUE;
	JL_BAD;
}



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Space )

// Finalize should not destroy the item, but simply unwrap it.
DEFINE_FINALIZE() {

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx, obj);
	if ( spaceId == NULL )
		return;
}


/**doc
$TOC_MEMBER $INAME
 $INAME( parentSpace )
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	ode::dSpaceID parentSpace;
	if ( argc >= 1 )
		JL_CHK( JL_JsvalToSpaceID(cx, JL_ARG(1), &parentSpace) );
	else
		parentSpace = 0;

	ode::dSpaceID spaceId = ode::dHashSpaceCreate(parentSpace); // dSimpleSpaceCreate / dHashSpaceCreate / dQuadTreeSpaceCreate
	ode::dSpaceSetCleanup(spaceId, 0); // manual cleanup
	//ode::dHashSpaceSetLevels(spaceId, -3, 10);

	JL_SetPrivate(cx, obj, spaceId);
	return JS_TRUE;
	JL_BAD;
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

	ode::dSpaceID spaceId = (ode::dSpaceID)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(spaceId);
	if ( spaceId != NULL ) {

		while ( dSpaceGetNumGeoms(spaceId) )
			dSpaceRemove(spaceId, dSpaceGetGeom(spaceId, 0));

//		ode::dSpaceClean(spaceId);
//		ode::dSpaceDestroy(spaceId);
//		JL_SetPrivate(cx, obj, NULL);
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE

	BEGIN_FUNCTION_SPEC
		FUNCTION( destroy )
	END_FUNCTION_SPEC

END_CLASS
