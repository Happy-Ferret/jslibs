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
#include "font.h"

#include <math.h>

#include "../jslang/bstringapi.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define FONT_SLOT_BORDERWIDTH 0
#define FONT_SLOT_USEKERNING 1
#define FONT_SLOT_SIZE 2
#define FONT_SLOT_LETTERSPACING 3

extern FT_Library _freetype;

// FREETYPE_ERROR


BEGIN_CLASS( Font ) // Start the definition of the class. It defines some symbols: _name, _class, _prototype

DEFINE_FINALIZE() { // called when the Garbage Collector is running if there are no remaing references to this object.

	FT_Face face = (FT_Face)JS_GetPrivate(cx, obj);
	if ( face != NULL ) {

		FT_Done_Face(face);
		JS_SetPrivate(cx, obj, NULL);
	}
}

DEFINE_CONSTRUCTOR() { // Called when the object is constructed ( a = new Template() ) or activated ( a = Template() ). To distinguish the cases, use JS_IsConstructing() or use the RT_ASSERT_CONSTRUCTING() macro.

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);

	const char *facePath;
	J_CHECK_CALL( JsvalToString(cx, J_ARG(1), &facePath) );

	FT_Long faceIndex;

	if ( J_ARG_ISDEF(2) ) {
		
		J_JSVAL_TO_INT32(J_ARG(2), faceIndex);
	} else {
		
		faceIndex = 0;
	}

	FT_Face face;
	FT_Error status;
	status = FT_New_Face( _freetype, facePath, 0, &face );
	// from memory: FT_New_Memory_Face
	// see. FT_Open_Face

	J_S_ASSERT_1( status == 0, "Unable to create font face (%s).", facePath );
	J_S_ASSERT_RESOURCE(face);
	J_CHECK_CALL( JS_SetPrivate(cx, obj, face) );
	return JS_TRUE;
}

/*
DEFINE_FUNCTION_FAST( SetSize ) {

	J_S_ASSERT_ARG_MIN(2);

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);

	FT_UInt width, height;

	J_JSVAL_TO_INT32( J_FARG(1), width ); // a value of 0 for one of the dimensions means ‘same as the other’.
	J_JSVAL_TO_INT32( J_FARG(2), height );

	FT_Error status;
	status = FT_Set_Pixel_Sizes(face, width, height);
	J_S_ASSERT( status == 0, "Unable to FT_Set_Pixel_Sizes." );

	return JS_TRUE;
}
*/


DEFINE_FUNCTION_FAST( Draw ) {

	J_S_ASSERT_ARG_MIN(1);

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);


	bool getWidthOnly;
	if ( J_FARG_ISDEF(2) )
		J_JSVAL_TO_BOOL(J_FARG(2), getWidthOnly);
	else
		getWidthOnly = false;


	JSString *jsstr = JS_ValueToString(cx, J_FARG(1));
	J_S_ASSERT( jsstr != NULL, "Invalid string." );
	jschar *str = JS_GetStringChars(jsstr);
	J_S_ASSERT( str != NULL, "Invalid string." );
	size_t strlen = JS_GetStringLength(jsstr);

	if ( strlen == 0 )
		return JS_TRUE;


	jsval tmp;

	J_CHECK_CALL( JS_GetReservedSlot(cx, J_FOBJ, FONT_SLOT_LETTERSPACING, &tmp) );
	int letterSpacing = JSVAL_IS_INT(tmp) ? JSVAL_TO_INT(tmp) : 0;

	J_CHECK_CALL( JS_GetReservedSlot(cx, J_FOBJ, FONT_SLOT_BORDERWIDTH, &tmp) );
	int borderWidth = JSVAL_IS_INT(tmp) ? JSVAL_TO_INT(tmp) : 0;

	J_CHECK_CALL( JS_GetReservedSlot(cx, J_FOBJ, FONT_SLOT_USEKERNING, &tmp) );
	int useKerning = JSVAL_IS_BOOLEAN(tmp) ? JSVAL_TO_BOOLEAN(tmp) == JS_TRUE : false;

	J_S_ASSERT( face->size->metrics.height > 0, "Invalid font size." );

	typedef struct {
		FT_Vector pos; // glyph origin on the baseline
		FT_Glyph image; // glyph image
	} Glyph;

	Glyph *glyphs = (Glyph*)JS_malloc(cx, sizeof(Glyph) * strlen);

	size_t i;

	FT_Pos posX = 0;
	FT_Pos posY = 0;
	FT_UInt prevGlyphIndex = 0;
