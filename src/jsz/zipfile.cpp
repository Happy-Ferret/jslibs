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



NEVER_INLINE bool FASTCALL
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



bool PrepareReadCurrentFile( JSContext *cx, JS::HandleObject obj ) {

	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->inZipOpened && pv->uf );

	unz_file_info pfile_info;
	UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0) );

	if ( pfile_info.flag & 1 ) { // has password

		jl::BufString password;
		JS::RootedValue tmp(cx);

		JL_CHK( JL_GetReservedSlot( obj, SLOT_CURRENTPASSWORD, &tmp) );
		if ( !tmp.isUndefined() )
			JL_CHK( jl::getValue(cx, tmp, &password) );
		if ( !password )
			return ThrowZipFileError(cx, JLERR_PASSWORDREQUIRED);
		UNZ_CHK( unzOpenCurrentFilePassword(pv->uf, password) );
	} else {

		UNZ_CHK( unzOpenCurrentFile(pv->uf) );
	}

	pv->inZipOpened = true;
	pv->remainingLength = pfile_info.uncompressed_size;

	return true;
	JL_BAD;
}



bool NativeInterfaceStreamRead( JSContext *cx, JS::HandleObject obj, char *buf, size_t *amount ) {

	JL_ASSERT_INSTANCE(obj, JL_THIS_CLASS);

	Private *pv = (Private *)JL_GetPrivate(obj);
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

	// empty data mean EOF ?

	return true;
	JL_BAD;
}


DEFINE_FINALIZE() {

	JL_IGNORE( fop );

	Private *pv = (Private*)js::GetObjectPrivate(obj);
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
  f.open(ZipFile.CREATE);
  f.select('file1.txt');
  f.write('data1');
  f.close();
  }}}
**/
DEFINE_CONSTRUCTOR() {

	Private *pv = NULL;

	JL_DEFINE_ARGS;

	JL_ASSERT_CONSTRUCTING();
	JL_ASSERT_ARGC(1);
	JL_DEFINE_CONSTRUCTOR_OBJ;
	
	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_FILENAME, JL_ARG(1)) );

	pv = (Private*)jl_calloc(sizeof(Private), 1);
	JL_ASSERT_ALLOC(pv);
	pv->uf = NULL;
	pv->zf = NULL;

	JL_updateMallocCounter(cx, sizeof(Private));
	JL_ASSERT( !pv->uf && !pv->zf );
	JL_ASSERT( !pv->inZipOpened );

	//JL_CHK( ReserveStreamReadInterface(cx, obj) );
	JL_CHK( SetStreamReadInterface(cx, JL_OBJ, NativeInterfaceStreamRead) );

	JL_SetPrivate(JL_OBJ, pv);
	return true;

bad:
	if ( pv ) {

		if ( pv->zf )
			zipClose(pv->zf, NULL);
		if ( pv->uf )
			unzClose(pv->uf);
		jl_free(pv);
	}
	return false;
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
	jl::BufString filename;

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_CHK( jl::getSlot(cx, JL_OBJ, SLOT_FILENAME, &filename) );
	JL_CHK( jl::getValue(cx, JL_ARG(1), &mode) );

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

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Close a zip file.
**/
DEFINE_FUNCTION( close ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv = (Private*)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		ASSERT( !pv->zf );
		UNZ_CHK( unzClose(pv->uf) );
	} else
	if ( pv->zf ) {

		ASSERT( !pv->uf );
		jl::BufString comment;
		JS::RootedValue tmp(cx);
		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_GLOBALCOMMENT, &tmp) );
		if ( !tmp.isUndefined() )
			JL_CHK( jl::getValue(cx, tmp, &comment) );
		ZIP_CHK( zipClose(pv->zf, comment) );
	}

	// handle reusability
	//memset(pv, 0, sizeof(Private));

	jl_free(pv);
	JL_SetPrivate(JL_OBJ, NULL);

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( filename )
  Select a file in the zip file.
  $H example 1
  {{{
  var f = new ZipFile('test.zip');
  f.open(ZipFile.READ);
  f.select('file1.txt');
  print( f.read() );
  f.close();
  }}}
  $H example 2
  {{{
  var f = new ZipFile('test.zip');
  f.open(ZipFile.CREATE);
  f.select('file1.txt');
  f.write('content1');
  f.close();
  }}}
