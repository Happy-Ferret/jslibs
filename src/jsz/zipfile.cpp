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


EXTERN_C void zipfile_free( void *ptr ) {

	jl_free(ptr);
}

EXTERN_C void *zipfile_malloc( size_t size ) {

	return jl_malloc(size);
}


#include <zip.h>
#include <unzip.h>

//#include <jsdate.h>

#define JLERR_PASSWORDREQUIRED -1000
#define JLERR_ENDOFLIST -1001


#define UNZ_CHK( apiStatus ) \
	JL_MACRO_BEGIN \
		const int st = (apiStatus); \
		if ( st != UNZ_OK ) { \
			return ThrowZipFileError(cx, st); \
		} else { } \
	JL_MACRO_END


#define ZIP_CHK( apiStatus ) \
	JL_MACRO_BEGIN \
		const int st = (apiStatus); \
		if ( st != ZIP_OK ) { \
			return ThrowZipFileError(cx, st); \
		} else { } \
	JL_MACRO_END



NEVER_INLINE JSBool FASTCALL
ThrowZipFileError( JSContext *cx, int errorCode );


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3471 $
**/

BEGIN_CLASS( ZipFile )

enum {
	SLOT_FILENAME,
	SLOT_INZIPFILENAME,
	SLOT_CURRENTDATE,
	SLOT_CURRENTEXTRAFIELD,
	SLOT_CURRENTLEVEL,
	SLOT_CURRENTPASSWORD,
	SLOT_GLOBALCOMMENT,
	SLOTCOUNT
};

struct Private {

	unzFile uf;
	zipFile zf;
	bool inZipOpened;
	bool eol; // end of list
	uLong remainingLength;
};



JSBool PrepareReadCurrentFile( JSContext *cx, JSObject *obj ) {

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->inZipOpened && pv->uf );

	unz_file_info pfile_info;
	UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0) );

	if ( pfile_info.flag & 1 ) { // has password

		JLStr password;
		jsval tmp;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTPASSWORD, &tmp) );
		if ( !JSVAL_IS_VOID(tmp) )
			JL_CHK( JL_JsvalToNative(cx, tmp, &password) );
		if ( !password.IsSet() )
			return ThrowZipFileError(cx, JLERR_PASSWORDREQUIRED);
		UNZ_CHK( unzOpenCurrentFilePassword(pv->uf, password) );
	} else {

		UNZ_CHK( unzOpenCurrentFile(pv->uf) );
	}

	pv->inZipOpened = true;
	pv->remainingLength = pfile_info.uncompressed_size;

	return JS_TRUE;
	JL_BAD;
}



JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	if ( pv->eol )
		UNZ_CHK( JLERR_ENDOFLIST );

	if ( !pv->inZipOpened )
		JL_CHK( PrepareReadCurrentFile(cx, obj) );

	int rd;
	rd = unzReadCurrentFile(pv->uf, buf, *amount);
	if ( rd < 0 )
		return ThrowZipFileError(cx, rd);
	
	ASSERT( (uLong)rd <= *amount );

	pv->remainingLength -= rd;

	*amount = rd;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FINALIZE() {

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	if ( pv ) {

		if ( pv->zf ) {

			ASSERT( !pv->uf );
			zipClose(pv->zf, NULL);
		} else if ( pv->uf ) {

			ASSERT( !pv->zf );
			unzClose(pv->uf);
		}
		jl_free(pv);
	}
}


