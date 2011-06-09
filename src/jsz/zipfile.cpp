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

#define SLOT_Z_ZIPFILE__FILENAME 0

#define MODE_READ 1
#define MODE_WRITE 2


BEGIN_CLASS( ZipFileError )

const char *ZipFileErrorConstString( int errorCode ) {

	switch (errorCode) {
		case ZIP_OK: return "OK";
		case ZIP_ERRNO: return "ERRNO";
		case ZIP_PARAMERROR: return "PARAMERROR";
		case ZIP_BADZIPFILE: return "BADZIPFILE";
		case ZIP_INTERNALERROR: return "INTERNALERROR";
		case UNZ_CRCERROR: return "CRCERROR";
	}
	return "UNKNOWN_ERROR";
}

DEFINE_PROPERTY_GETTER( const ) {

	JL_INGORE(id);

	JL_GetReservedSlot( cx, obj, 0, vp );
	if ( JSVAL_IS_VOID(*vp) )
		return JS_TRUE;
	int errorCode = JSVAL_TO_INT(*vp);
	JSString *str = JS_NewStringCopyZ( cx, ZipFileErrorConstString(errorCode) );
	*vp = STRING_TO_JSVAL( str );
	return JS_TRUE;
}

DEFINE_PROPERTY_GETTER( code ) {

	JL_INGORE(id);

	return JL_GetReservedSlot( cx, obj, 0, vp );
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

	JSObject *error = JS_NewObjectWithGivenProto( cx, JL_CLASS(ZipFileError), JL_PROTOTYPE(cx, ZipFileError), NULL );
	JS_SetPendingException( cx, OBJECT_TO_JSVAL( error ) );
	JL_CHK( JL_SetReservedSlot( cx, error, 0, INT_TO_JSVAL(errorCode) ) );
	JL_SAFE( JL_ExceptionSetScriptLocation(cx, error) );
	return JS_FALSE;
	JL_BAD;
}





BEGIN_CLASS( ZipFile )

struct Private {

	zipFile zf;
	unzFile uf;
	int level;
};


DEFINE_FINALIZE() {

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	if ( pv ) {

		if ( pv->zf ) {

			zipClose(pv->zf, NULL);
		} else if ( pv->uf ) {

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
	JL_SetPrivate(cx, obj, pv);

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Open ) {

	int mode;
	JLStr fileName;

	JL_DEFINE_FUNCTION_OBJ

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC_MIN(1);

	JL_CHK( JL_ReservedSlotToNative(cx, obj, SLOT_Z_ZIPFILE__FILENAME, &fileName) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &mode) );

	if ( mode < 0 ) {

		pv->uf = unzOpen(fileName);
		if ( pv->uf == NULL ) {

			JL_ERR( E_FILE, E_ACCESS, E_DETAILS, E_NAME(fileName) );
		}
	} else {

		JL_ASSERT_ARGC_MAX(2);

		pv->zf = zipOpen(fileName, mode);
		if ( pv->zf == NULL ) {

			JL_ERR( E_FILE, E_ACCESS, E_DETAILS, E_NAME(fileName) );
		}

		if ( JL_ARG_ISDEF(2) ) {

			JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &pv->level) );
			JL_ASSERT_ARG_VAL_RANGE( pv->level, Z_NO_COMPRESSION, Z_BEST_COMPRESSION, 2 );
		} else {

			pv->level = Z_DEFAULT_COMPRESSION;
		}
	}

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Close ) {

	JL_DEFINE_FUNCTION_OBJ

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARG_COUNT(0);

	int status;
	if ( pv->uf ) {

		status = unzClose(pv->uf);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);
	} else
	if ( pv->zf ) {

		status = zipClose(pv->zf, NULL);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);
	} else
		JL_ERR( E_OBJ, E_STATE );

	jl_free(pv);
	JL_SetPrivate(cx, obj, NULL);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Select ) {

	JLStr fileNameInZip;

	JL_DEFINE_FUNCTION_OBJ

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARG_COUNT(1);

	int status;
	if ( pv->uf ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileNameInZip) );
		status = unzLocateFile(pv->uf, fileNameInZip, 1);
		if ( status == UNZ_END_OF_LIST_OF_FILE ) {

			*JL_RVAL = JSVAL_FALSE;
			return JS_TRUE;
		} else if ( status != UNZ_OK ) {
			
			return ThrowZipFileError(cx, status);
		}

		status = unzOpenCurrentFile(pv->uf);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

	} else
	if ( pv->zf ) {

		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &fileNameInZip) );
		zipCloseFileInZip(pv->zf);
		status = zipOpenNewFileInZip(pv->zf, fileNameInZip, NULL, NULL, NULL, NULL, NULL, NULL, pv->level == 0 ? 0 : Z_DEFLATED, pv->level);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);

	} else
		JL_ERR( E_OBJ, E_STATE );

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Read ) {

	JL_DEFINE_FUNCTION_OBJ;

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARGC_RANGE(0, 1);
	JL_ASSERT( pv->uf, E_FILE, E_READ );

	int status;
	uLong length;
	if ( JL_ARG_ISDEF(1) ) {

		JL_ASSERT_ARG_IS_NUMBER(1);
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &length) );
	} else {

		unz_file_info pfile_info;
		status = unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0);
		if ( status != UNZ_OK )
			return ThrowZipFileError(cx, status);
		length = pfile_info.uncompressed_size;
	}
	
	uint8_t *buffer;
	buffer = (uint8_t *)jl_malloc(length +1);
	JL_ASSERT_ALLOC(buffer);
	status = unzReadCurrentFile(pv->uf, buffer, length);
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);
	buffer[length] = '\0'; 
	JL_CHK( JL_NewBlob(cx, buffer, length, JL_RVAL) );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( Write ) {

	JLStr data;

	JL_DEFINE_FUNCTION_OBJ;

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT_ARG_COUNT(1);

	JL_ASSERT( pv->zf, E_FILE, E_WRITE );

	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &data) );
	ASSERT( data.IsSet() );

	int status;
	status = zipWriteInFileInZip(pv->zf, data.GetConstStr(), data.Length());
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);

	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}