/*
	FT_Vector start;
	FT_Matrix matrix;
	float angle = 1;
	start.x = 0;
	start.y = 0;
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10L );
*/

	for ( i=0; i<strlen; i++ ) {

		FT_UInt glyphIndex = FT_Get_Char_Index( face, str[i] );
		FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		FT_Get_Glyph(face->glyph, &glyphs[i].image);

//		FT_Glyph_Transform( glyphs[i].image, &matrix, NULL );

		glyphs[i].pos.x = posX;
		glyphs[i].pos.y = posY + face->size->metrics.ascender;

		// prepare next char
		if ( useKerning && prevGlyphIndex && glyphIndex ) {
			
			FT_Vector delta;
			FT_Get_Kerning( face, prevGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta );
			posX += delta.x;
		}
		int adv = face->glyph->advance.x + (letterSpacing << 6);
		posX += adv >= 0 ? adv : 0;
		prevGlyphIndex = glyphIndex;
	}
	posX -= letterSpacing << 6; // we do not need the letterSpacing at the end of the text
	posY += face->size->metrics.height;
   // here, text extents from (0,0) to (posX,posY)

	int width = (posX >> 6) + borderWidth * 2; // * 2 because left and right border
	int height = (posY >> 6) + borderWidth * 2; // ...

	if ( getWidthOnly ) {

		*J_FRVAL = INT_TO_JSVAL( width );
	} else {

	// allocates the resulting image buffer
		size_t bufLength = width * height * 1; // 1 channel

		char *buf = (char*)JS_malloc(cx, bufLength); // JS_malloc do not supports 0 bytes size

		JSObject *bstr = NewBString(cx, buf, bufLength);
		*J_FRVAL = OBJECT_TO_JSVAL( bstr );
		J_S_ASSERT( bstr != NULL, "Unable to create a BString." ); // (TBD) free buf
		JS_DefineProperty(cx, bstr, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
		JS_DefineProperty(cx, bstr, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
		JS_DefineProperty(cx, bstr, "channels", INT_TO_JSVAL(1), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	// render glyphs in the bitmap
		memset(buf, 0, bufLength);
		for ( i=0; i<strlen; i++ ) {
		
			if ( glyphs[i].image->format != FT_GLYPH_FORMAT_BITMAP )
				FT_Glyph_To_Bitmap( &(glyphs[i].image), FT_RENDER_MODE_NORMAL, 0, 1 );
				
			FT_BitmapGlyph bitmap = (FT_BitmapGlyph)glyphs[i].image;

			int dPosX = borderWidth + (glyphs[i].pos.x >> 6) + bitmap->left;
			int dPosY = borderWidth + (glyphs[i].pos.y >> 6) - bitmap->top; // bitmap->top is the vertical distance from the pen position (on the baseline) to the topmost border of the glyph bitmap.

			int x, y;
			for ( y=0; y<bitmap->bitmap.rows; y++ )
				for ( x=0; x<bitmap->bitmap.width; x++ )
					buf[ (x+dPosX) + (y+dPosY) * width ] |= (char)bitmap->bitmap.buffer[ x + y * bitmap->bitmap.width ];

			FT_Done_Glyph( glyphs[i].image );
		}
	}

	JS_free(cx, glyphs);
	return JS_TRUE;
}


DEFINE_PROPERTY( encoding ) {

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);
	int encoding;
	J_JSVAL_TO_INT32( *vp, encoding );
	FT_Select_Charmap(face, (FT_Encoding)encoding);
	return JS_TRUE;
}


DEFINE_PROPERTY( ascender ) {

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);
	*vp = INT_TO_JSVAL(face->size->metrics.ascender >> 6);
	return JS_TRUE;
}
	
DEFINE_PROPERTY( descender ) {

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);
	*vp = INT_TO_JSVAL(face->size->metrics.descender >> 6);
	return JS_TRUE;
}

DEFINE_PROPERTY( width ) {

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_FOBJ);
	J_S_ASSERT_RESOURCE(face);
	*vp = INT_TO_JSVAL(face->size->metrics.max_advance >> 6);
	return JS_TRUE;
}


DEFINE_PROPERTY( size ) {

	FT_Face face = (FT_Face)JS_GetPrivate(cx, J_OBJ);
	J_S_ASSERT_RESOURCE(face);

	int size;
	if ( *vp == JSVAL_VOID ) {

		size = 0;
	} else {

		J_JSVAL_TO_INT32( *vp, size );
		J_S_ASSERT( size >= 0, "Invalid font size." );
	}

	FT_Error status;
	status = FT_Set_Pixel_Sizes(face, 0, size);
	J_S_ASSERT( status == 0, "Unable to FT_Set_Pixel_Sizes." );
	return JS_TRUE;
}