/**doc
$TOC_MEMBER $INAME
 $INAME( filename )
  Constructs a new inflater or deflater object.
  $H example
  {{{
  var f = new ZipFile('test.zip');
  f.Open(ZipFile.CREATE);
  f.Select('file1.txt');
  f.Write('data1');
  f.Close();
  }}}
**/
DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_ASSERT_ARGC(1);

	JL_DEFINE_CONSTRUCTOR_OBJ;
	
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_FILENAME, JL_ARG(1)) );

	Private *pv = (Private *)jl_calloc(sizeof(Private), 1);
	JL_ASSERT_ALLOC(pv);
	JL_updateMallocCounter(cx, sizeof(Private));
	JL_ASSERT( !pv->uf && !pv->zf );
	JL_ASSERT( !pv->inZipOpened );
	JL_SetPrivate(cx, obj, pv);

	//JL_CHK( ReserveStreamReadInterface(cx, obj) );
	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( mode )
  Open a zip file for reading (ZipFile.READ) or writing (ZipFile.CREATE, ZipFile.CREATEAFTER, ZipFile.ADDINZIP).
   * if mode is CREATE, a new file is created.
   * if the file file exist and mode is CREATEAFTER, the zip will be created at the end of the file.
   * if the file file exist and mode is ADDINZIP, files are added in existing zip (be sure you don't add file that doesn't exist).
   If the zipfile cannot be opened, an error is rised.
**/
DEFINE_FUNCTION( open ) {

	int mode;
	JLStr filename;

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC(1);

	JL_CHK( JL_ReservedSlotToNative(cx, obj, SLOT_FILENAME, &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &mode) );

	if ( mode < 0 ) {

		pv->uf = unzOpen(filename);
		if ( !pv->uf ) {

			JL_ERR( E_FILE, E_ACCESS, E_SEP, E_NAME(filename), E_READ );
		}
	} else {

		pv->zf = zipOpen(filename, mode);
		if ( !pv->zf ) {

			JL_ERR( E_FILE, E_ACCESS, E_SEP, E_NAME(filename), E_WRITE );
		}
	}

	ASSERT( pv && !pv->uf == !!pv->zf );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close a zip file.
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC(0);

	if ( pv->uf ) {

		ASSERT( !pv->zf );
		UNZ_CHK( unzClose(pv->uf) );
	} else
	if ( pv->zf ) {

		ASSERT( !pv->uf );
		JLStr comment;
		jsval tmp;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_GLOBALCOMMENT, &tmp) );
		if ( !JSVAL_IS_VOID(tmp) )
			JL_CHK( JL_JsvalToNative(cx, tmp, &comment) );
		ZIP_CHK( zipClose(pv->zf, comment.GetConstStrZOrNULL()) );
	}

	jl_free(pv);
	JL_SetPrivate(cx, obj, NULL);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( filename )
  Select a file in the zip file.
  $H example 1
  {{{
  var f = new ZipFile('test.zip');
  f.Open(ZipFile.READ);
  f.Select('file1.txt');
  Print( f.Read() );
  f.Close();
  }}}
  $H example 2
  {{{
  var f = new ZipFile('test.zip');
  f.Open(ZipFile.CREATE);
  f.Select('file1.txt');
  f.Write('content1');
  f.Close();
  }}}
**/
DEFINE_FUNCTION( select ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC(1);

	*JL_RVAL = JSVAL_VOID;

	if ( pv->uf ) {

		JLStr inZipFilename;

		if ( pv->inZipOpened ) {

			UNZ_CHK( unzCloseCurrentFile(pv->uf) );
			pv->inZipOpened = false;
		}

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &inZipFilename) );
		int status;
		status = unzLocateFile(pv->uf, inZipFilename, 1); // iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
		if ( status == UNZ_END_OF_LIST_OF_FILE ) {

			pv->eol = true;
			return JS_TRUE;
		}
		UNZ_CHK( status );
		pv->eol = false;
	} else
	if ( pv->zf ) {

		if ( pv->inZipOpened ) {
			
			ZIP_CHK( zipCloseFileInZip(pv->zf) );
			pv->inZipOpened = false;
		}
		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_INZIPFILENAME, JL_ARG(1)) );
	}

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Go to the first file in the zip file.
  $H example
  {{{
  var f = new ZipFile('test.zip');
  f.Open(ZipFile.READ);
  for ( f.GoFirst(); !f.eol; f.GoNext() ) {

    Print( ' '+f.filename, ' : ', f.Read(), ' (lvl='+f.level+' eol=', f.eol, ')\n' );
  }
  }}}
