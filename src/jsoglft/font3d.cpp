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


#define OGLFT_SLOT_FONT 0

enum {
	OUTLINE,
	FILLED,
	SOLID,
	MONOCHROME,
	MONOCHROME_TEXTURE,
	GRAYSCALE,
	GRAYSCALE_TEXTURE,
	TRANSLUCENT,
	TRANSLUCENT_TEXTURE,
};


class ColorTess : public OGLFT::ColorTess {

	JSRuntime *_rt;
	JSObject *_obj;
	jsval _function;
	GLfloat _colorTmp[4]; // the GLfloat* color() return value directly goes in glColor()

	GLfloat* color( GLdouble* p ) {

		JSContext *cx = JL_GetFirstContext(_rt);
		jsval arg[2] = { JSVAL_NULL, JSVAL_NULL }; // memset(arg, 0, sizeof(arg));
		JL_CHK( JL_NativeVectorToJsval(cx, p, 3, &arg[1], false) );
		JL_CHK( JS_CallFunctionValue(cx, _obj, _function, COUNTOF(arg)-1, arg+1, arg) );
		uint32_t length;
		JL_CHK( JL_JsvalToNativeVector(cx, *arg, _colorTmp, COUNTOF(_colorTmp), &length) );
		return _colorTmp;
	bad:
		JS_ReportPendingException(cx);
		return _colorTmp;
	}

public:
	ColorTess(JSRuntime *rt, JSObject *obj, jsval function) : _rt(rt), _obj(obj), _function(function) {
	}
};


struct Private {

	OGLFT::Face *face;
	FT_Face ftface;
	int style;
	int size;
};

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3060 $
**/
BEGIN_CLASS( Font3D ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	if ( JL_GetHostPrivate(fop->runtime())->canSkipCleanup )
		return;

	Private *pv = (Private*)JL_GetPrivate(obj);
	if ( pv == NULL )
		return;
	if ( pv->face == NULL ) // has not been properly initialized. see constructor.
		return;

	if ( pv->style == OUTLINE || pv->style == FILLED || pv->style == SOLID ) {

		ASSERT( pv->face != NULL );
		OGLFT::Polygonal *poly;
//		poly = dynamic_cast<OGLFT::Polygonal*>(pv->face);
//		poly = (OGLFT::Polygonal*)pv->face;
		poly = static_cast<OGLFT::Polygonal*>(pv->face);
		OGLFT::ColorTess *colorTess = poly->colorTess();
		if ( colorTess != NULL )
			delete colorTess;
	}
	delete pv->face;
	jl_free(pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( font, style [, size] )
  $H arguments
   $ARG $OBJ font: a font object from the jsfont module.
   $ARG $ENUM style: the drawing style:
    $CONST OUTLINE
    $CONST FILLED
    $CONST SOLID
    $CONST MONOCHROME
    $CONST MONOCHROME_TEXTURE
    $CONST GRAYSCALE
    $CONST GRAYSCALE_TEXTURE
    $CONST TRANSLUCENT
	 $CONST TRANSLUCENT_TEXTURE
   $ARG $REAL size: the point size of the font to generate. A point is essentially 1/72th of an inch. By default, the size of _font_ is used.
  $H beware
   For the raster styles (MONOCHROME, GRAYSCALE and TRANSLUCENT), it is essential that the pixel store unpacking alignment be set to 1.
  $H example
{{{
Ogl.pixelStore( Ogl.UNPACK_ALIGNMENT, 1 );
var font = new Font('c:\\windows\\fonts\\arial.ttf');
var font3d = new Font3D(f, Font3D.GRAYSCALE, 48);
...
f3d.draw('Hello World');
}}}
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_ASSERT_ARGC_RANGE( 2, 3 );
	JL_ASSERT_ARG_IS_OBJECT(1);
	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JSObject *fontObj = &JL_ARG(1).toObject();

	JL_ASSERT_INSTANCE( fontObj, JL_GetCachedClass(JL_GetHostPrivate(cx), "Font") );

	FT_Face ftface = GetJsfontPrivate(cx, fontObj)->face;
	JL_ASSERT_OBJECT_STATE( ftface, JL_GetClassName(fontObj) );

	float currentSize = (float)ftface->size->metrics.y_scale / (float)ftface->units_per_EM;
	float size;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &size) );
	else
		size = currentSize;

	pv = (Private*)jl_calloc(1, sizeof(Private));
	JL_ASSERT_ALLOC(pv);
	JL_updateMallocCounter(cx, sizeof(Private));

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pv->style) );
	switch ( pv->style ) {
		case OUTLINE:
			pv->face = new OGLFT::Outline(ftface, size);
			break;
		case FILLED:
			pv->face = new OGLFT::Filled(ftface, size);
			break;
		case SOLID:
			pv->face = new OGLFT::Solid(ftface, size);
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
		case TRANSLUCENT:
			pv->face = new OGLFT::Translucent(ftface, size);
			break;
		case TRANSLUCENT_TEXTURE:
			pv->face = new OGLFT::TranslucentTexture(ftface, size);
			break;
		default:
			JL_ERR( E_ARG, E_NUM(2), E_INVALID );
	}

	JL_ASSERT( pv->face->isValid(), E_LIB, E_OPERATION, E_STR("font"), E_CREATE );

	pv->ftface = ftface;
	pv->size = (int)size;

