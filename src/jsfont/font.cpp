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
#include <jlmoduleprivate.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_SYNTHESIS_H

/*
typedef struct {
	FT_Face face;
	int borderWidth;
	int borderHeight;
	bool useKerning;
	float letterSpacingFactor;
	bool italic;
	bool bold;
} FaceInfo;
*/

/**doc fileIndex:top **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_CLASS( Font ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	if ( pv == NULL )
		return;
	FT_Done_Face(pv->face);
//	JL_SetPrivate(cx, obj, NULL);
	jl_free(pv);
}

/**doc
$TOC_MEMBER $INAME
 $INAME( filePathName [, faceIndex = 0] )
  Creates a new Font object and seletc the face to use.
  $H arguments
   $ARG $STR filePathName: the path of the font file.
   $ARG $INT faceIndex: the index of the face to use.
**/
DEFINE_CONSTRUCTOR() {

	JLStr filePathName;

	JL_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_ASSERT_ARGC_MIN(1);

//	ModulePrivate *mpv = (ModulePrivate*)GetModulePrivate(cx, _moduleId);

	int faceIndex;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &faceIndex) );
	else
		faceIndex = 0;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &filePathName) );

	JsfontModulePrivate *mpv;
	mpv = (JsfontModulePrivate*)ModulePrivateGet();

	JsfontPrivate *pv;
	pv = (JsfontPrivate*)jl_malloc(sizeof(JsfontPrivate));
	JL_ASSERT_ALLOC( pv );
	JL_updateMallocCounter(cx, sizeof(JsfontPrivate));

	FTCHK( FT_New_Face( mpv->ftLibrary, filePathName, faceIndex, &pv->face ) );
	// from memory: FT_New_Memory_Face
	// see. FT_Open_Face
	JL_ASSERT_ALLOC(pv->face);

	JL_SetPrivate(cx, obj, pv);
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/


/*
DEFINE_FUNCTION( SetSize ) {

	JL_ASSERT_ARGC_MIN(2);

	FT_Face face = (FT_Face)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(face);

	FT_UInt width, height;

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &width) ); // a value of 0 for one of the dimensions means same as the other.
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &height) );

	FT_Error status;
	status = FT_Set_Pixel_Sizes(face, width, height);
	JL_ASSERT( status == 0, "Unable to FT_Set_Pixel_Sizes." );

	return JS_TRUE;
	JL_BAD;
}
*/

/*
int MoveToFunc( const FT_Vector* to, void* user ) {

	return 0;
}
int LineToFunc( const FT_Vector* to, void* user ) {

	return 0;
}
int ConicToFunc( const FT_Vector* control, const FT_Vector* to, void* user ) {

	return 0;
}
int CubicToFunc( const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* user ) {

	return 0;
}

DEFINE_FUNCTION( GetCharOutline ) {

	JL_ASSERT_ARGC_MIN(1);

	FT_Face face;
	face = (FT_Face)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(face);

	FT_Outline_Funcs funcs;
	funcs.move_to = MoveToFunc;
	funcs.line_to = LineToFunc;
	funcs.conic_to = ConicToFunc;
	funcs.cubic_to = CubicToFunc;
	funcs.delta = 0;
	funcs.shift = 0;

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_ARG(1));
	JL_ASSERT( jsstr != NULL, "Invalid string." );
	JL_ASSERT( JL_GetStringLength(jsstr) == 1, "Invalid char" );
	jschar *str;
	str = JS_GetStringChars(jsstr);
	JL_ASSERT( str != NULL, "Invalid string." );

	FT_UInt glyphIndex = FT_Get_Char_Index( face, str[0] );
	FT_Error error = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT );

	FT_OutlineGlyph g;
	FTCHK( FT_Get_Glyph( face->glyph, (FT_Glyph*)&g ) );

	FTCHK( FT_Outline_Decompose(&g->outline, &funcs, NULL) );

	FT_Done_Glyph( (FT_Glyph)g );
	return JS_TRUE;
	JL_BAD;
}
*/

/**doc
$TOC_MEMBER $INAME
 $TYPE imageObject $INAME( oneChar )
  Draws one char with the current face.
  $H arguments
   $ARG $STR oneChar: string of one char.
  $H return value
   An image object that contains the char.
**/
DEFINE_FUNCTION( DrawChar ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);

	JsfontPrivate *pv;
	pv = (JsfontPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ASSERT( pv->face->size->metrics.height > 0 ); // JL_ASSERT_ERROR_NUM( pv->face->size->metrics.height > 0, JLSMSG_VALUE_OUTOFRANGE, "height > 0" );

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_ARG(1));
	JL_ASSERT( jsstr, E_ARG, E_NUM(1), E_TYPE, E_TY_CHAR );
	