**/
DEFINE_FUNCTION( goFirst ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC(0);
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	if ( pv->inZipOpened ) {

		UNZ_CHK( unzCloseCurrentFile(pv->uf) );
		pv->inZipOpened = false;
	}

	UNZ_CHK( unzGoToFirstFile(pv->uf) );
	pv->eol = false;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Go to the next file in the zip file.
**/
DEFINE_FUNCTION( goNext ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC(0);
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	*JL_RVAL = JSVAL_VOID;

	if ( pv->inZipOpened ) {

		UNZ_CHK( unzCloseCurrentFile(pv->uf) );
		pv->inZipOpened = false;
	}

	if ( pv->eol )
		UNZ_CHK( JLERR_ENDOFLIST );

	int status;
	status = unzGoToNextFile(pv->uf);
	if ( status == UNZ_END_OF_LIST_OF_FILE ) {

		pv->eol = true;
		return JS_TRUE;
	}
	UNZ_CHK( status );
	pv->eol = false;

	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( index )
  Go to index-th file in the zip file.
**/
DEFINE_FUNCTION( goTo ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC(1);
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	*JL_RVAL = JSVAL_VOID;

	uLong index;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &index) );

	if ( pv->inZipOpened ) {

		UNZ_CHK( unzCloseCurrentFile(pv->uf) );
		pv->inZipOpened = false;
	}

	int status;
	status = unzGoToFirstFile(pv->uf);
	for ( ; index; --index ) {

		status = unzGoToNextFile(pv->uf);
		if ( status == UNZ_END_OF_LIST_OF_FILE ) {

			pv->eol = true;
			return JS_TRUE;
		}
		UNZ_CHK( status );
	}
	pv->eol = false;

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( [amount] )
  Read _amount_ of data from the current file in the zip file.
  If _amount_ is omitted, the data is read from the current position to the end of the file.
  If the returned data is an empty string, this mean that the end of the file is reached.
  On error, a ZipFileError exception is rised.
**/
DEFINE_FUNCTION( read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC_RANGE(0, 1);

	//if ( pv->eol ) {

	//	*JL_RVAL = JSVAL_VOID;
	//	return JS_TRUE;
	//}
	if ( pv->eol )
		UNZ_CHK( JLERR_ENDOFLIST );

	if ( !pv->inZipOpened ) {

		JL_CHK( PrepareReadCurrentFile(cx, obj) );
	}

	uLong requestedLength;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_NUMBER(1);
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &requestedLength) );
		requestedLength = JL_MIN(requestedLength, pv->remainingLength);
	} else {

		requestedLength = pv->remainingLength;
	}
	
	uint8_t *buffer;
	buffer = (uint8_t *)jl_malloc(requestedLength +1);
	JL_ASSERT_ALLOC(buffer);
	int rd;
	rd = unzReadCurrentFile(pv->uf, buffer, requestedLength);
	if ( rd < 0 )
		return ThrowZipFileError(cx, rd);
	
	ASSERT( (uLong)rd <= requestedLength );

//	if ( JL_MaybeRealloc(requestedLength, rd) )
//		buffer = (uint8_t*)jl_realloc(buffer, rd +1);

	JL_updateMallocCounter(cx, rd);
	buffer[rd] = '\0'; 
	JL_CHK( JL_NewBlob(cx, buffer, rd, JL_RVAL) );

	pv->remainingLength -= rd;

	ASSERT( unzeof(pv->uf) == (pv->remainingLength == 0) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data )
  Append _data_ to the current file in the zip file.