//	pv->face->setCompileMode(OGLFT::Face::COMPILE);

	return JL_SetReservedSlot( obj, OGLFT_SLOT_FONT, JL_ARG(1)); // GC protection

	JL_SetPrivate(obj, pv);
	return true;

bad:
	if ( pv ) {

		// (TBD) delete poly->colorTess() ? see finalize
		if ( pv->face )
			delete pv->face; // static_cast<OGLFT::Polygonal*>(pv->face);
		jl_free(pv);
	}
	return false;
}

//DEFINE_FUNCTION( call ) {
//	return true;
//}



/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( string [, absolute = false ] )
  Compute the bounding box info for a string.
  $H arguments
   $ARG $STR string: the string to measure.
   $ARG $BOOL absolute: if true, compute the bounding box info for a string with conversion to modeling coordinates.
**/
DEFINE_FUNCTION( measure ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE( 1, 2 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	bool absolute;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &absolute) );
	else
		absolute = false;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	{
		OGLFT::BBox bbox = absolute ? pv->face->measure(str) : pv->face->measureRaw(str);
		JSObject *arrObj = JS_NewArrayObject(cx, 4);
		JL_CHK( arrObj );
		*JL_RVAL = OBJECT_TO_JSVAL(arrObj);
		jsval tmp;
		JL_CHK(JL_NativeToJsval(cx, bbox.x_min_, &tmp) );
		JL_CHK( JL_SetElement(cx, arrObj, 0, &tmp) );
		JL_CHK(JL_NativeToJsval(cx, bbox.y_min_, &tmp) );
		JL_CHK( JL_SetElement(cx, arrObj, 1, &tmp) );
		JL_CHK(JL_NativeToJsval(cx, bbox.x_max_, &tmp) );
		JL_CHK( JL_SetElement(cx, arrObj, 2, &tmp) );
		JL_CHK(JL_NativeToJsval(cx, bbox.y_max_, &tmp) );
		JL_CHK( JL_SetElement(cx, arrObj, 3, &tmp) );
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $ARRAY $INAME( string )
  Compute the width for a string.
  $H arguments
   $ARG $STR string: the string to measure.
**/
DEFINE_FUNCTION( width ) {
	
	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC( 1 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	float vector_scale_ = ( pv->size * 100 ) / ( 72.f * pv->ftface->units_per_EM );

	{
		OGLFT::BBox bbox = pv->face->measureRaw(str);
		JL_CHK(JL_NativeToJsval(cx, bbox.x_max_ * vector_scale_, JL_RVAL) );
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( string [, x, y ] )
  Draw a string using the current MODELVIEW matrix or at the given 2D point.
  $H arguments
   $ARG $STR string: the string to draw.
   $ARG $REAL x: the X position.
   $ARG $REAL y: the Y position.
  $H example
{{{
var f = new Font('c:\\windows\\fonts\\arial.ttf');
var f3d = new Font3D(f, Font3D.SOLID, 12);
...
f3d.draw('test');
}}}
**/
DEFINE_FUNCTION( draw ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE( 1, 3 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

//	const char *str;
//	size_t length;
//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &str, &length) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );

	if ( JL_ARGC >= 2 ) {

		float x, y;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &x) );
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &y) );
		pv->face->draw(x, y, str);
	} else {

		if ( str.Length() == 1 )
			pv->face->draw(str.GetConstStr()[0]);
		else
			pv->face->draw(str);
	}

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( string )
  Compile a string into an OpenGL display list for later rendering.
  Essentially, the string is rendered at the origin of the current MODELVIEW.
  $H note
   No other display lists should be open when this routine is called. Also, the Face does not keep track of these lists, so you must delete them in order to recover the memory.
  $H arguments
   $ARG $STR string: the string to draw.
  $H return value
   the display list name for the string.
**/
DEFINE_FUNCTION( compile ) {

	JLData str;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC( 1 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	GLuint list = pv->face->compile(str);
	*JL_RVAL = INT_TO_JSVAL(list);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ color ] )
  This is the nominal color of the glyphs. A lot of other things can alter what you actually see! $LF
  If the _color_ argument is ommited, the current OpenGL color is used instead (see. Ogl.Color).
  $H node 
   Changing the foreground color invalidates the glyph cache.
  $H arguments
   $ARG $ARR color: array of 4 values corresponding to the red, green, blue and alpha components of the foreground color.
**/
DEFINE_FUNCTION( setColor ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE( 0, 1 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setUndefined();
	if ( JL_ARGC == 0 ) {

		GLfloat color[4];
		glGetFloatv(GL_CURRENT_COLOR, color);
		pv->face->setForegroundColor(color);
		return true;
	}

	JL_ASSERT_ARG_IS_ARRAY(1);

	GLfloat color[4];
	uint32_t len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), color, COUNTOF(color), &len) );
//	JL_ASSERT( len >= 3, "Invalid color." );
	if ( len < 4 )
		color[3] = 1.f;
	if ( len == 1 )
		color[1] = color[2] = color[0];
	pv->face->setForegroundColor(color);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [ color ] )
  This is the nominal background color of the glyphs. A lot of other things can alter what you actually see! $LF
  If the _color_ argument is ommited, the current OpenGL clear color is used instead (see Ogl.ClearColor).
  $H note
   changing the background color invalidates the glyph cache.
  $H arguments
   $ARG $ARR color: array of 4 values corresponding to the red, green, blue and alpha components of the background color.