DEFINE_PROPERTY_GETTER( useKerning ) {

	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, FONT_SLOT_USEKERNING, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( useKerning ) {

	bool boolVal;
	if ( *vp == JSVAL_VOID ) {

		boolVal = false;
	} else {

		J_JSVAL_TO_BOOL( *vp, boolVal );
	}
	J_CHECK_CALL( JS_SetReservedSlot(cx, obj, FONT_SLOT_USEKERNING, BOOLEAN_TO_JSVAL(boolVal)) );
	return JS_TRUE;
}


DEFINE_PROPERTY_GETTER( borderWidth ) {

	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, FONT_SLOT_BORDERWIDTH, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( borderWidth ) {

	int intVal;
	if ( *vp == JSVAL_VOID ) {

		intVal = 0;
	} else {

		J_JSVAL_TO_INT32( *vp, intVal );
		J_S_ASSERT( intVal >= 0, "Invalid border size." );
	}
	J_CHECK_CALL( JS_SetReservedSlot(cx, obj, FONT_SLOT_BORDERWIDTH, INT_TO_JSVAL(intVal)) );
	return JS_TRUE;
}


DEFINE_PROPERTY_GETTER( letterSpacing ) {

	J_CHECK_CALL( JS_GetReservedSlot(cx, obj, FONT_SLOT_LETTERSPACING, vp) );
	return JS_TRUE;
}

DEFINE_PROPERTY_SETTER( letterSpacing ) {

	int intVal;
	if ( *vp == JSVAL_VOID )
		intVal = 0;
	else
		J_JSVAL_TO_INT32( *vp, intVal );
	J_CHECK_CALL( JS_SetReservedSlot(cx, obj, FONT_SLOT_LETTERSPACING, INT_TO_JSVAL(intVal)) );
	return JS_TRUE;
}


CONFIGURE_CLASS // This section containt the declaration and the configuration of the class

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(4)

	BEGIN_FUNCTION_SPEC
		FUNCTION_FAST(Draw)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_WRITE_STORE(size)
		PROPERTY(useKerning)
		PROPERTY(borderWidth)
		PROPERTY(letterSpacing)
		PROPERTY_WRITE_STORE(encoding)
		PROPERTY_READ(ascender)
		PROPERTY_READ(descender)
		PROPERTY_READ(width)
	END_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC

		CONST_INTEGER( NONE				,FT_ENCODING_NONE				)
		CONST_INTEGER( MS_SYMBOL		,FT_ENCODING_MS_SYMBOL		)
		CONST_INTEGER( UNICODE			,FT_ENCODING_UNICODE			)
		CONST_INTEGER( SJIS				,FT_ENCODING_SJIS				)
		CONST_INTEGER( GB2312			,FT_ENCODING_GB2312			)
		CONST_INTEGER( BIG5				,FT_ENCODING_BIG5				)
		CONST_INTEGER( WANSUNG			,FT_ENCODING_WANSUNG			)
		CONST_INTEGER( JOHAB				,FT_ENCODING_JOHAB			)
		CONST_INTEGER( MS_SJIS			,FT_ENCODING_MS_SJIS			)
		CONST_INTEGER( MS_GB2312		,FT_ENCODING_MS_GB2312		)
		CONST_INTEGER( MS_BIG5			,FT_ENCODING_MS_BIG5			)
		CONST_INTEGER( MS_WANSUNG		,FT_ENCODING_MS_WANSUNG		)
		CONST_INTEGER( MS_JOHAB			,FT_ENCODING_MS_JOHAB		)
		CONST_INTEGER( ADOBE_STANDARD	,FT_ENCODING_ADOBE_STANDARD)
		CONST_INTEGER( ADOBE_EXPERT	,FT_ENCODING_ADOBE_EXPERT	)
		CONST_INTEGER( ADOBE_CUSTOM	,FT_ENCODING_ADOBE_CUSTOM	)
		CONST_INTEGER( ADOBE_LATIN_1	,FT_ENCODING_ADOBE_LATIN_1	)
		CONST_INTEGER( OLD_LATIN_2		,FT_ENCODING_OLD_LATIN_2	)
		CONST_INTEGER( APPLE_ROMAN		,FT_ENCODING_APPLE_ROMAN	)
	END_CONST_INTEGER_SPEC

END_CLASS


/*
	API: http://www.freetype.org/freetype2/docs/reference/ft2-base_interface.html#FT_Done_FreeType
	Glyph API: http://www.freetype.org/freetype2/docs/reference/ft2-glyph_management.html
	Tutorial: http://www.freetype.org/freetype2/docs/tutorial/step1.html

	on-line demo: http://mbox.troja.mff.cuni.cz/~peak/ftdemo/index.cgi
	-> code: http://www.koders.com/c/fid49219DA46C44B7DFDA3807DC8B0CADCEE972CEB7.aspx

*/