**/
DEFINE_FUNCTION( write ) {

	JLStr data;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARGC(1);

	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	if ( !pv->inZipOpened ) {

		JLStr inZipFilename;
		JLStr currentExtraField;

		zip_fileinfo zipfi;
		zipfi.dosDate = 0;
		zipfi.external_fa = 0;
		zipfi.internal_fa = 0;

		jsval tmp;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTDATE, &tmp) );
		if ( JSVAL_IS_VOID(tmp) ) {

			//memset(&zipfi.tmz_date, 0, sizeof(zipfi.tmz_date));
			zipfi.tmz_date.tm_sec = 0;
			zipfi.tmz_date.tm_min = 0;
			zipfi.tmz_date.tm_hour = 0;
			zipfi.tmz_date.tm_mday = 0;
			zipfi.tmz_date.tm_mon = (uInt)-1;
			zipfi.tmz_date.tm_year = 0;
		} else {

			ASSERT( !JSVAL_IS_PRIMITIVE(tmp) && JS_ObjectIsDate(cx, JSVAL_TO_OBJECT(tmp)) );

			JSObject *dateObj;
			dateObj = JSVAL_TO_OBJECT(tmp);
			zipfi.tmz_date.tm_sec = js_DateGetSeconds(cx, dateObj);
			zipfi.tmz_date.tm_min = js_DateGetMinutes(cx, dateObj);
			zipfi.tmz_date.tm_hour = js_DateGetHours(cx, dateObj);
			zipfi.tmz_date.tm_mday = js_DateGetDate(cx, dateObj);
			zipfi.tmz_date.tm_mon = js_DateGetMonth(cx, dateObj) - 1;
			zipfi.tmz_date.tm_year = js_DateGetYear(cx, dateObj);
		}

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTEXTRAFIELD, &tmp) );
		if ( !JSVAL_IS_VOID(tmp) )
			JL_CHK( JL_JsvalToNative(cx, tmp, &currentExtraField) );

		JL_CHK( JL_ReservedSlotToNative(cx, obj, SLOT_INZIPFILENAME, &inZipFilename) );

		int level;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTLEVEL, &tmp) );
		if ( !JSVAL_IS_VOID(tmp) ) {

			JL_CHK( JL_JsvalToNative(cx, tmp, &level) );
			//JL_ASSERT_ARG_VAL_RANGE( level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION, 2 );
			level = JL_MINMAX(level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION);
		} else {

			level = Z_DEFAULT_COMPRESSION;
		}

		ZIP_CHK( zipOpenNewFileInZip(pv->zf, inZipFilename, &zipfi, currentExtraField.GetConstStrZOrNULL(), currentExtraField.LengthOrZero(), NULL, NULL, NULL, level == 0 ? 0 : Z_DEFLATED, level) );
		pv->inZipOpened = true;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &data) );
	ASSERT( data.IsSet() );

	ZIP_CHK( zipWriteInFileInZip(pv->zf, data.GetConstStr(), data.Length()) );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}




/**doc
=== Properties ===
**/


