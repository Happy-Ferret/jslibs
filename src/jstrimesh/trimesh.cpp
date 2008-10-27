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
#include "trimesh.h"

#define REAL float

struct Private {

	REAL* vertices; // 1 vertex = 3 coords
	size_t vertexCount;

	u_int32_t *indices;
	size_t indexCount;
};


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Trimesh ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	if ( pv ) {

		JS_free(cx, pv);
	}
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	Private *pv = (Private*)JS_malloc(cx, sizeof(Private));
	pv->vertexCount = 0;
	pv->vertices = NULL;
	pv->indexCount = 0;
	pv->indices = NULL;
	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}


//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}

DEFINE_FUNCTION_FAST( AddVertex ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	double x, y, z;
	J_CHK( JsvalToDouble(cx, J_FARG(1), &x) );
	J_CHK( JsvalToDouble(cx, J_FARG(1), &y) );
	J_CHK( JsvalToDouble(cx, J_FARG(1), &z) );

	pv->vertices = (REAL*)JS_realloc(cx, pv->vertices, (pv->vertexCount + 1) * 3 * sizeof(REAL));
	J_S_ASSERT_ALLOC(pv->vertices);

	REAL *pos = pv->vertices + pv->vertexCount * 3;
	*(pos++) = x;
	*(pos++) = y;
	*(pos++) = z;

	J_CHK( IntToJsval(cx, pv->vertexCount, J_FRVAL) );
	pv->vertexCount += 1;

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( AddIndices ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	for ( size_t i = 0; i < argc; i++ )
		J_S_ASSERT_INT(J_FARG(i+1));

	pv->indices = (u_int32_t*)JS_realloc(cx, pv->indices, (pv->indexCount + argc) * sizeof(REAL));
	J_S_ASSERT_ALLOC(pv->vertices);

	u_int32_t *pos = pv->indices;

	u_int32_t index;
	for ( size_t i = 0; i < argc; i++ ) {

		J_CHK( JsvalToUInt(cx, J_FARG(i+1), &index) );
		*(pos++) = index;
	}

	pv->indexCount += 1;
	return JS_TRUE;
}



CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC(AddVertex, 3)
		FUNCTION_FAST(AddIndices)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
//		PROPERTY(prop)
	END_PROPERTY_SPEC

END_CLASS