**/
DEFINE_FUNCTION( setBackgroundColor ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE( 0, 1 );

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_RVAL.setUndefined();

	if ( JL_ARGC == 0 ) {

		GLclampf color[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
		pv->face->setBackgroundColor(color);
		return true;
	}

	JL_ASSERT_ARG_IS_ARRAY(1);

	GLfloat color[4];
	uint32_t len;
	JL_CHK( JL_JsvalToNativeVector(cx, JL_ARG(1), color, COUNTOF(color), &len) );
//	JL_ASSERT( len >= 3, "Invalid color." );
	if ( len < 4 )
		color[3] = 1.f;
	if ( len == 1 )
		color[1] = color[2] = color[0];
	pv->face->setBackgroundColor(color);

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME $READONLY
  Is the height (i.e., line spacing) at the current character size.
**/
DEFINE_PROPERTY_GETTER( height ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK( JL_NativeToJsval(cx, pv->face->height(), vp) );
	return jl::StoreProperty(cx, obj, id, vp, true);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  If advance is true, then the changes made to the MODELVIEW matrix to render a string are allowed to remain.
  Otherwise, the library pushes the current MODELVIEW matrix onto the matrix stack,
  renders the string and then pops it off again. Rendering a character always modifies the MODELVIEW matrix.
**/
DEFINE_PROPERTY_GETTER( advance ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_CHK(JL_NativeToJsval(cx, pv->face->advance(), vp) );
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( advance ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	bool advance;
	JL_CHK( JL_JsvalToNative(cx, *vp, &advance) );
	pv->face->setAdvance(advance);
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Is the number of steps to tessellate each curved segment of a glyph outline.$LF
  TrueType and Type1 files describe the boundaries of glyphs with quadratic and cubic curves, respectively.
  Since OpenGL can only really draw straight lines, these curves have to be tessellated.
  The number of steps used is fixed for all glyphs in the face, but can be changed through this property.
  $H note
   This value is only applicable for OUTLINE, FILLED and SOLID styles.
   Changing this value invalidates any cached display lists for glyphs in this face.
**/
DEFINE_PROPERTY_SETTER( tessellationSteps ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_ASSERT( pv->style == FILLED || pv->style == SOLID || pv->style == OUTLINE, E_THISOPERATION, E_NOTSUPPORTED ); // "operation not supported with this style of object"

	OGLFT::Polygonal *poly;
//	poly = dynamic_cast<OGLFT::Polygonal*>(pv->face);
//	poly = (OGLFT::Polygonal*)pv->face;
	poly = static_cast<OGLFT::Polygonal*>(pv->face);

	int tess;
	JL_CHK( JL_JsvalToNative(cx, *vp, &tess) );
	JL_ASSERT( tess >= 1, E_VALUE, E_MIN, E_NUM(1) ); 

	poly->setTessellationSteps(tess);
	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $FUNCTION $INAME
  Each tesselated vertex is passed to this function, which returns a color for that position in space.
  $H example
{{{
var f = new Font('c:\\windows\\fonts\\arial.ttf');
var f3d = new Font3D(f, Font3D.FILLED, 48);
f3d.colorCallback = function( pos ) {

	return [pos[0]/20, pos[1]/20, pos[2]/20, 1]; // Bob ?
}
...
f3d.draw('Marley');
}}}
**/
DEFINE_PROPERTY_SETTER( colorCallback ) {

	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private*)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	JL_ASSERT( pv->style == FILLED || pv->style == SOLID || pv->style == OUTLINE, E_THISOPERATION, E_NOTSUPPORTED );

	OGLFT::Polygonal *poly;
//	poly = dynamic_cast<OGLFT::Polygonal*>(pv->face);
//	poly = (OGLFT::Polygonal*)pv->face;
	poly = static_cast<OGLFT::Polygonal*>(pv->face);

	if ( vp.isUndefined() ) {

		OGLFT::ColorTess *colorTess = poly->colorTess();
		if ( colorTess != NULL ) {

			delete colorTess;
			poly->setColorTess(NULL);
		}
	} else {

		JL_ASSERT_IS_CALLABLE(*vp, "");
		OGLFT::ColorTess *colorTess = new ColorTess(JL_GetRuntime(cx), obj, *vp);
		poly->setColorTess(colorTess);
	}

	return jl::StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/*
DEFINE_FUNCTION( setCharacterDisplayLists ) {

	OGLFT::DisplayLists lists;

	JL_ASSERT_ARGC( 1 );
	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_ASSERT_ARG_IS_ARRAY(1);
	JSObject *arrObj = JSVAL_TO_OBJECT(JL_ARG(1));
	unsigned length;
	JL_CHK( JS_GetArrayLength(cx, arrObj, &length) );
	for ( unsigned i = 0; i < length; i++ ) {

		JL_CHK( JL_GetElement(cx, arrObj, i, JL_RVAL) );
		JL_ASSERT_INT( *JL_RVAL );
		lists.push_back(JL_RVAL.toInt32());
	}
	pv->face->setCharacterDisplayLists(lists);
	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}
*/


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(jl::SvnRevToInt("$Revision: 3060 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1) // OGLFT_SLOT_FONT

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(measure)
		FUNCTION(width)
		FUNCTION(draw)
		FUNCTION(compile)
		FUNCTION(setColor)
		FUNCTION(setBackgroundColor)
//		FUNCTION(setCharacterDisplayLists)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( height )
		PROPERTY( advance )
		PROPERTY_SETTER( tessellationSteps )
		PROPERTY_SETTER( colorCallback )
	END_PROPERTY_SPEC

	BEGIN_CONST
		CONST_INTEGER_SINGLE( OUTLINE )
		CONST_INTEGER_SINGLE( FILLED )
		CONST_INTEGER_SINGLE( SOLID )
		CONST_INTEGER_SINGLE( MONOCHROME )
		CONST_INTEGER_SINGLE( MONOCHROME_TEXTURE )
		CONST_INTEGER_SINGLE( GRAYSCALE )
		CONST_INTEGER_SINGLE( GRAYSCALE_TEXTURE )
		CONST_INTEGER_SINGLE( TRANSLUCENT )
		CONST_INTEGER_SINGLE( TRANSLUCENT_TEXTURE )
	END_CONST

END_CLASS