/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Get or set the global comment of the zip file.
**/
DEFINE_PROPERTY_GETTER( globalComment ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		unz_global_info pglobal_info;
		UNZ_CHK( unzGetGlobalInfo(pv->uf, &pglobal_info) );

		uLong commentLength = pglobal_info.size_comment;
		char *comment;
		comment = (char *)jl_malloc(commentLength +1);
		JL_ASSERT_ALLOC( comment );

		int rd;
		rd = unzGetGlobalComment(pv->uf, comment, commentLength);
		if ( rd < 0 )
			return ThrowZipFileError(cx, rd);

		JL_updateMallocCounter(cx, rd);
		comment[rd] = '\0';
		JL_CHK( JL_NewBlob(cx, comment, rd, JL_RVAL) );
	} else 
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_GLOBALCOMMENT, vp) );
	}
	
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( globalComment ) {

	JL_IGNORE(id);
	JL_IGNORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_GLOBALCOMMENT, *vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $BOOL $INAME
  Get the end-of-list status of zip file.
  If this property is true, the end of the file list stored in the zip file is reached.
  $H example
  {{{
  ...
  for ( g.GoFirst(); !g.eol; g.GoNext() ) { ...
  }}}
**/
DEFINE_PROPERTY_GETTER( eol ) {
	
	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_VALUE, E_READ );

	return JL_NativeToJsval(cx, pv->eol, vp);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Get the name of the current file in the zip file (see Select()).
**/
DEFINE_PROPERTY_GETTER( filename ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	
	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return JS_TRUE;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		char buffer[256];
		char *filename;

		unz_file_info pfile_info;

		UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, buffer, sizeof(buffer), NULL, 0, NULL, 0) );
		if ( pfile_info.size_filename <= sizeof(buffer) ) {

			filename = buffer;
		} else {
			
			// this case shoul be quite rare.
			filename = (char *)jl_malloca(pfile_info.size_filename);
			JL_ASSERT_ALLOC( filename );
			unzGetCurrentFileInfo(pv->uf, &pfile_info, filename, pfile_info.size_filename, NULL, 0, NULL, 0);
		}

		JSBool ok;
		ok = JL_NativeToJsval(cx, filename, pfile_info.size_filename, vp);
		
		if ( filename != buffer )
			jl_freea(filename);

		JL_CHK( ok );

	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_INZIPFILENAME, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the compression level of the current file in the zip file.
  If the zip file is open for writing, the compression level must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_GETTER( level ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return JS_TRUE;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		unz_file_info pfile_info;
		UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0) );

		int level = 0;
		if ( pfile_info.compression_method != 0 ) {

			switch ( pfile_info.flag & 0x6 ) {
				case 6: level = 1; break;
				case 4: level = 2; break;
				case 2: level = 9; break;
				case 0: level = 5; break;
			}
		}
		*JL_RVAL = INT_TO_JSVAL(level);
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTDATE, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( level ) {

	JL_IGNORE(id);
	JL_IGNORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_ASSERT( JSVAL_IS_VOID(*vp) || JSVAL_IS_NUMBER(*vp), E_TYPE, E_TY_NUMBER );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_CURRENTLEVEL, *vp) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Date $INAME
  Get or set the date of the current file in the zip file.
  If the zip file is open for writing, the date must be set before the first byte of the file is written.
  The date is represented by a JavaScript Date object.
  $H example
  {{{
  var f = new ZipFile('test.zip');
  f.Open(ZipFile.CREATE);
  f.Select('foo/bar/file1.txt');
  f.date = new Date(2008,6,29);
  f.Write('content1');
  f.Close();
  }}}
**/
DEFINE_PROPERTY_GETTER( date ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return JS_TRUE;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		unz_file_info pfile_info;
		UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0) );

		const tm_unz *d = &pfile_info.tmu_date;
		JSObject *dateObj = JS_NewDateObject(cx, d->tm_year, d->tm_mon, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);
		JL_CHK( dateObj );
		*vp = OBJECT_TO_JSVAL( dateObj );
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTDATE, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( date ) {

	JL_IGNORE(id);
	JL_IGNORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_ASSERT( JSVAL_IS_VOID(*vp) || !JSVAL_IS_PRIMITIVE(*vp) && JS_ObjectIsDate(cx, JSVAL_TO_OBJECT(*vp)), E_VALUE, E_INVALID );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_CURRENTDATE, *vp) );

	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Get or set extra data to the current file in the zip file.
  If the zip file is open for writing, the extra data must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_GETTER( extra ) {

	JL_IGNORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return JS_TRUE;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		int extraLength;
		extraLength = unzGetLocalExtrafield(pv->uf, NULL, 0);
		if ( extraLength < 0 )
			return ThrowZipFileError(cx, extraLength);

		char *buffer;
		buffer = (char *)jl_malloc(extraLength +1);
		JL_ASSERT_ALLOC( buffer );

		int rd;
		rd = unzGetLocalExtrafield(pv->uf, buffer, extraLength);
		if ( rd < 0 )
			return ThrowZipFileError(cx, rd);

		JL_updateMallocCounter(cx, rd);
		buffer[rd] = '\0';
		JL_CHK( JL_NewBlob(cx, buffer, rd, JL_RVAL) );

	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_CURRENTEXTRAFIELD, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( extra ) {

	JL_IGNORE(id);
	JL_IGNORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_CURRENTEXTRAFIELD, *vp) );
	return JS_TRUE;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Set the password required by the current file in the zip file.
  If the zip file is open for writing, the password must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_SETTER( password ) {

	JL_IGNORE(id);
	JL_IGNORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_INVALID );

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_CURRENTPASSWORD, *vp) );
	return JS_TRUE;
	JL_BAD;
}



