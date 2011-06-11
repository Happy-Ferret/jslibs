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

#include <zip.h>
#include <unzip.h>

#include <jsdate.h>

#define PASSWORDREQUIRED -1000

#define SLOT_Z_ZIPFILE__FILENAME 0
#define SLOT_Z_ZIPFILE__GLOBALCOMMENT 1
#define SLOT_Z_ZIPFILE__INZIPFILENAME 2
#define SLOT_Z_ZIPFILE__CURRENTDATE 3
#define SLOT_Z_ZIPFILE__CURRENTPASSWORD 4


NEVER_INLINE JSBool FASTCALL
ThrowZipFileError( JSContext *cx, int errorCode );


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision: 3471 $
**/

BEGIN_CLASS( ZipFile )

struct Private {

	bool inZipOpened;
	zipFile zf;
	unzFile uf;

	int level; // compression level
	bool eol; // end of list
};



JSBool PrepareReadCurrentFile( JSContext *cx, JSObject *obj, uLong *fileSize ) {

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	int status;

	unz_file_info pfile_info;
	status = unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0);
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);
	bool needPassword = (pfile_info.flag & 1) == 1;
	*fileSize = pfile_info.uncompressed_size;

	if ( !pv->inZipOpened ) {

		JLStr password;

		if ( needPassword ) {

			jsval tmp;
			JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__CURRENTPASSWORD, &tmp) );
			if ( !JSVAL_IS_VOID(tmp) )
				JL_CHK( JL_JsvalToNative(cx, tmp, &password) );
			if ( !password.IsSet() )
				return ThrowZipFileError(cx, PASSWORDREQUIRED);
			status = unzOpenCurrentFilePassword(pv->uf, password);
		} else {

			status = unzOpenCurrentFile(pv->uf);
		}

		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

		pv->inZipOpened = true;
	}
	return JS_TRUE;
	JL_BAD;
}



JSBool NativeInterfaceStreamRead( JSContext *cx, JSObject *obj, char *buf, size_t *amount ) {

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);

	uLong fileSize;
	JL_CHK( PrepareReadCurrentFile(cx, obj, &fileSize) );

	int rd;
	rd = unzReadCurrentFile(pv->uf, buf, *amount);
	if ( rd < 0 ) {

		if ( rd == UNZ_ERRNO )
			JL_ERR( E_FILE, E_ACCESS );
		else
			JL_ERR( E_DATA, E_INVALID );
	}
	
	ASSERT( (uLong)rd <= *amount );

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


