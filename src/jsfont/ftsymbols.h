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


struct FTSymbols {
	FT_Error (*Init_FreeType)( FT_Library *alibrary );
	FT_Error (*Done_FreeType)( FT_Library *alibrary );
	FT_Error (*Select_Charmap)( FT_Face face, FT_Encoding encoding );
	FT_Error (*New_Face)( FT_Library library, const char* filepathname, FT_Long face_index, FT_Face *aface );
	FT_Error (*Done_Face)( FT_Face face );
	void (*Set_Transform)( FT_Face face, FT_Matrix* matrix, FT_Vector* delta );
	void (*Vector_Unit)( FT_Vector* vec, FT_Angle angle );
	FT_Error (*Load_Glyph)( FT_Face face, FT_UInt glyph_index, FT_Int32 load_flags );
	FT_UInt (*Get_Char_Index)( FT_Face face, FT_ULong charcode );
	void (*Done_Glyph)( FT_Glyph glyph );
	void (*FT_Glyph_Get_CBox)( FT_Glyph glyph, FT_UInt bbox_mode, FT_BBox *acbox );
	FT_Error (*Get_Glyph)( FT_GlyphSlot slot, FT_Glyph *aglyph );
	FT_Error (*Set_Char_Size)( FT_Face face, FT_F26Dot6 char_width, FT_F26Dot6 char_height, FT_UInt horz_resolution, FT_UInt vert_resolution );
	FT_Error (*Glyph_To_Bitmap)( FT_Glyph* the_glyph, FT_Render_Mode render_mode, FT_Vector* origin, FT_Bool destroy );
	FT_Error (*Glyph_Transform)( FT_Glyph glyph, FT_Matrix* matrix, FT_Vector* delta );
	void (*Vector_Rotate)( FT_Vector* vec, FT_Angle angle );
	FT_Error (*Glyph_Copy)( FT_Glyph source, FT_Glyph *target );
	FT_Error (*Outline_Decompose)( FT_Outline* outline, const FT_Outline_Funcs* func_interface, void* user );
	FT_Error (*Render_Glyph)( FT_GlyphSlot slot, FT_Render_Mode render_mode );
};

typedef void (*GetFTSymbols_t)(FTSymbols *sym);

void GetFTSymbols(FTSymbols *sym) {

//	sym->Init_FreeType = FT_Init_FreeType;
}

#define FT_DEFINE_SYMBOLS \
FTSymbols _ftSymbols; \
FT_EXPORT( FT_Error ) FT_Init_FreeType( FT_Library *alibrary ) { return _ftSymbols.Init_FreeType(alibrary); } \
FT_EXPORT( FT_Error ) FT_Done_FreeType( FT_Library *alibrary ) { return _ftSymbols.Done_FreeType(alibrary); } \
FT_EXPORT( FT_Error ) FT_Select_Charmap( FT_Face face, FT_Encoding encoding ) { return _ftSymbols.Select_Charmap(face, encoding); }

