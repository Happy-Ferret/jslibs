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

#ifndef _JSFONT_H_
#define _JSFONT_H_

#include <jlhelper.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H


#define FTCHK( apiCall ) \
JL_MACRO_BEGIN \
	FT_Error apiError = (apiCall); \
	if ( apiError != FT_Err_Ok ) { \
		JL_REPORT_ERROR_NUM(cx, JLSMSG_LIB_ERROR, IntegerToString(apiError, 10)); \
	} \
JL_MACRO_END


struct FTSymbols {

	FT_Error (*Init_FreeType)( FT_Library *alibrary );
	FT_Error (*Done_FreeType)( FT_Library alibrary );
	FT_Error (*Select_Charmap)( FT_Face face, FT_Encoding encoding );
	FT_Error (*New_Face)( FT_Library library, const char* filepathname, FT_Long face_index, FT_Face *aface );
	FT_Error (*Done_Face)( FT_Face face );
	void (*Set_Transform)( FT_Face face, FT_Matrix* matrix, FT_Vector* delta );
	void (*Vector_Unit)( FT_Vector* vec, FT_Angle angle );
	FT_Error (*Load_Glyph)( FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags );
	FT_UInt (*Get_Char_Index)( FT_Face face, FT_ULong charcode );
	void (*Done_Glyph)( FT_Glyph glyph );
	void (*Glyph_Get_CBox)( FT_Glyph glyph, FT_UInt bbox_mode, FT_BBox *acbox );
	FT_Error (*Get_Glyph)( FT_GlyphSlot slot, FT_Glyph *aglyph );
	FT_Error (*Set_Char_Size)( FT_Face face, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution );
	FT_Error (*Glyph_To_Bitmap)( FT_Glyph* the_glyph, FT_Render_Mode render_mode, FT_Vector* origin, FT_Bool destroy );
	FT_Error (*Glyph_Transform)( FT_Glyph glyph, FT_Matrix* matrix, FT_Vector* delta );
	void (*Vector_Rotate)( FT_Vector* vec, FT_Angle angle );
	FT_Error (*Glyph_Copy)( FT_Glyph source, FT_Glyph *target );
	FT_Error (*Outline_Decompose)( FT_Outline* outline, const FT_Outline_Funcs* func_interface, void* user );
	FT_Error (*Render_Glyph)( FT_GlyphSlot slot, FT_Render_Mode render_mode );
	FT_Error (*New_Memory_Face)( FT_Library library, const FT_Byte* file_base, FT_Long file_size, FT_Long face_index, FT_Face *aface );
};