#ifdef DEBUG

DEFINE_FUNCTION( zipfileTest ) {

	JL_IGNORE(vp);
	JL_IGNORE(cx);
	JL_IGNORE(argc);

	return JS_TRUE;
	JL_BAD;
}

#endif // DEBUG


/**doc
=== Constants ===
**/

/**doc
$TOC_MEMBER $INAME
 Open mode
 * READ
 * CREATE
 * CREATEAFTER
 * ADDINZIP
**/

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3466 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS( SLOTCOUNT )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( open )
		FUNCTION( close )
		FUNCTION( select )
		FUNCTION( write )
		FUNCTION( read )
		FUNCTION( goFirst )
		FUNCTION( goNext )
		FUNCTION( goTo )
#ifdef DEBUG
		FUNCTION( zipfileTest )
#endif // DEBUG
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( globalComment )
		PROPERTY_GETTER( eol )
		PROPERTY_GETTER( filename )
		PROPERTY( level )
		PROPERTY( date )
		PROPERTY( extra )
		PROPERTY_SETTER( password )
	END_PROPERTY_SPEC

	S_ASSERT( APPEND_STATUS_CREATE >= 0 && APPEND_STATUS_CREATEAFTER >= 0 && APPEND_STATUS_ADDINZIP >= 0 );

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( READ, -1 ) 
		CONST_INTEGER( CREATE, APPEND_STATUS_CREATE ) 
		CONST_INTEGER( CREATEAFTER, APPEND_STATUS_CREATEAFTER ) 
		CONST_INTEGER( ADDINZIP, APPEND_STATUS_ADDINZIP ) 
	END_CONST_INTEGER_SPEC

END_CLASS


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3471 $
**/
BEGIN_CLASS( ZipFileError )

const char *ZipFileErrorConstString( int errorCode ) {

	switch (errorCode) {
		case ZIP_OK: return "OK";
		case ZIP_ERRNO: return "ERRNO";
		case ZIP_PARAMERROR: return "PARAMERROR";
		case ZIP_BADZIPFILE: return "BADZIPFILE";
		case ZIP_INTERNALERROR: return "INTERNALERROR";
		case UNZ_CRCERROR: return "CRCERROR";
		case UNZ_END_OF_LIST_OF_FILE: return "END_OF_LIST_OF_FILE";
		case JLERR_PASSWORDREQUIRED: return "PASSWORDREQUIRED";
		case JLERR_ENDOFLIST: return "ENDOFLIST";
	}
	return "UNKNOWN_ERROR";
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_IGNORE(id);

	JL_GetReservedSlot(cx, obj, 0, vp);
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ZipFileErrorConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( code ) {

	JL_IGNORE(id);

	return JL_GetReservedSlot(cx, obj, 0, vp);
}

DEFINE_FUNCTION( toString ) {

	JL_IGNORE(argc);

	JL_DEFINE_FUNCTION_OBJ;
	return _constGetter(cx, obj, JSID_EMPTY, JL_RVAL);
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3466 $"))
	IS_INCONSTRUCTIBLE
	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
	END_PROPERTY_SPEC
	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
//		FUNCTION_ARGC(_serialize, 1)
//		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC
	HAS_RESERVED_SLOTS(1)
END_CLASS


NEVER_INLINE JSBool FASTCALL
ThrowZipFileError( JSContext *cx, int errorCode ) {

	ASSERT( errorCode <= 0 );
	JSObject *error = JS_NewObjectWithGivenProto(cx, JL_CLASS(ZipFileError), JL_PROTOTYPE(cx, ZipFileError), NULL);
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot(cx, error, 0, INT_TO_JSVAL(errorCode)) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}