DEFINE_PROPERTY_GETTER( date ) {

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT( pv->uf, E_FILE, E_READ );

	unz_file_info pfile_info;
	int status;
	status = unzGetCurrentFileInfo(pv->uf, &pfile_info, NULL, 0, NULL, 0, NULL, 0);
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);

	const tm_unz *d = &pfile_info.tmu_date;
	JSObject *dateObj = JS_NewDateObject(cx, d->tm_year, d->tm_mon, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);
	JL_CHK( dateObj );

	*vp = OBJECT_TO_JSVAL( dateObj );

	return JS_TRUE;
	JL_BAD;
}


DEFINE_PROPERTY_SETTER( date ) {

	JL_ERR( E_API, E_NOTIMPLEMENTED );
/*
	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT( pv->zf, E_FILE, E_WRITE );
	JL_ASSERT( !JSVAL_IS_PRIMITIVE(*vp) && JS_ObjectIsDate(cx, JSVAL_TO_OBJECT(*vp)) );

	JSObject *dateObj = JSVAL_TO_OBJECT(*vp);

	zip_fileinfo pfile_info;
	tm_zip *d = &pfile_info.tmz_date;

	d->tm_year = js_DateGetYear(cx, dateObj);
	d->tm_mon = js_DateGetMonth(cx, dateObj);
	d->tm_mday = js_DateGetDate(cx, dateObj);
	d->tm_hour = js_DateGetHours(cx, dateObj);
	d->tm_min = js_DateGetMinutes(cx, dateObj);
	d->tm_sec = js_DateGetSeconds(cx, dateObj);
*/

	return JS_TRUE;
	JL_BAD;
}




DEFINE_FUNCTION( next ) {

	JL_DEFINE_FUNCTION_OBJ;

	jsval tmp;
	JL_CHK( JS_GetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
	if ( JSVAL_IS_VOID(tmp) ) {
	
		return JS_ThrowStopIteration(cx);
	}

	JL_ASSERT_OBJECT_STATE( !JSVAL_IS_PRIMITIVE(tmp), "ZipFile iterator" );
	
	JSObject *zipFileObj;
	zipFileObj = JSVAL_TO_OBJECT(tmp);
	JL_ASSERT_INHERITANCE(zipFileObj, JL_CLASS(ZipFile));

	Private *pv = (Private *)JL_GetPrivate(cx, zipFileObj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT( pv->uf, E_FILE, E_READ );

	char buffer[PATH_MAX];
	char *fileName;

	int status;
	unz_file_info pfile_info;

	status = unzGetCurrentFileInfo(pv->uf, &pfile_info, buffer, sizeof(buffer), NULL, 0, NULL, 0);
	if ( status != UNZ_OK )
		return ThrowZipFileError(cx, status);

	if ( pfile_info.size_filename <= sizeof(buffer) ) {

		fileName = buffer;
	} else {

		fileName = (char *)jl_malloca(pfile_info.size_filename);
		unzGetCurrentFileInfo(pv->uf, &pfile_info, fileName, pfile_info.size_filename, NULL, 0, NULL, 0);
	}

	JL_CHK( JL_NativeToJsval(cx, fileName, pfile_info.size_filename, JL_RVAL) );

	status = unzGoToNextFile(pv->uf);
	if ( status == UNZ_END_OF_LIST_OF_FILE ) {

		jsval tmp;
		tmp = JSVAL_VOID;
		JL_CHK( JS_SetPropertyById(cx, obj, INT_TO_JSID(0), &tmp) );
	} else if ( status != UNZ_OK ) {

		return ThrowZipFileError(cx, status);
	}

	return JS_TRUE;
	JL_BAD;
}



DEFINE_ITERATOR_OBJECT() {

	JL_CHKM( !keysonly, E_NAME("for...in"), E_NOTSUPPORTED );

	Private *pv = (Private *)JL_GetPrivate(cx, obj);
	JL_ASSERT_THIS_OBJECT_STATE(pv);
	JL_ASSERT( pv->uf, E_FILE, E_READ );

	JSObject *iterator = JS_NewObject(cx, NULL, NULL, NULL);
	JL_CHK( iterator );
	JL_CHK( JS_DefineFunction(cx, iterator, "next", _next, 0, 0) );

	jsval tmp;
	tmp = OBJECT_TO_JSVAL(obj);
	JL_CHK( JS_SetPropertyById(cx, iterator, INT_TO_JSID(0), &tmp) );

	int status;
	status = unzGoToFirstFile(pv->uf);
	if ( status != UNZ_OK ) {

		ThrowZipFileError(cx, status);
		goto bad;
	}
	return iterator;
bad:
	return NULL;
}



CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision: 3466 $"))
	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	HAS_ITERATOR_OBJECT

/*
	HAS_GET_PROPERTY
*/

	BEGIN_FUNCTION_SPEC
		FUNCTION( Open )
		FUNCTION( Close )
		FUNCTION( Select )
		FUNCTION( Write )
		FUNCTION( Read )

//		FUNCTION( next ) // iterator
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY( date )
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
