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

#include "trimeshPub.h"

DECLARE_CLASS( Trimesh )

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
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Trimesh ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	if ( JL_GetHostPrivate(cx)->canSkipCleanup ) // do not cleanup in unsafe mode.
		return;

	Surface *pv = (Surface*)JL_GetPrivate(cx, obj);
	if ( !pv )
		return;

	if ( pv->vertex )
		JS_free(cx, pv->vertex);
	if ( pv->normal )
		JS_free(cx, pv->normal);
	if ( pv->textureCoordinate )
		JS_free(cx, pv->textureCoordinate);
	if ( pv->color )
		JS_free(cx, pv->color);

	if ( pv->index )
		JS_free(cx, pv->index);

	JS_free(cx, pv);
}

DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	Surface *pv;
	pv = (Surface*)JS_malloc(cx, sizeof(Surface));
	JL_CHK( pv );
	memset(pv, 0, sizeof(Surface));

	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


//DEFINE_PROPERTY( prop ) {
//	return JS_TRUE;
//}
/*
DEFINE_FUNCTION( AddVertex ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	double x, y, z;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &x) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &y) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &z) );

	if ( pv->verticesDataSize < (pv->vertexCount + 1) * 3 * sizeof(SURFACE_REAL_TYPE) ) {
		
		pv->verticesDataSize *= 2;
		pv->vertices = (SURFACE_REAL_TYPE*)JS_realloc(cx, pv->vertices, pv->verticesDataSize);
	}

	JL_CHK( pv->vertices );

	SURFACE_REAL_TYPE *pos = pv->vertices + pv->vertexCount * 3;
	*(pos++) = x;
	*(pos++) = y;
	*(pos++) = z;

	JL_CHK( JL_NativeToJsval(cx, pv->vertexCount, JL_RVAL) );
	pv->vertexCount += 1;

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( AddTriangle ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	INDEX i1, i2, i3;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &i1) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &i2) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &i3) );

//	pv->indices = (INDEX*)JS_realloc(cx, pv->indices, (pv->indexCount + 1) * 3 * sizeof(SURFACE_REAL_TYPE));

	if ( pv->indicesDataSize < (pv->indexCount + 1) * sizeof(SURFACE_INDEX_TYPE) ) {
		
		pv->verticesDataSize *= 2;
		pv->vertices = (SURFACE_REAL_TYPE*)JS_realloc(cx, pv->vertices, pv->verticesDataSize);
	}

	JL_CHK( pv->indices );

	SURFACE_INDEX_TYPE *pos = pv->indices + pv->indexCount * 3;
	*(pos++) = i1;
	*(pos++) = i2;
	*(pos++) = i3;

	JL_CHK( JL_NativeToJsval(cx, pv->indexCount, JL_RVAL) );
	pv->indexCount += 1;
	return JS_TRUE;
	JL_BAD;
}
*/

/*
DEFINE_FUNCTION( AddIndices ) {

	Private *pv = (Private*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	for ( size_t i = 0; i < argc; i++ )
		JL_ASSERT_INT(JL_ARG(i+1));

	pv->indices = (SURFACE_INDEX_TYPE*)JS_realloc(cx, pv->indices, (pv->indexCount + argc) * sizeof(SURFACE_REAL_TYPE));
	JL_CHK( pv->vertices );

	SURFACE_INDEX_TYPE *pos = pv->indices;

	SURFACE_INDEX_TYPE index;
	for ( size_t i = 0; i < argc; i++ ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(i+1), &index) );
		*(pos++) = index;
	}

	pv->indexCount += 1;
	return JS_TRUE;
	JL_BAD;
}
*/