**/
DEFINE_FUNCTION( select ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	JL_RVAL.setUndefined();

	if ( pv->uf ) {

		jl::BufString inZipFilename;

		if ( pv->inZipOpened ) {

			UNZ_CHK( unzCloseCurrentFile(pv->uf) );
			pv->inZipOpened = false;
		}

		JL_CHK( jl::getValue(cx, JL_ARG(1), &inZipFilename) );
		int status;
		status = unzLocateFile(pv->uf, inZipFilename, 1); // iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
		if ( status == UNZ_END_OF_LIST_OF_FILE ) {

			pv->eol = true;
			return true;
		}
		UNZ_CHK( status );
		pv->eol = false;
	} else
	if ( pv->zf ) {

		if ( pv->inZipOpened ) {
			
			ZIP_CHK( zipCloseFileInZip(pv->zf) );
			pv->inZipOpened = false;
		}
		JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_INZIPFILENAME, JL_ARG(1)) );
	}

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Go to the first file in the zip file.
  $H example
  {{{
  var f = new ZipFile('test.zip');
  f.open(ZipFile.READ);
  for ( f.goFirst(); !f.eol; f.goNext() ) {

    print( ' '+f.filename, ' : ', f.read(), ' (lvl='+f.level+' eol=', f.eol, ')\n' );
  }
  }}}
**/
DEFINE_FUNCTION( goFirst ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	if ( pv->inZipOpened ) {

		UNZ_CHK( unzCloseCurrentFile(pv->uf) );
		pv->inZipOpened = false;
	}

	UNZ_CHK( unzGoToFirstFile(pv->uf) );
	pv->eol = false;

	JL_RVAL.setUndefined();
	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME()
  Go to the next file in the zip file.
**/
DEFINE_FUNCTION( goNext ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(0);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	JL_RVAL.setUndefined();

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
		return true;
	}
	UNZ_CHK( status );
	pv->eol = false;

	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( index )
  Go to index-th file in the zip file.
**/
DEFINE_FUNCTION( goTo ) {

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_NOTSUPPORTED );

	JL_RVAL.setUndefined();

	uLong index;
	JL_CHK( jl::getValue(cx, JL_ARG(1), &index) );

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
			return true;
		}
		UNZ_CHK( status );
	}
	pv->eol = false;

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $DATA | $VOID $INAME( [amount = $UNDEF] )
  Read _amount_ of data from the current file in the zip file.
  If _amount_ is omitted, the data is read from the current position to the end of the file.
  If the return value is $UNDEF this mean that the end of the file is reached.
  On error, a ZipFileError exception is rised.
**/
DEFINE_FUNCTION( read ) {

	jl::BufBase buffer;

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC_RANGE(0, 1);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE( pv );
	ASSERT( pv && !pv->uf == !!pv->zf );

	//if ( pv->eol ) {

	//	JL_RVAL.setUndefined();
	//	return true;
	//}
	if ( pv->eol )
		UNZ_CHK( JLERR_ENDOFLIST );

	if ( !pv->inZipOpened ) {

		JL_CHK( PrepareReadCurrentFile(cx, JL_OBJ) );
	}

	if ( pv->remainingLength == 0 ) {

		JL_RVAL.setUndefined();
		return true;
	}

	uLong requestedLength;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_NUMBER(1);
		JL_CHK( jl::getValue(cx, JL_ARG(1), &requestedLength) );
		requestedLength = jl::min(requestedLength, pv->remainingLength);
	} else {

		requestedLength = pv->remainingLength;
	}
	
	//buffer = JL_NewBuffer(cx, requestedLength, JL_RVAL);
	buffer.alloc(requestedLength);
	JL_ASSERT_ALLOC(buffer);

	int rd;
	rd = unzReadCurrentFile(pv->uf, buffer.data(), requestedLength);
	if ( rd < 0 )
		return ThrowZipFileError(cx, rd);
	
	ASSERT( (uLong)rd <= requestedLength );
	pv->remainingLength -= rd;
	ASSERT( unzeof(pv->uf) == (pv->remainingLength == 0) );

	buffer.setUsed(rd);
	JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );

	return true;
bad:
	return false;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( data )
  Append _data_ to the current file in the zip file.
