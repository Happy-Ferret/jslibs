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

#include "trimeshPub.h"

/*
    * glVertexPointer():  specify pointer to vertex coords array
    * glNormalPointer():  specify pointer to normal array
    * glColorPointer():  specify pointer to RGB color array
    * glTexCoordPointer():  specify pointer to texture cords array
    * glEdgeFlagPointer():  specify pointer to edge flag array

glGenBuffersARB()
glBufferDataARB()
	 
*/


/**doc
$CLASS_HEADER
**/
BEGIN_CLASS( Trimesh ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Surface *pv = (Surface*)JS_malloc(cx, sizeof(Surface));
	if ( pv ) {

		JS_free(cx, pv);
	}
}

DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	Surface *pv = (Surface*)JS_malloc(cx, sizeof(Surface));

	pv->vertexCount = 0;
	pv->indexCount = 0;

	J_CHK( JS_SetPrivate(cx, obj, pv) );
	return JS_TRUE;
}


//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}
/*
DEFINE_FUNCTION_FAST( AddVertex ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	double x, y, z;
	J_CHK( JsvalToDouble(cx, J_FARG(1), &x) );
	J_CHK( JsvalToDouble(cx, J_FARG(1), &y) );
	J_CHK( JsvalToDouble(cx, J_FARG(1), &z) );

	if ( pv->verticesDataSize < (pv->vertexCount + 1) * 3 * sizeof(REAL) ) {
		
		pv->verticesDataSize *= 2;
		pv->vertices = (REAL*)JS_realloc(cx, pv->vertices, pv->verticesDataSize);
	}

	J_S_ASSERT_ALLOC(pv->vertices);

	REAL *pos = pv->vertices + pv->vertexCount * 3;
	*(pos++) = x;
	*(pos++) = y;
	*(pos++) = z;

	J_CHK( IntToJsval(cx, pv->vertexCount, J_FRVAL) );
	pv->vertexCount += 1;

	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( AddTriangle ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	INDEX i1, i2, i3;
	J_CHK( JsvalToUInt(cx, J_FARG(1), &i1) );
	J_CHK( JsvalToUInt(cx, J_FARG(1), &i2) );
	J_CHK( JsvalToUInt(cx, J_FARG(1), &i3) );

//	pv->indices = (INDEX*)JS_realloc(cx, pv->indices, (pv->indexCount + 1) * 3 * sizeof(REAL));

	if ( pv->indicesDataSize < (pv->indexCount + 1) * sizeof(INDEX) ) {
		
		pv->verticesDataSize *= 2;
		pv->vertices = (REAL*)JS_realloc(cx, pv->vertices, pv->verticesDataSize);
	}

	J_S_ASSERT_ALLOC(pv->indices);

	INDEX *pos = pv->indices + pv->indexCount * 3;
	*(pos++) = i1;
	*(pos++) = i2;
	*(pos++) = i3;

	J_CHK( IntToJsval(cx, pv->indexCount, J_FRVAL) );
	pv->indexCount += 1;
	return JS_TRUE;
}
*/

/*
DEFINE_FUNCTION_FAST( AddIndices ) {

	Private *pv = (Private*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	for ( size_t i = 0; i < argc; i++ )
		J_S_ASSERT_INT(J_FARG(i+1));

	pv->indices = (INDEX*)JS_realloc(cx, pv->indices, (pv->indexCount + argc) * sizeof(REAL));
	J_S_ASSERT_ALLOC(pv->vertices);

	INDEX *pos = pv->indices;

	INDEX index;
	for ( size_t i = 0; i < argc; i++ ) {

		J_CHK( JsvalToUInt(cx, J_FARG(i+1), &index) );
		*(pos++) = index;
	}

	pv->indexCount += 1;
	return JS_TRUE;
}
*/


DEFINE_FUNCTION_FAST( DefineVertexBuffer ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARRAY( J_FARG(1) );
	Surface *pv = (Surface*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	JSObject *arrayObj = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint count;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	J_S_ASSERT( count % 3 == 0, "Invalid count, need [X,Y,Z,...]" );

	pv->vertex = (REAL*)JS_malloc(cx, sizeof(REAL) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
//		if ( sizeof(REAL) == sizeof(float) )
			J_CHK( JsvalToFloat(cx, item, &pv->vertex[i]) );
//		else
//			J_CHK( JsvalToDouble(cx, item, &pv->vertex[i]) );
	}
	pv->vertexCount = count / 3;
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DefineNormalBuffer ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARRAY( J_FARG(1) );
	Surface *pv = (Surface*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	JSObject *arrayObj = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint count;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	J_S_ASSERT( count % 3 == 0, "Invalid count, need [X,Y,Z,...]" );
	J_S_ASSERT_2( count == pv->vertexCount * 3, "Wrong array size %d, need %d.", count, pv->vertexCount * 3 );

	pv->color = (REAL*)JS_malloc(cx, sizeof(REAL) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToFloat(cx, item, &pv->normal[i]) );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DefineTextureCoordinateBuffer ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARRAY( J_FARG(1) );
	Surface *pv = (Surface*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	JSObject *arrayObj = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint count;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	J_S_ASSERT( count % 3 == 0, "Invalid count, need [S,T,R,...]" );
	J_S_ASSERT_2( count == pv->vertexCount * 3, "Wrong array size %d, need %d.", count, pv->vertexCount * 3 );

	pv->color = (REAL*)JS_malloc(cx, sizeof(REAL) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToFloat(cx, item, &pv->textureCoordinate[i]) );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DefineColorBuffer ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARRAY( J_FARG(1) );
	Surface *pv = (Surface*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	JSObject *arrayObj = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint count;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	J_S_ASSERT( count % 4 == 0, "Invalid count, need [R,G,B,A,...]" );
	J_S_ASSERT_2( count == pv->vertexCount * 4, "Wrong array size %d, need %d.", count, pv->vertexCount * 4 );

	pv->color = (REAL*)JS_malloc(cx, sizeof(REAL) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToFloat(cx, item, &pv->color[i]) );
	}
	return JS_TRUE;
}


DEFINE_FUNCTION_FAST( DefineIndexBuffer ) {

	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_ARRAY( J_FARG(1) );
	Surface *pv = (Surface*)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_ALLOC(pv);

	JSObject *arrayObj = JSVAL_TO_OBJECT(J_FARG(1));
	jsuint count;
	J_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	J_S_ASSERT( count % 3 == 0, "Invalid count, need [V1,V2,V3,...]" );

	pv->vertex = (REAL*)JS_malloc(cx, sizeof(REAL) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		J_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		J_CHK( JsvalToUInt(cx, item, &pv->index[i]) );
	}
	pv->indexCount = count / 3;
	return JS_TRUE;
}


DEFINE_PROPERTY( vertexCount ) {

	Surface *pv = (Surface*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_ALLOC(pv);
	J_CHK( IntToJsval(cx, pv->vertexCount, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY( indexCount ) {

	Surface *pv = (Surface*)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_ALLOC(pv);
	J_CHK( IntToJsval(cx, pv->indexCount, vp) );
	return JS_TRUE;
}



CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_PRIVATE
//	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST_ARGC(DefineVertexBuffer, 1)
		FUNCTION_FAST_ARGC(DefineNormalBuffer, 1)
		FUNCTION_FAST_ARGC(DefineTextureCoordinateBuffer, 1)
		FUNCTION_FAST_ARGC(DefineColorBuffer, 1)
		FUNCTION_FAST_ARGC(DefineIndexBuffer, 1)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(vertexCount)
		PROPERTY_READ(indexCount)
	END_PROPERTY_SPEC

END_CLASS