DEFINE_FUNCTION( DefineVertexBuffer ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_ARRAY(1);
	Surface *pv;
	pv  = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	*JL_RVAL = JSVAL_VOID;

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(JL_ARG(1));
	jsuint count;
	JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	if ( count == 0 )
		return JS_TRUE;

	JL_ASSERT( (count > 0) && (count % 3 == 0), E_ARG, E_NUM(1), E_INVALID, E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid count, need [X,Y,Z, ...]"

	pv->vertex = (SURFACE_REAL_TYPE*)JS_malloc(cx, sizeof(SURFACE_REAL_TYPE) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &item) );
//		if ( sizeof(SURFACE_REAL_TYPE) == sizeof(float) )
		JL_CHK( JL_JsvalToNative(cx, item, &pv->vertex[i]) );
//		else
//			JL_CHK( JL_JsvalToNative(cx, item, &pv->vertex[i]) );
	}
	pv->vertexCount = count / 3;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( DefineNormalBuffer ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_ARRAY(1);
	Surface *pv;
	pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	*JL_RVAL = JSVAL_VOID;

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(JL_ARG(1));
	jsuint count;
	JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	if ( count == 0 )
		return JS_TRUE;

	JL_ASSERT( (count > 0) && (count % 3 == 0) && count == pv->vertexCount * 3, E_ARG, E_NUM(1), E_INVALID, E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid count, need [X,Y,Z, ...]"

	pv->normal = (SURFACE_REAL_TYPE*)JS_malloc(cx, sizeof(SURFACE_REAL_TYPE) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		JL_CHK( JL_JsvalToNative(cx, item, &pv->normal[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( DefineTextureCoordinateBuffer ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_ARRAY(1);
	Surface *pv;
	pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	*JL_RVAL = JSVAL_VOID;

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(JL_ARG(1));
	jsuint count;
	JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	if ( count == 0 )
		return JS_TRUE;

	JL_ASSERT( (count > 0) && (count % 3 == 0) && count == pv->vertexCount * 3, E_ARG, E_NUM(1), E_INVALID, E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid count, need [S,T,R, ...]"

	pv->textureCoordinate = (SURFACE_REAL_TYPE*)JS_malloc(cx, sizeof(SURFACE_REAL_TYPE) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		JL_CHK( JL_JsvalToNative(cx, item, &pv->textureCoordinate[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( DefineColorBuffer ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_ARRAY(1);
	Surface *pv;
	pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	*JL_RVAL = JSVAL_VOID;

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(JL_ARG(1));
	jsuint count;
	JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	if ( count == 0 )
		return JS_TRUE;

	JL_ASSERT( (count > 0) && (count % 4 == 0) && count == pv->vertexCount * 4, E_ARG, E_NUM(1), E_INVALID, E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid count, need [R,G,B,A, ...]"

	pv->color = (SURFACE_REAL_TYPE*)JS_malloc(cx, sizeof(SURFACE_REAL_TYPE) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		JL_CHK( JL_JsvalToNative(cx, item, &pv->color[i]) );
	}
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( DefineIndexBuffer ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN( 1 );
	JL_ASSERT_ARG_IS_ARRAY(1);
	Surface *pv;
	pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	*JL_RVAL = JSVAL_VOID;

	JSObject *arrayObj;
	arrayObj = JSVAL_TO_OBJECT(JL_ARG(1));
	jsuint count;
	JL_CHK( JS_GetArrayLength(cx, arrayObj, &count) );

	if ( count == 0 )
		return JS_TRUE;

	JL_ASSERT( (count > 0) && (count % 3 == 0), E_ARG, E_NUM(1), E_INVALID, E_SEP, E_ARRAYLENGTH, E_INVALID ); // "Invalid count, need [V1,V2,V3, ...]"

	pv->index = (SURFACE_INDEX_TYPE*)JS_malloc(cx, sizeof(SURFACE_INDEX_TYPE) * count);

	jsval item;
	for ( jsuint i = 0; i < count; i++ ) {

		JL_CHK( JS_GetElement(cx, arrayObj, i, &item) );
		JL_CHK( JL_JsvalToNative(cx, item, &pv->index[i]) );
	}
	pv->indexCount = count;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( vertexCount ) {

	Surface *pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( JL_NativeToJsval(cx, pv->vertexCount, vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( indexCount ) {

	Surface *pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_CHK( JL_NativeToJsval(cx, pv->indexCount, vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( hasNormal ) {

	Surface *pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = BOOLEAN_TO_JSVAL( pv->normal != NULL );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( hasTextureCoordinateBuffer ) {

	Surface *pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = BOOLEAN_TO_JSVAL( pv->normal != NULL );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( hasColor ) {

	Surface *pv = (Surface*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	*vp = BOOLEAN_TO_JSVAL( pv->normal != NULL );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_ARGC(DefineVertexBuffer, 1)
		FUNCTION_ARGC(DefineNormalBuffer, 1)
		FUNCTION_ARGC(DefineTextureCoordinateBuffer, 1)
		FUNCTION_ARGC(DefineColorBuffer, 1)
		FUNCTION_ARGC(DefineIndexBuffer, 1)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( vertexCount )
		PROPERTY_GETTER( indexCount )
		PROPERTY_GETTER( hasNormal )
		PROPERTY_GETTER( hasTextureCoordinateBuffer )
		PROPERTY_GETTER( hasColor )
	END_PROPERTY_SPEC

END_CLASS