//	JL_ASSERT( JL_GetStringLength(jsstr) == 1, "Invalid char" );
//	jschar *str;
//	str = JS_GetStringChars(jsstr);
//	JL_ASSERT( str != NULL, "Invalid string." );

	size_t strlen;
	const jschar *str;
	str = JS_GetStringCharsAndLength(cx, jsstr, &strlen);
	JL_ASSERT( strlen == 1, E_ARG, E_NUM(1), E_TYPE, E_TY_CHAR );

	FTCHK( FT_Load_Char(pv->face, str[0], FT_LOAD_RENDER) );

	int width;
	width = pv->face->glyph->bitmap.width;
	int height;
	height = pv->face->glyph->bitmap.rows;

	size_t bufLength;
	bufLength = width * height * 1; // 1 channel

	uint8_t *buf;
	buf = (uint8_t*)JS_malloc(cx, bufLength +1);
	JL_CHK( buf );

	buf[bufLength] = 0;
	JL_CHK( JL_NewBlob(cx, buf, bufLength, JL_RVAL) );
	JSObject *blobObj;
	JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
	*JL_RVAL = OBJECT_TO_JSVAL( blobObj );

	JL_CHK(JL_SetProperty(cx, blobObj, "width", width) );
	JL_CHK(JL_SetProperty(cx, blobObj, "height", height) );
	JL_CHK(JL_SetProperty(cx, blobObj, "channels", 1) );
	memcpy( buf, pv->face->glyph->bitmap.buffer, bufLength );

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $TYPE imageObject | $TYPE integer $INAME( text [, keepTrailingSpace = false] [, getWidthOnly = false ] )
  Draws a string with the current face.
  $H arguments
   $ARG $STR text: the single-line text to draw.
   $ARG $BOOL keepTrailingSpace: if true, the last letter separator space is keept.
   $ARG $BOOL getWidthOnly: if true, the function will return the length (in pixel) of the _text_.
  $H return value
   An image object that contains the text or the length of the text in pixel.