#define FT_DEFINE_SYMBOLS \
	FTSymbols _ftSymbols; \
	FT_EXPORT( FT_Error ) FT_Init_FreeType( FT_Library *alibrary ) { return _ftSymbols.Init_FreeType(alibrary); } \
	FT_EXPORT( FT_Error ) FT_Done_FreeType( FT_Library alibrary ) { return _ftSymbols.Done_FreeType(alibrary); } \
	FT_EXPORT( FT_Error ) FT_Select_Charmap( FT_Face face, FT_Encoding encoding ) { return _ftSymbols.Select_Charmap(face, encoding); } \
	FT_EXPORT( FT_Error ) FT_New_Face( FT_Library library, const char*  filepathname, FT_Long face_index, FT_Face *aface ) { return _ftSymbols.New_Face(library, filepathname, face_index, aface ); } \
	FT_EXPORT( FT_Error ) FT_Done_Face( FT_Face face ) { return _ftSymbols.Done_Face( face ); } \
	FT_EXPORT( void ) FT_Set_Transform( FT_Face face, FT_Matrix*  matrix, FT_Vector* delta ) { return _ftSymbols.Set_Transform( face, matrix, delta); } \
	FT_EXPORT( void ) FT_Vector_Unit( FT_Vector* vec, FT_Angle angle ) { return _ftSymbols.Vector_Unit( vec, angle ); } \
	FT_EXPORT( FT_Error ) FT_Load_Glyph( FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags ) { return _ftSymbols.Load_Glyph( face, glyph_index, load_flags ); } \
	FT_EXPORT( FT_UInt ) FT_Get_Char_Index( FT_Face face, FT_ULong charcode ) { return _ftSymbols.Get_Char_Index( face, charcode ); } \
	FT_EXPORT( void ) FT_Done_Glyph( FT_Glyph glyph ) { return _ftSymbols.Done_Glyph( glyph ); } \
	FT_EXPORT( void ) FT_Glyph_Get_CBox( FT_Glyph glyph, FT_UInt bbox_mode, FT_BBox *acbox ) { return _ftSymbols.Glyph_Get_CBox( glyph, bbox_mode, acbox ); } \
	FT_EXPORT( FT_Error ) FT_Get_Glyph( FT_GlyphSlot slot, FT_Glyph *aglyph ) { return _ftSymbols.Get_Glyph( slot, aglyph ); } \
	FT_EXPORT( FT_Error ) FT_Set_Char_Size( FT_Face face, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution ) { return _ftSymbols.Set_Char_Size( face, char_width, char_height, horz_resolution, vert_resolution ); } \
	FT_EXPORT( FT_Error ) FT_Glyph_To_Bitmap( FT_Glyph* the_glyph, FT_Render_Mode render_mode, FT_Vector* origin, FT_Bool destroy ) { return _ftSymbols.Glyph_To_Bitmap( the_glyph, render_mode, origin, destroy ); } \
	FT_EXPORT( FT_Error ) FT_Glyph_Transform( FT_Glyph glyph, FT_Matrix* matrix, FT_Vector* delta ) { return _ftSymbols.Glyph_Transform( glyph, matrix, delta ); } \
	FT_EXPORT( void ) FT_Vector_Rotate( FT_Vector* vec, FT_Angle angle ) { return _ftSymbols.Vector_Rotate( vec, angle ); } \
	FT_EXPORT( FT_Error ) FT_Glyph_Copy( FT_Glyph source, FT_Glyph *target ) { return _ftSymbols.Glyph_Copy( source, target ); } \
	FT_EXPORT( FT_Error ) FT_Outline_Decompose( FT_Outline* outline, const FT_Outline_Funcs* func_interface, void* user ) { return _ftSymbols.Outline_Decompose( outline, func_interface, user ); } \
	FT_EXPORT( FT_Error ) FT_Render_Glyph( FT_GlyphSlot slot, FT_Render_Mode render_mode ) { return _ftSymbols.Render_Glyph( slot, render_mode ); } \
	FT_EXPORT( FT_Error ) FT_New_Memory_Face( FT_Library library, const FT_Byte* file_base, FT_Long file_size, FT_Long face_index, FT_Face *aface ) { return _ftSymbols.New_Memory_Face( library, file_base, file_size, face_index, aface ); } \


inline void GetFTSymbols(FTSymbols *sym) {

	sym->Init_FreeType = FT_Init_FreeType;
	sym->Done_FreeType = FT_Done_FreeType;
	sym->Select_Charmap = FT_Select_Charmap;
	sym->New_Face = FT_New_Face;
	sym->Done_Face = FT_Done_Face;
	sym->Set_Transform = FT_Set_Transform;
	sym->Vector_Unit = FT_Vector_Unit;
	sym->Load_Glyph = FT_Load_Glyph;
	sym->Get_Char_Index = FT_Get_Char_Index;
	sym->Done_Glyph = FT_Done_Glyph;
	sym->Glyph_Get_CBox = FT_Glyph_Get_CBox;
	sym->Get_Glyph = FT_Get_Glyph;
	sym->Set_Char_Size = FT_Set_Char_Size;
	sym->Glyph_To_Bitmap = FT_Glyph_To_Bitmap;
	sym->Glyph_Transform = FT_Glyph_Transform;
	sym->Vector_Rotate = FT_Vector_Rotate;
	sym->Glyph_Copy = FT_Glyph_Copy;
	sym->Outline_Decompose = FT_Outline_Decompose;
	sym->Render_Glyph = FT_Render_Glyph;
	sym->New_Memory_Face = FT_New_Memory_Face;
}


// private structure of the jsfont module
struct JsfontModulePrivate {

	FT_Library ftLibrary;
	void (*GetFTSymbols)(FTSymbols *sym);
};


#define FONT_SLOT_HORIZONTALPADDING 0
#define FONT_SLOT_VERTICALPADDING 1
#define FONT_SLOT_USEKERNING 2
#define FONT_SLOT_SIZE 3
#define FONT_SLOT_LETTERSPACING 4
#define FONT_SLOT_ITALIC 5
#define FONT_SLOT_BOLD 6


// private structure of the Font class
struct JsfontPrivate {

	FT_Face face;

	//int horizontalPadding;
	//int verticalPadding;
	//int useKerning;
	//int size;
	//int letterSpacing;
	//bool italic;
	//bool bold;
};


ALWAYS_INLINE JsfontPrivate* GetJsfontPrivate(JSContext *cx, JSObject *fontObj) {
	
	JsfontPrivate *pv = (JsfontPrivate*)JL_GetPrivate(cx, fontObj);
	JL_ASSERT( pv );
	return pv;
}


#endif // _JSFONT_H_