DEFINE_CONSTRUCTOR() {

	JL_ASSERT_CONSTRUCTING();
	JL_ASSERT_ARG_COUNT(1);

	JL_DEFINE_CONSTRUCTOR_OBJ;
	
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__FILENAME, JL_ARG(1)) );

	Private *pv = (Private *)jl_calloc(sizeof(Private), 1);
	JL_ASSERT_ALLOC(pv);
	JL_ASSERT( pv->uf == NULL && pv->zf == NULL );
	JL_ASSERT( !pv->inZipOpened );
	JL_SetPrivate(cx, obj, pv);

	//JL_CHK( ReserveStreamReadInterface(cx, obj) );
	JL_CHK( SetStreamReadInterface(cx, obj, NativeInterfaceStreamRead) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Open ) {

	int mode;
	JLStr filename;

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE()

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC_RANGE(1, 2);

	JL_CHK( JL_ReservedSlotToNative(cx, obj, SLOT_Z_ZIPFILE__FILENAME, &filename) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &mode) );

	if ( mode < 0 ) {

		JL_ASSERT_ARGC_MAX(1);

		pv->uf = unzOpen(filename);
		if ( !pv->uf ) {

			JL_ERR( E_FILE, E_ACCESS, E_DETAILS, E_NAME(filename) );
		}
	} else {

		pv->zf = zipOpen(filename, mode);
		if ( !pv->zf ) {

			JL_ERR( E_FILE, E_ACCESS, E_DETAILS, E_NAME(filename) );
		}

		if ( JL_ARG_ISDEF(2) ) {

			JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pv->level) );
			JL_ASSERT_ARG_VAL_RANGE( pv->level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION, 2 );
		} else {

			pv->level = Z_DEFAULT_COMPRESSION;
		}
	}

	ASSERT( pv && !pv->uf == !!pv->zf );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE()

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARG_COUNT(0);

	int status;
	if ( pv->uf ) {

		ASSERT( !pv->zf );
		status = unzClose(pv->uf);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);
	} else
	if ( pv->zf ) {

		ASSERT( !pv->uf );

		JLStr comment;
		jsval tmp;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__GLOBALCOMMENT, &tmp) );
		if ( !JSVAL_IS_VOID(tmp) )
			JL_CHK( JL_JsvalToNative(cx, tmp, &comment) );
		status = zipClose(pv->zf, comment.GetConstStrZOrNULL());
		if ( status != ZIP_OK )
			return ThrowZipFileError(cx, status);
	}

	jl_free(pv);
	JL_SetPrivate(cx, obj, NULL);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Select ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARG_COUNT(1);

	ASSERT( pv && !pv->uf == !!pv->zf );

	*JL_RVAL = JSVAL_VOID;

	int status;
	if ( pv->uf ) {

		JLStr inZipFilename;

		if ( pv->inZipOpened ) {

			status = unzCloseCurrentFile(pv->uf);
			if ( status != UNZ_OK )
				return ThrowZipFileError(cx, status);
			pv->inZipOpened = false;
		}

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &inZipFilename) );
		status = unzLocateFile(pv->uf, inZipFilename, 1);
		if ( status == UNZ_END_OF_LIST_OF_FILE ) {

			pv->eol = true;
			return JS_TRUE;
		}
		
		if ( status != UNZ_OK ) {
			
			return ThrowZipFileError(cx, status);
		}

		pv->eol = false;
	} else
	if ( pv->zf ) {

		if ( pv->inZipOpened ) {
			
			status = zipCloseFileInZip(pv->zf);
			if ( status != ZIP_OK )
				return ThrowZipFileError(cx, status);
			pv->inZipOpened = false;
		}

		JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__INZIPFILENAME, JL_ARG(1)) );
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( GoFirst ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE()

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARG_COUNT(0);
	JL_ASSERT(pv->uf, E_FILE, E_READ);

	int status;
	status = unzGoToFirstFile(pv->uf);
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);

	pv->eol = false;

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( GoNext ) {

	JL_DEFINE_FUNCTION_OBJ
	JL_ASSERT_THIS_INSTANCE()

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARG_COUNT(0);
	JL_ASSERT( pv->uf, E_FILE, E_READ );
	ASSERT( !pv->uf == !!pv->zf );

	*JL_RVAL = JSVAL_VOID;

	int status;
	status = unzGoToNextFile(pv->uf);

	if ( status == UNZ_END_OF_LIST_OF_FILE ) {

		pv->eol = true;
		return JS_TRUE;
	} else if ( status != UNZ_OK ) {

		return ThrowZipFileError(cx, status);
	}

	pv->eol = false;

	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE()
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE( pv );

	uLong fileSize;
	JL_CHK( PrepareReadCurrentFile(cx, obj, &fileSize) );

	uLong requestedLength;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_NUMBER(1);
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &requestedLength) );
	} else {

		requestedLength = fileSize;
	}
	
	uint8_t *buffer;
	buffer = (uint8_t *)jl_malloc(requestedLength +1);
	JL_ASSERT_ALLOC(buffer);
	int rd;
	rd = unzReadCurrentFile(pv->uf, buffer, requestedLength);
	if ( rd < 0 ) {

		if ( rd == UNZ_ERRNO )
			JL_ERR( E_FILE, E_ACCESS );
		else
			JL_ERR( E_DATA, E_INVALID );
	}
	
	ASSERT( (uLong)rd <= requestedLength );

	if ( JL_MaybeRealloc(requestedLength, rd) )
		buffer = (uint8_t*)jl_realloc(buffer, rd +1);

	buffer[rd] = '\0'; 
	JL_CHK( JL_NewBlob(cx, buffer, rd, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Write ) {

	JLStr data;

	JL_DEFINE_FUNCTION_OBJ;
	JL_ASSERT_THIS_INSTANCE()
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT_ARG_COUNT(1);

	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	int status;

	if ( !pv->inZipOpened ) {

		JLStr inZipFilename;

		zip_fileinfo zipfi;
		zipfi.dosDate = 0;
		zipfi.external_fa = 0;
		zipfi.internal_fa = 0;

		jsval tmp;
		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__CURRENTDATE, &tmp) );
		if ( JSVAL_IS_VOID(tmp) ) {

			memset(&zipfi.tmz_date, 0, sizeof(zipfi.tmz_date));
		} else {

			ASSERT( JSVAL_IS_NULL(tmp) || !JSVAL_IS_PRIMITIVE(tmp) && JS_ObjectIsDate(cx, JSVAL_TO_OBJECT(tmp)) );

			JSObject *dateObj;
			dateObj = JSVAL_TO_OBJECT(tmp);
			zipfi.tmz_date.tm_sec = js_DateGetSeconds(cx, dateObj);
			zipfi.tmz_date.tm_min = js_DateGetMinutes(cx, dateObj);
			zipfi.tmz_date.tm_hour = js_DateGetHours(cx, dateObj);
			zipfi.tmz_date.tm_mday = js_DateGetDate(cx, dateObj);
			zipfi.tmz_date.tm_mon = js_DateGetMonth(cx, dateObj) - 1;
			zipfi.tmz_date.tm_year = js_DateGetYear(cx, dateObj);
		}

		JL_CHK( JL_ReservedSlotToNative(cx, obj, SLOT_Z_ZIPFILE__INZIPFILENAME, &inZipFilename) );
		status = zipOpenNewFileInZip(pv->zf, inZipFilename, &zipfi, NULL, NULL, NULL, NULL, NULL, pv->level == 0 ? 0 : Z_DEFLATED, pv->level);
		if ( status != ZIP_OK )
			return ThrowZipFileError(cx, status);

		pv->inZipOpened = true;
	}

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &data) );
	ASSERT( data.IsSet() );

	status = zipWriteInFileInZip(pv->zf, data.GetConstStr(), data.Length());
	if ( status != ZIP_OK )
		return ThrowZipFileError(cx, status);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( eol ) {
	
	JL_INGORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_FILE, E_READ );

	return JL_NativeToJsval(cx, pv->eol, vp);
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( filename ) {

	JL_INGORE(id);

	JL_ASSERT_THIS_INSTANCE()
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	
	if ( pv->uf ) {

		char buffer[256];
		char *filename;

		int status;
		unz_file_info pfile_info;

		status = unzGetCurrentFileInfo(pv->uf, &pfile_info, buffer, sizeof(buffer), NULL, 0, NULL, 0);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

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

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__INZIPFILENAME, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( date ) {

	JL_INGORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		unz_file_info pfile_info;
		int status;
		status = unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

		const tm_unz *d = &pfile_info.tmu_date;
		JSObject *dateObj = JS_NewDateObject(cx, d->tm_year, d->tm_mon, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);
		JL_CHK( dateObj );
		*vp = OBJECT_TO_JSVAL( dateObj );
	} else
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__CURRENTDATE, vp) );
	}

	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( date ) {

	JL_INGORE(id);
	JL_INGORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_ASSERT( JSVAL_IS_NULL(*vp) || !JSVAL_IS_PRIMITIVE(*vp) && JS_ObjectIsDate(cx, JSVAL_TO_OBJECT(*vp)), E_VALUE, E_INVALID );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__CURRENTDATE, *vp) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_GETTER( comment ) {

	JL_INGORE(id);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );

	if ( pv->uf ) {

		int status;

		unz_global_info pglobal_info;
		status = unzGetGlobalInfo(pv->uf, &pglobal_info);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

		char *comment = (char *)jl_malloc(pglobal_info.size_comment +1);
		JL_ASSERT_ALLOC( comment );

		int rd;
		rd = unzGetGlobalComment(pv->uf, comment, pglobal_info.size_comment);
		if ( rd < 0 )
			JL_ERR( E_DATA, E_INVALID );
		comment[rd] = '\0';
		JL_CHK( JL_NewBlob(cx, comment, rd, JL_RVAL) );
	} else 
	if ( pv->zf ) {

		JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__GLOBALCOMMENT, vp) );
	}
	
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_SETTER( comment ) {

	JL_INGORE(id);
	JL_INGORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__GLOBALCOMMENT, *vp) );
	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( password ) {

	JL_INGORE(id);
	JL_INGORE(strict);

	JL_ASSERT_THIS_INSTANCE();
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	ASSERT( pv && !pv->uf == !!pv->zf );
	JL_ASSERT( pv->uf, E_THISOPERATION, E_INVALID );

	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_Z_ZIPFILE__CURRENTPASSWORD, *vp) );
	return JS_TRUE;
	JL_BAD;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3466 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(5)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Close )
		FUNCTION( Select )
		FUNCTION( Write )
		FUNCTION( Read )
		FUNCTION( GoFirst )
		FUNCTION( GoNext )
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_GETTER( filename )
		PROPERTY( date )
		PROPERTY( comment )
		PROPERTY_SETTER( password )
		PROPERTY_GETTER( eol )
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
	END_STATIC_PROPERTY_SPEC

	BEGIN_CONST_INTEGER_SPEC
		CONST_INTEGER( READ, -1 ) 
		CONST_INTEGER( CREATE, APPEND_STATUS_CREATE ) 
		CONST_INTEGER( CREATEAFTER, APPEND_STATUS_CREATEAFTER ) 
		CONST_INTEGER( ADDINZIP, APPEND_STATUS_ADDINZIP ) 
	END_CONST_INTEGER_SPEC

END_CLASS






BEGIN_CLASS( ZipFileError )

const char *ZipFileErrorConstString( int errorCode ) {

	switch (errorCode) {
		case ZIP_OK: return "OK";
		case ZIP_ERRNO: return "ERRNO";
		case ZIP_PARAMERROR: return "PARAMERROR";
		case ZIP_BADZIPFILE: return "BADZIPFILE";
		case ZIP_INTERNALERROR: return "INTERNALERROR";
		case UNZ_CRCERROR: return "CRCERROR";
		case PASSWORDREQUIRED: return "PASSWORDREQUIRED";
	}
	return "UNKNOWN_ERROR";
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_INGORE(id);

	JL_GetReservedSlot(cx, obj, 0, vp);
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ZipFileErrorConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( code ) {

	JL_INGORE(id);

	return JL_GetReservedSlot(cx, obj, 0, vp);
}

DEFINE_FUNCTION( toString ) {

	JL_INGORE(argc);

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

	JSObject *error = JS_NewObjectWithGivenProto(cx, JL_CLASS(ZipFileError), JL_PROTOTYPE(cx, ZipFileError), NULL);
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot(cx, error, 0, INT_TO_JSVAL(errorCode)) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}