**/
DEFINE_FUNCTION( write ) {

	jl::BufString data;

	JL_DEFINE_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	JL_ASSERT_ARGC(1);

	Private *pv = (Private *)JL_GetPrivate(JL_OBJ);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	if ( !pv->inZipOpened ) {

		jl::BufString inZipFilename, currentExtraField;

		zip_fileinfo zipfi;
		zipfi.dosDate = 0;
		zipfi.external_fa = 0;
		zipfi.internal_fa = 0;

		JS::RootedValue tmp(cx);
		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_CURRENTDATE, &tmp) );
		if ( tmp.isUndefined() ) {

			zipfi.tmz_date.tm_sec = 0;
			zipfi.tmz_date.tm_min = 0;
			zipfi.tmz_date.tm_hour = 0;
			zipfi.tmz_date.tm_mday = 0;
			zipfi.tmz_date.tm_mon = 0;
			zipfi.tmz_date.tm_year = 1980;
		} else {

			ASSERT( tmp.isObject() );
			JS::RootedObject dateObj(cx, &tmp.toObject());
			ASSERT( JS_ObjectIsDate(cx, dateObj) );
			zipfi.tmz_date.tm_sec = js_DateGetSeconds(dateObj);
			zipfi.tmz_date.tm_min = js_DateGetMinutes(cx, dateObj);
			zipfi.tmz_date.tm_hour = js_DateGetHours(cx, dateObj);
			zipfi.tmz_date.tm_mday = js_DateGetDate(cx, dateObj);
			zipfi.tmz_date.tm_mon = js_DateGetMonth(cx, dateObj);
			zipfi.tmz_date.tm_year = js_DateGetYear(cx, dateObj);
		}

		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_CURRENTEXTRAFIELD, &tmp) );
		if ( !tmp.isUndefined() )
			JL_CHK( jl::getValue(cx, tmp, &currentExtraField) );

		JL_CHK( jl::getSlot(cx, JL_OBJ, SLOT_INZIPFILENAME, &inZipFilename) );

		int level;
		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_CURRENTLEVEL, &tmp) );
		if ( !tmp.isUndefined() ) {

			JL_CHK( jl::getValue(cx, tmp, &level) );
			//JL_ASSERT_ARG_VAL_RANGE( level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION, 2 );
			level = jl::minmax(level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION);
		} else {

			level = Z_DEFAULT_COMPRESSION;
		}

		ZIP_CHK( zipOpenNewFileInZip(pv->zf, inZipFilename, &zipfi, currentExtraField.toStringZOrNull<const char*>(), currentExtraField.lengthOrZero(), NULL, NULL, NULL, level == 0 ? 0 : Z_DEFLATED, level) );
		pv->inZipOpened = true;
	}

	JL_CHK( jl::getValue(cx, JL_ARG(1), &data) );
	ASSERT( data );

	ZIP_CHK( zipWriteInFileInZip(pv->zf, data.toData<const uint8_t*>(), data.length()) );

	JL_RVAL.setUndefined();
	return true;
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

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {
		
		jl::BufBase buffer;

		unz_global_info pglobal_info;
		UNZ_CHK( unzGetGlobalInfo(pv->uf, &pglobal_info) );
		
		uLong commentLength = pglobal_info.size_comment;
		//uint8_t *comment;
		
		//comment = JL_NewBuffer(cx, commentLength, vp);
		buffer.alloc(commentLength, true);
		JL_ASSERT_ALLOC( buffer );

		int rd;
		rd = unzGetGlobalComment(pv->uf, (char*)buffer.data(), commentLength);
		if ( rd < 0 )
			return ThrowZipFileError(cx, rd);
		ASSERT( (uLong)rd == commentLength );
		JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot( obj, SLOT_GLOBALCOMMENT, vp) );
	}
	
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( globalComment ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_SetReservedSlot(obj, SLOT_GLOBALCOMMENT, JL_RVAL) );
	return true;
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
  for ( g.goFirst(); !g.eol; g.goNext() ) { ...
  }}}
**/
DEFINE_PROPERTY_GETTER( eol ) {
	
	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_VALUE, E_READ );

	return jl::setValue(cx, JL_RVAL, pv->eol);
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Get the name of the current file in the zip file (see Select()).
**/
DEFINE_PROPERTY_GETTER( filename ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	
	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return true;
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

		bool ok;
		ok = jl::setValue(cx, vp, jl::strSpec(filename, pfile_info.size_filename));
		
		if ( filename != buffer )
			jl_freea(filename);

		JL_CHK( ok );

	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot( obj, SLOT_INZIPFILENAME, vp) );
	}

	return true;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $INT $INAME
  Get or set the compression level of the current file in the zip file.
  If the zip file is open for writing, the compression level must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_GETTER( level ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return true;
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
		JL_RVAL.setInt32(level);
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot( obj, SLOT_CURRENTLEVEL, vp) );
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( level ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_ASSERT( vp.isUndefined() || vp.isNumber(), E_TYPE, E_TY_NUMBER );
	JL_CHK( JL_SetReservedSlot(obj, SLOT_CURRENTLEVEL, JL_RVAL) );

	return true;
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
  f.open(ZipFile.CREATE);
  f.select('foo/bar/file1.txt');
  f.date = new Date(2008,6,29);
  f.write('content1');
  f.close();
  }}}