**/
DEFINE_FUNCTION( DrawString ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_ARGC_MIN(1);

	JsfontPrivate *pv;
	pv = (JsfontPrivate*)JL_GetPrivate(cx, JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	ASSERT( pv->face->size->metrics.height > 0 ); // JL_ASSERT_ERROR_NUM( pv->face->size->metrics.height > 0, JLSMSG_VALUE_OUTOFRANGE, "height > 0" );

	JSString *jsstr;
	jsstr = JS_ValueToString(cx, JL_ARG(1));
	JL_ASSERT( jsstr, E_ARG, E_NUM(1), E_TYPE, E_TY_STRING );

	//jschar *str;
	//str = JS_GetStringChars(jsstr);
	//JL_ASSERT( str != NULL, "Invalid string." );
	//size_t strlen;
	//strlen = JL_GetStringLength(jsstr);

	size_t strlen;
	const jschar *str;
	str = JS_GetStringCharsAndLength(cx, jsstr, &strlen);



	bool keepTrailingSpace;
	keepTrailingSpace = false;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &keepTrailingSpace) );

	bool getWidthOnly;
	getWidthOnly = false;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &getWidthOnly) );


	jsval tmp;

	int letterSpacing;
	letterSpacing = 0;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_LETTERSPACING, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &letterSpacing) );

	int horizontalPadding;
	horizontalPadding = 0;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_HORIZONTALPADDING, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &horizontalPadding) );

	int verticalPadding;
	verticalPadding = 0;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_VERTICALPADDING, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &verticalPadding) );

	bool useKerning;
	useKerning = true;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_USEKERNING, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &useKerning) );

	bool isItalic;
	isItalic = false;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_ITALIC, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &isItalic) );

	bool isBold;
	isBold = false;
	JL_CHK( JL_GetReservedSlot(cx, obj, FONT_SLOT_BOLD, &tmp) );
	if ( !JSVAL_IS_VOID( tmp ) )
		JL_CHK( JL_JsvalToNative(cx, tmp, &isBold) );


	typedef struct {
		FT_Vector pos; // glyph origin on the baseline
		FT_Glyph image; // glyph image
	} Glyph;

	Glyph glyphs_static[32]; // memory optimization only
	Glyph *glyphs;
	if ( strlen > sizeof(glyphs_static)/sizeof(*glyphs_static) ) {

		glyphs = (Glyph*)jl_malloc(sizeof(Glyph) * strlen);
		JL_ASSERT_ALLOC( glyphs );
	} else {

		glyphs = glyphs_static;
	}

	size_t i;

	FT_Pos posX;
	posX = 0;
	FT_Pos posY;
	posY = 0;
	FT_UInt prevGlyphIndex;
	prevGlyphIndex = 0;
	FT_Pos advance;
	advance = 0;

	for ( i=0; i<strlen; i++ ) {

		FT_UInt glyphIndex = FT_Get_Char_Index( pv->face, str[i] );
		FTCHK( FT_Load_Glyph(pv->face, glyphIndex, FT_LOAD_DEFAULT) );
		if ( isItalic )
			FT_GlyphSlot_Oblique(pv->face->glyph);
		if ( isBold )
			FT_GlyphSlot_Embolden(pv->face->glyph);

		FTCHK( FT_Get_Glyph(pv->face->glyph, &glyphs[i].image) );

		glyphs[i].pos.x = posX;
		glyphs[i].pos.y = posY + pv->face->size->metrics.ascender;

		// prepare next char
		advance = 0;
		if ( useKerning && prevGlyphIndex && glyphIndex ) {

			FT_Vector delta;
			FTCHK( FT_Get_Kerning( pv->face, prevGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta ) );
			advance += delta.x;
		}

		advance += pv->face->glyph->advance.x + (letterSpacing << 6);
		posX += advance;
		prevGlyphIndex = glyphIndex;
	}

	if ( !keepTrailingSpace ) { // (TBD) enhance this

		posX += -advance + pv->face->glyph->metrics.horiBearingX + pv->face->glyph->metrics.width; // we do not need the letterSpacing at the end of the text, but we need to advance by the glyph width.
	}

	// Doc.	The ascender is the vertical distance from the horizontal baseline to the highest character coordinate in a font face.
	//			Unfortunately, font formats define the ascender differently. For some, it represents the ascent of all capital latin characters (without accents),
	//			for others it is the ascent of the highest accented character, and finally, other formats define it as being equal to global_bbox.yMax.
	posY += pv->face->size->metrics.ascender + -pv->face->size->metrics.descender;

   // here, text extents from (0,0) to (posX,posY)

	int width;
	width = (posX >> 6) + horizontalPadding * 2; // * 2 because left and right border
	int height;
	height = (posY >> 6) + verticalPadding * 2; // ...

	if ( getWidthOnly ) {

		*JL_RVAL = INT_TO_JSVAL( width );
	} else {

		// allocates the resulting image buffer
		size_t bufLength = width * height * 1; // 1 channel

		char *buf = (char*)JS_malloc(cx, bufLength +1); // JS_malloc do not supports 0 bytes size and blob need a trailing 0
		JL_CHK( buf );
		memset(buf, 0, bufLength +1);

		JL_CHK( JL_NewBlob(cx, buf, bufLength, JL_RVAL) );
		JSObject *blobObj;
		JL_CHK( JS_ValueToObject(cx, *JL_RVAL, &blobObj) );
		*JL_RVAL = OBJECT_TO_JSVAL( blobObj );

		JL_CHK(JL_SetProperty(cx, blobObj, "width", width) );
		JL_CHK(JL_SetProperty(cx, blobObj, "height", height) );
		JL_CHK(JL_SetProperty(cx, blobObj, "channels", 1) );

		// render glyphs in the bitmap
		for ( i=0; i<strlen; i++ ) {

			if ( glyphs[i].image->format != FT_GLYPH_FORMAT_BITMAP )
				FTCHK( FT_Glyph_To_Bitmap( &(glyphs[i].image), FT_RENDER_MODE_NORMAL, 0, 1 ) );

			FT_BitmapGlyph bitmap = (FT_BitmapGlyph)glyphs[i].image;

			int dPosX = horizontalPadding + (glyphs[i].pos.x >> 6) + bitmap->left;
			int dPosY = verticalPadding + (glyphs[i].pos.y >> 6) - bitmap->top; // bitmap->top is the vertical distance from the pen position (on the baseline) to the topmost border of the glyph bitmap.

			int x, y, px, py;
			for ( y=0; y<bitmap->bitmap.rows; y++ ) {

				py = dPosY + y;
				if ( py >= 0 && py < height ) {

					for ( x=0; x<bitmap->bitmap.width; x++ ) {

						px = dPosX + x;
						if ( px >= 0 &&  px < width ) {

							buf[ px + py * width ] |= (char)bitmap->bitmap.buffer[ x + y * bitmap->bitmap.width ];
						}
					}
				}
			}
			FT_Done_Glyph( glyphs[i].image );
		}
	}

	if ( glyphs != glyphs_static ) // memory optimization only
		jl_free(glyphs);
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the ascender length (in pixel) of the current face.
  $H note
   The ascender is the portion of a letter in a Latin-derived alphabet that extends above the mean line of a font. That is, the part of the letter that is taller than the font's x-height.
