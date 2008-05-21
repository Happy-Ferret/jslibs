***** BEGIN LICENSE BLOCK *****
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

#include "static.h"

#include "../jslang/bstringapi.h"

#include <ft2build.h>
#include FT_FREETYPE_H


BEGIN_STATIC

DEFINE_FUNCTION_FAST( DrawChar ) {

	J_S_ASSERT_ARG_MIN(1);

	FT_ULong chr;
	if ( JSVAL_IS_INT(J_FARG(1)) ) {

		chr = JSVAL_TO_INT(J_FARG(1));
	} else
	if ( JSVAL_IS_STRING(J_FARG(1)) ) {

		JSString *str = JSVAL_TO_STRING( J_FARG(1) );

		J_S_ASSERT( JS_GetStringLength(str) >= 1, "Invalid char." );
		jschar *chars = JS_GetStringChars(str);
		chr = chars[0];
	} else
		J_REPORT_ERROR( "Invalid argument." );


	FT_Library freetype;

	FT_Error status;
	status = FT_Init_FreeType( &freetype );
	J_S_ASSERT( status == 0, "Unable to initialize FreeType library." );


	FT_Face face;
	status = FT_New_Face( freetype, "arial.ttf", 0, &face );
	// FT_New_Memory_Face
	J_S_ASSERT( status == 0, "Unable to create the face." );
	//	face->num_glyphs;

	status = FT_Set_Char_Size( face, 0, 16*64, 100, 0 );


	status = FT_Load_Char( face, chr, FT_LOAD_RENDER );

	int width = face->glyph->bitmap.width;
	int height = face->glyph->bitmap.rows;


	size_t bufLength = width * height * 1; // 1 channel

	void *buf = JS_malloc(cx, bufLength);

	JSObject *bstr = NewBString(cx, buf, bufLength);
	J_S_ASSERT( bstr != NULL, "Unable to create a BString." );
	*J_FRVAL = OBJECT_TO_JSVAL( bstr );

	JS_DefineProperty(cx, bstr, "width", INT_TO_JSVAL(width), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, bstr, "height", INT_TO_JSVAL(height), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );
	JS_DefineProperty(cx, bstr, "channels", INT_TO_JSVAL(1), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT );

	memcpy( buf, face->glyph->bitmap.buffer, bufLength );
	
/*
	for ( var y = 0; y < height; y++ ) {
		
		memcpy( (char*)buf + width * y, face->glyph->buffer
		(char*)buf[width
	}
*/



//	status = FT_Render_Glyph( face->glyph, FT_RENDER_MODE_NORMAL ); // FT_RENDER_MODE_NORMAL for 256 gray levels. FT_RENDER_MODE_MONO for monochrome
	// face->glyph->bitmap_left
	// face->glyph->bitmap_top
//	face->glyph->advance.x = 
//	FT_Err_Unknown_File_Format
	return JS_TRUE;
}

CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION_FAST( DrawChar )
	END_STATIC_FUNCTION_SPEC

END_STATIC
