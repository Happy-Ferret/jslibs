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

#include "../jsfont/jsfont.h"

#include "oglft/OGLFT.h"

#define OGLFT_SLOT_FONT 0

enum {
	OUTLINE,
	FILLED,
	MONOCHROME,
	MONOCHROME_TEXTURE,
	GRAYSCALE,
	GRAYSCALE_TEXTURE,
	TRANSLUCENT,
	TRANSLUCENT_TEXTURE,
};

struct Private {

	OGLFT::Face *face;
	int type;
	int size;
};

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3060 $
**/
BEGIN_CLASS( Font3D ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;
	delete pv->face;
	jl_free(pv);
}

DEFINE_CONSTRUCTOR() { // Called when the object is constructed ( a = new Template() ) or activated ( a = Template() ). To distinguish the cases, use JS_IsConstructing() or use the JL_S_ASSERT_CONSTRUCTING() macro.

	JL_S_ASSERT_CONSTRUCTING();
	JL_S_ASSERT_THIS_CLASS();

	JL_S_ASSERT_ARG_RANGE( 2, 3 );
	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JSObject *fontObj = JSVAL_TO_OBJECT( JL_ARG(1) );
	JL_S_ASSERT_CLASS( fontObj, JL_GetRegistredNativeClass(cx, "Font") );

	FT_Face ftface = GetJsfontPrivate(cx, fontObj)->face;
	JL_S_ASSERT_RESOURCE( ftface );


	float currentSize = ftface->size->metrics.y_scale / ftface->units_per_EM;
	float size;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JsvalToFloat(cx, JL_ARG(3), &size) );
	else
		size = currentSize;

	Private *pv = (Private*)jl_calloc(1,sizeof(Private));
	JL_S_ASSERT_ALLOC(pv);
	JL_SetPrivate(cx, obj, pv);

	JL_CHK( JsvalToInt(cx, JL_ARG(2), &pv->type) );
	switch ( pv->type ) {
		case OUTLINE:
			pv->face = new OGLFT::Outline(ftface, size);
			break;
		case FILLED:
			pv->face = new OGLFT::Filled(ftface, size);
			break;
		case MONOCHROME:
			pv->face = new OGLFT::Monochrome(ftface, size);
			break;
		case MONOCHROME_TEXTURE:
			pv->face = new OGLFT::MonochromeTexture(ftface, size);
			break;
		case GRAYSCALE:
			pv->face = new OGLFT::Grayscale(ftface, size);
			break;
		case GRAYSCALE_TEXTURE:
			pv->face = new OGLFT::GrayscaleTexture(ftface, size);
			break;
		case TRANSLUCENT_TEXTURE:
			pv->face = new OGLFT::TranslucentTexture(ftface, size);
			break;
		default:
			JL_REPORT_ERROR("Invalid 3D font style");
	}

	JL_S_ASSERT( pv->face->isValid(), "Failed to create the font." );

	JL_CHK( JL_SetReservedSlot(cx, obj, OGLFT_SLOT_FONT, JL_ARG(1)) );

	return JS_TRUE;
	JL_BAD;
}

//DEFINE_FUNCTION( Call ) {
//	return JS_TRUE;
//}

DEFINE_FUNCTION_FAST( Draw ) {

	JL_S_ASSERT_ARG_RANGE( 1, 4 );
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	const char *str;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &str) );

	if ( JL_ARGC == 1 ) {

		pv->face->draw(str);
	} else {

		float x, y;
		JL_CHK( JsvalToFloat(cx, JL_FARG(1), &x) );
		JL_CHK( JsvalToFloat(cx, JL_FARG(2), &y) );
		pv->face->draw(x, y, str);
	}
	*JL_FRVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION_FAST( Compile ) {

	JL_S_ASSERT_ARG( 1 );
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	const char *str;
	JL_CHK( JsvalToString(cx, &JL_FARG(1), &str) );
	GLuint list = pv->face->compile(str);
	*JL_FRVAL = INT_TO_JSVAL(list);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( SetColor ) {

	JL_S_ASSERT_ARG_RANGE( 0, 1 );
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	*JL_FRVAL = JSVAL_VOID;
	if ( JL_ARGC == 0 ) {

		GLfloat color[4];
		glGetFloatv(GL_CURRENT_COLOR, color);
		pv->face->setForegroundColor(color);
		return JS_TRUE;
	}

	JL_S_ASSERT_ARRAY( JL_FARG(1) );

	GLfloat color[4];
	uint32 len;
	JsvalToFloatVector(cx, JL_FARG(1), color, COUNTOF(color), &len);
//	JL_S_ASSERT( len >= 3, "Invalid color." );
	if ( len < 4 )
		color[3] = 1.f;
	if ( len == 1 )
		color[1] = color[2] = color[0];
	pv->face->setForegroundColor(color);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION_FAST( SetBackgroundColor ) {

	JL_S_ASSERT_ARG_RANGE( 0, 1 );
	Private *pv = (Private*)JL_GetPrivate(cx, JL_FOBJ);
	JL_S_ASSERT_RESOURCE( pv );
	*JL_FRVAL = JSVAL_VOID;
	if ( JL_ARGC == 0 ) {

		GLclampf color[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
		pv->face->setBackgroundColor(color);
		return JS_TRUE;
	}

	JL_S_ASSERT_ARRAY( JL_FARG(1) );

	GLfloat color[4];
	uint32 len;
	JsvalToFloatVector(cx, JL_FARG(1), color, COUNTOF(color), &len);
//	JL_S_ASSERT( len >= 3, "Invalid color." );
	if ( len < 4 )
		color[3] = 1.f;
	if ( len == 1 )
		color[1] = color[2] = color[0];
	pv->face->setBackgroundColor(color);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY( height ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( DoubleToJsval(cx, pv->face->height(), vp) );
	return JL_StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}

/*
DEFINE_PROPERTY_GETTER( tessellationSteps ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	JL_CHK( IntToJsval(cx, pv->face->tessellationSteps(), vp) );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( tessellationSteps ) {

	Private *pv = (Private*)JL_GetPrivate(cx, obj);
	JL_S_ASSERT_RESOURCE( pv );
	int tessellation;
	JL_CHK( JsvalToInt(cx, *vp, &tessellation) );
	face->setTessellationSteps(tessellation);
	return JS_TRUE;
	JL_BAD;
}
*/

CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(JL_SvnRevToInt("$Revision: 3060 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // OGLFT_SLOT_FONT

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Draw)
		FUNCTION_FAST(Compile)
		FUNCTION_FAST(SetColor)
		FUNCTION_FAST(SetBackgroundColor)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(height)
//		PROPERTY( tessellationSteps )
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER_SINGLE( OUTLINE )
		CONST_INTEGER_SINGLE( FILLED )
		CONST_INTEGER_SINGLE( MONOCHROME )
		CONST_INTEGER_SINGLE( MONOCHROME_TEXTURE )
		CONST_INTEGER_SINGLE( GRAYSCALE )
		CONST_INTEGER_SINGLE( GRAYSCALE_TEXTURE )
		CONST_INTEGER_SINGLE( TRANSLUCENT )
		CONST_INTEGER_SINGLE( TRANSLUCENT_TEXTURE )
	END_CONST_INTEGER_SPEC

END_CLASS