**/
DEFINE_PROPERTY_GETTER( date ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return true;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		unz_file_info pfile_info;
		UNZ_CHK( unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0) );

		const tm_unz *d = &pfile_info.tmu_date;
		if ( d->tm_year == 1980 && d->tm_mon == 0 && d->tm_mday == 0 && d->tm_hour == 0 && d->tm_min == 0 && d->tm_sec == 0 ) {

			vp.setUndefined();
		} else {
		
			JS::RootedObject dateObj(cx, JS_NewDateObject(cx, d->tm_year, d->tm_mon, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec));
			JL_CHK( dateObj );
			JL_RVAL.setObject(*dateObj);
		}
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot( obj, SLOT_CURRENTDATE, vp) );
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( date ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_ASSERT( vp.isUndefined() || jl::isDate(cx, vp), E_VALUE, E_INVALID );
	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_CURRENTDATE, JL_RVAL) );

	return true;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Get or set extra data to the current file in the zip file.
  If the zip file is open for writing, the extra data must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_GETTER( extra ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		jl::BufBase buffer;

		//if ( pv->eol ) {

		//	*vp = JSVAL_VOID;
		//	return true;
		//}
		if ( pv->eol )
			UNZ_CHK( JLERR_ENDOFLIST );

		int extraLength;
		extraLength = unzGetLocalExtrafield(pv->uf, NULL, 0);
		if ( extraLength < 0 )
			return ThrowZipFileError(cx, extraLength);

		//uint8_t *buffer;
		//buffer = JL_NewBuffer(cx, extraLength, vp);
		buffer.alloc(extraLength, true);
		JL_ASSERT_ALLOC( buffer );

		int rd;
		rd = unzGetLocalExtrafield(pv->uf, buffer.data(), extraLength);
		if ( rd < 0 )
			return ThrowZipFileError(cx, rd);
		ASSERT( rd == extraLength );
		JL_CHK( BlobCreate(cx, buffer, JL_RVAL) );
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(JL_OBJ, SLOT_CURRENTEXTRAFIELD, JL_RVAL) );
	}

	return true;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( extra ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_CURRENTEXTRAFIELD, JL_RVAL) );
	return true;
	JL_BAD;
}



/**doc
$TOC_MEMBER $INAME
 $STRING $INAME
  Set the password required by the current file in the zip file.
  If the zip file is open for writing, the password must be set before the first byte of the file is written.
**/
DEFINE_PROPERTY_SETTER( password ) {

	JL_DEFINE_PROP_ARGS;

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_INVALID );

	JL_CHK( JL_SetReservedSlot(JL_OBJ, SLOT_CURRENTPASSWORD, JL_RVAL) );
	return true;
	JL_BAD;
}



#ifdef DEBUG

DEFINE_FUNCTION( zipfileTest ) {

	JL_IGNORE(vp, cx, argc);

	return true;
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

	REVISION(jl::SvnRevToInt("$Revision: 3466 $"))
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

	BEGIN_CONST
		CONST_INTEGER( READ, -1 ) 
		CONST_INTEGER( CREATE, APPEND_STATUS_CREATE ) 
		CONST_INTEGER( CREATEAFTER, APPEND_STATUS_CREATEAFTER ) 
		CONST_INTEGER( ADDINZIP, APPEND_STATUS_ADDINZIP ) 
	END_CONST

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

	JL_DEFINE_PROP_ARGS;

	JL_GetReservedSlot( obj, 0, vp);
	if ( vp.isUndefined() )
		return true;
	JSString *str = JS_NewStringCopyZ( cx, ZipFileErrorConstString(vp.toInt32()) );
	JL_ASSERT_ALLOC( str );
	JL_RVAL.setString(str);
	return true;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( code ) {

	JL_DEFINE_PROP_ARGS;

	return JL_GetReservedSlot( obj, 0, vp);
}

DEFINE_FUNCTION( toString ) {

	JL_DEFINE_ARGS;

	return jl::getProperty(cx, JL_OBJ, L("const"), JL_RVAL);
}

CONFIGURE_CLASS

	REVISION(jl::SvnRevToInt("$Revision: 3466 $"))
	HAS_RESERVED_SLOTS(1)

	IS_UNCONSTRUCTIBLE

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( code )
		PROPERTY_GETTER( const )
	END_PROPERTY_SPEC
	BEGIN_FUNCTION_SPEC
		FUNCTION(toString)
//		FUNCTION_ARGC(_serialize, 1)
//		FUNCTION_ARGC(_unserialize, 1)
	END_FUNCTION_SPEC

END_CLASS


NEVER_INLINE bool FASTCALL
ThrowZipFileError( JSContext *cx, int errorCode ) {

	ASSERT( errorCode <= 0 );
	JS::RootedObject error(cx, jl::newObjectWithGivenProto(cx, JL_CLASS(ZipFileError), JL_CLASS_PROTOTYPE(cx, ZipFileError)));
	JL_CHK( jl::setException(cx, error) );
	JL_CHK( jl::setSlot(cx, error, 0, errorCode) );
	JL_SAFE( jl::addScriptLocation(cx, &error) );
	return false;
	JL_BAD;
}