**/
DEFINE_PROPERTY_GETTER( ascender ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	*vp = INT_TO_JSVAL(pv->face->size->metrics.ascender >> 6);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the descender length (in pixel) of the current face.
  $H note
   The descender is the portion of a letter in a Latin alphabet that extends below the baseline of a font. For example, in the letter y, the descender would be the "tail," or that portion of the diagonal line which lies below the v created by the two lines converging.
**/
DEFINE_PROPERTY_GETTER( descender ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	*vp = INT_TO_JSVAL(pv->face->size->metrics.descender >> 6);
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the maximum width (in pixel) of the current face.
**/
DEFINE_PROPERTY_GETTER( width ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	*vp = INT_TO_JSVAL(pv->face->size->metrics.max_advance >> 6);
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  is the size (in pixel) of the current face.
**/
DEFINE_PROPERTY_SETTER( size ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	FT_UInt size;
	if ( !JSVAL_IS_VOID( *vp ) )
		JL_CHK( JL_JsvalToNative(cx, *vp, &size) );
	else
		size = 0;

	FTCHK( FT_Set_Pixel_Sizes(pv->face, size, size) );
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE enum $INAME
  is the current encoding.
  $H supported encodings
   see constants section below.
**/
DEFINE_PROPERTY_SETTER( encoding ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	unsigned int encoding;
	JL_CHK( JL_JsvalToNative(cx, *vp, &encoding) );
	FTCHK( FT_Select_Charmap(pv->face, (FT_Encoding)encoding) );
	return JL_StoreProperty(cx, obj, id, vp, false);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME $READONLY
  is the postscript name of the face.
**/
DEFINE_PROPERTY_GETTER( poscriptName ) {

	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	JL_CHK( JL_NativeToJsval(cx, FT_Get_Postscript_Name(pv->face), vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  enable or disable kerning usage for the current face.
  $H note
   Kerning is the process of adjusting letter spacing in a proportional font. In a well-kerned font, the two-dimensional blank spaces between each pair of letters all have similar area.
**/
DEFINE_PROPERTY_GETTER( useKerning ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_USEKERNING, vp);
}

DEFINE_PROPERTY_SETTER( useKerning ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_USEKERNING, *vp);
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  is the size (in pixel) of the horizontal padding of any drawn text i.e. the space before and after the text.
**/
DEFINE_PROPERTY_GETTER( horizontalPadding ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_HORIZONTALPADDING, vp);
}

DEFINE_PROPERTY_SETTER( horizontalPadding ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_HORIZONTALPADDING, *vp);
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  is the size (in pixel) of the vertical padding of any drawn text i.e. the space above and below the text.
**/
DEFINE_PROPERTY_GETTER( verticalPadding ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_VERTICALPADDING, vp);
}

DEFINE_PROPERTY_SETTER( verticalPadding ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_VERTICALPADDING, *vp);
}


/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  is the length (in pixel) of the additional space added between each letter in a text.
**/
DEFINE_PROPERTY_GETTER( letterSpacing ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_LETTERSPACING, vp);
}

DEFINE_PROPERTY_SETTER( letterSpacing ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_LETTERSPACING, *vp);
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  enable or disable italic.
**/
DEFINE_PROPERTY_GETTER( italic ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_ITALIC, vp);
}

DEFINE_PROPERTY_SETTER( italic ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_ITALIC, *vp);
}


/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  enable or disable bold.
**/
DEFINE_PROPERTY_GETTER( bold ) {

	return JL_GetReservedSlot(cx, obj, FONT_SLOT_BOLD, vp);
}

DEFINE_PROPERTY_SETTER( bold ) {

	return JL_SetReservedSlot(cx, obj, FONT_SLOT_BOLD, *vp);
}

/**doc
=== Constants ===
**/

/**doc

 Encoding constants
  $CONST NONE

  $CONST MS_SYMBOL

  $CONST UNICODE

  $CONST SJIS

  $CONST GB2312

  $CONST BIG5

  $CONST WANSUNG

  $CONST JOHAB

  $CONST MS_SJIS

  $CONST MS_GB2312

  $CONST MS_BIG5

  $CONST MS_WANSUNG

  $CONST MS_JOHAB

  $CONST ADOBE_STANDARD

  $CONST ADOBE_EXPERT

  $CONST ADOBE_CUSTOM

  $CONST ADOBE_LATIN_1

  $CONST OLD_LATIN_2

  $CONST APPLE_ROMAN

**/


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(7)

	BEGIN_FUNCTION_SPEC
//		FUNCTION(GetCharOutline)
		FUNCTION(DrawString)
		FUNCTION(DrawChar)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( poscriptName )
		PROPERTY_SETTER( size )
		PROPERTY( useKerning )
		PROPERTY( horizontalPadding )
		PROPERTY( verticalPadding )
		PROPERTY( letterSpacing )
		PROPERTY( italic )
		PROPERTY( bold )
		PROPERTY_SETTER( encoding )
		PROPERTY_GETTER( ascender )
		PROPERTY_GETTER( descender )
		PROPERTY_GETTER( width )
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( NONE				,FT_ENCODING_NONE				)
		CONST_INTEGER( MS_SYMBOL		,FT_ENCODING_MS_SYMBOL		)
		CONST_INTEGER( UNICODE			,FT_ENCODING_UNICODE			)
		CONST_INTEGER( SJIS				,FT_ENCODING_SJIS				)
		CONST_INTEGER( GB2312			,FT_ENCODING_GB2312			)
		CONST_INTEGER( BIG5				,FT_ENCODING_BIG5				)
		CONST_INTEGER( WANSUNG			,FT_ENCODING_WANSUNG			)
		CONST_INTEGER( JOHAB			,FT_ENCODING_JOHAB			)
		CONST_INTEGER( MS_SJIS			,FT_ENCODING_MS_SJIS			)
		CONST_INTEGER( MS_GB2312		,FT_ENCODING_MS_GB2312		)
		CONST_INTEGER( MS_BIG5			,FT_ENCODING_MS_BIG5			)
		CONST_INTEGER( MS_WANSUNG		,FT_ENCODING_MS_WANSUNG		)
		CONST_INTEGER( MS_JOHAB			,FT_ENCODING_MS_JOHAB		)
		CONST_INTEGER( ADOBE_STANDARD	,FT_ENCODING_ADOBE_STANDARD)
		CONST_INTEGER( ADOBE_EXPERT		,FT_ENCODING_ADOBE_EXPERT	)
		CONST_INTEGER( ADOBE_CUSTOM		,FT_ENCODING_ADOBE_CUSTOM	)
		CONST_INTEGER( ADOBE_LATIN_1	,FT_ENCODING_ADOBE_LATIN_1	)
		CONST_INTEGER( OLD_LATIN_2		,FT_ENCODING_OLD_LATIN_2	)
		CONST_INTEGER( APPLE_ROMAN		,FT_ENCODING_APPLE_ROMAN	)
	END_CONST_INTEGER_SPEC

END_CLASS


/**doc
=== Examples ===
$H example 1
 Write "Hello world" in the file text.png
{{{
LoadModule('jslang');
LoadModule('jsstd');
LoadModule('jsfont');
LoadModule('jsimage');
LoadModule('jsprotex');
LoadModule('jsio');

var f = new Font('arial.ttf');
f.size = 100;
f.verticalPadding = -16;
var img = f.DrawString('Hello world', true);

var t = new Texture(img);
var t1 = new Texture(t);

t.BoxBlur(10,10);
t1.OppositeLevels();
t.Add(t1);
t.OppositeLevels();
t.Add(1);

new File('text.png').content = EncodePngImage(t.Export());
}}}
**/




/*
	API: http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_Done_FreeType
	Glyph API: http://www.freetype.org/freetype2/docs/reference/ft2-glyph_management.html
	Tutorial: http://www.freetype.org/freetype2/docs/tutorial/step1.html

	on-line demo: http://mbox.troja.mff.cuni.cz/~peak/ftdemo/index.cgi
	-> code: http://www.koders.com/c/fid49219DA46C44B7DFDA3807DC8B0CADCEE972CEB7.aspx

*/
