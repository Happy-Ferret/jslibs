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

#include <stdlib.h>
#include <setjmp.h>

#define DLLAPI
#include <fcgiapp.h>


//static FCGX_Stream *_in, *_out, *_err;
//static FCGX_ParamArray _request.envp;
static FCGX_Request _request; // (TBD) fix static keyword issue

//static jmp_buf env;

static bool _initDone = false; // (TBD) fix static keyword issue

void onExit() {
	
	FCGX_ShutdownPending();
	FCGX_Free(&_request, 1);

//	longjmp(env,0); // if the value argument of longjmp is 0, setjmp returns 1.
}

/**doc fileIndex:topmost **/

/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
**/
BEGIN_STATIC


DEFINE_FUNCTION( Accept ) {

	if (!_initDone) {
	
		int status = FCGX_Init(); // (TBD) do it only once
		JL_S_ASSERT_ERROR_NUM( status == 0, JLSMSG_INIT_FAIL, "FCGX" );
		FCGX_InitRequest(&_request, 0, FCGI_FAIL_ACCEPT_ON_INTR); // doc: fail_on_intr is ignored in the Win lib.
		status = atexit(&onExit);
		JL_S_ASSERT( status == 0, "Unable to setup fcgi exit.");
		_initDone = true;
	}

//	if ( setjmp(env) == 0 ) {
//	}

	int rc;
	rc = FCGX_Accept_r(&_request);
	*JL_RVAL = INT_TO_JSVAL( rc );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( GetParam ) {

	if ( argc >= 1 ) {

		JLStr paramName;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &paramName) );
		char *paramValue = FCGX_GetParam(paramName, _request.envp);
		if ( paramValue != NULL ) {

			JSString *jsstr = JS_NewStringCopyZ(cx, paramValue);
			JL_CHK( jsstr );
			*JL_RVAL = STRING_TO_JSVAL( jsstr );
		} else
			*JL_RVAL = JSVAL_VOID;
	} else {
		
		// (TDB) use FCGX_ParamArray instead ?
		JSObject *argsObj = JS_NewObject(cx, NULL, NULL, NULL);
		JL_CHK(argsObj);
		for ( char** ptr = _request.envp; *ptr; ptr++ ) {

			char *separator = strchr( *ptr, '=' );
			JL_S_ASSERT( separator != NULL, "Unable to find the value." );
			*separator = '\0';
			JSString *value = JS_NewStringCopyZ(cx, separator + 1);
			JS_DefineProperty(cx, argsObj, *ptr, STRING_TO_JSVAL(value), NULL, NULL, JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT);
			*separator = '=';
		}
		*JL_RVAL = OBJECT_TO_JSVAL(argsObj);
	}
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Read ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	size_t len;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &len) );
	char* str;
	str = (char*)JS_malloc(cx, len + 1);
	int result;
	result = FCGX_GetStr( str, (int)len, _request.in );
	if ( result == 0 ) {
		
		JS_free(cx, str);
		*JL_RVAL = JL_GetEmptyStringValue(cx);
		return JS_TRUE;
	}
	str[result] = '\0';
	JSString *jsstr;
	//jsstr = JL_NewString(cx, str, result);
	jsstr = JLStr(str, result, true).GetJSString(cx);
	JL_CHK( jsstr );
	*JL_RVAL = STRING_TO_JSVAL( jsstr );
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Write ) {

	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	int result;
	result = FCGX_PutStr(str.GetConstStr(), (int)str.Length(), _request.out);
	if ( result >= 0 && (size_t)result < str.Length() ) { // returns unwritten data

		JSString *jsstr = JS_NewDependentString(cx, JSVAL_TO_STRING(JL_ARG(1)), result, str.Length() - result);
		JL_S_ASSERT( jsstr != NULL, "Unable to create the NewDependentString." );
		*JL_RVAL = STRING_TO_JSVAL( jsstr );
	} else
		*JL_RVAL = JL_GetEmptyStringValue(cx);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Flush ) {

	int result = FCGX_FFlush(_request.out);
	JL_S_ASSERT( result != -1, "Unable to flush the output stream." );
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( Log ) {

	JLStr str;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &str) );
	int result;
	result = FCGX_PutStr(str.GetConstStr(), (int)str.Length(), _request.err);
	JL_S_ASSERT( result != -1, "Unable to write to the log." );
	FCGX_FFlush(_request.err);
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( ShutdownPending ) {

	FCGX_ShutdownPending();
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
}

DEFINE_FUNCTION( URLEncode ) {

	JLStr srcStr;

	JL_S_ASSERT_ARG_MIN( 1 );
	static unsigned char hex[] = "0123456789ABCDEF";
	const char *src;
	size_t srcLen;

//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &src, &srcLen) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );
	srcLen = srcStr.Length();
	src = srcStr.GetConstStr();

	char *dest;
	dest = (char *)JS_malloc(cx, 3 * srcLen + 1);
	JL_CHK( dest );

	const char *it;
	char *it1;
	for ( it = src, it1 = dest; it < src + srcLen; it++ )
		if ( *it == ' ' )
			*(it1++) = '+';
		else
			if ( (*it < '0' && *it != '-' && *it != '.') || (*it < 'A' && *it > '9') || (*it > 'Z' && *it < 'a' && *it != '_') || (*it > 'z') ) {

				*(it1++) = '%';
				*(it1++) = hex[ *it >> 4 ];
				*(it1++) = hex[ *it & 0x0F ];
			} else
				*(it1++) = *it;

	*it1 = '\0';
	//*JL_RVAL = STRING_TO_JSVAL( JL_NewString(cx, dest, it1-dest ) ); // do not include the '\0' in the string length
	*JL_RVAL = STRING_TO_JSVAL( JLStr(dest, it1-dest, true).GetJSString(cx) );
	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( URLDecode ) {

	JLStr srcStr;

	JL_S_ASSERT_ARG_MIN( 1 );
	const char *src;
	size_t srcLen;

//	JL_CHK( JL_JsvalToStringAndLength(cx, &JL_ARG(1), &src, &srcLen) );
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &srcStr) );
	srcLen = srcStr.Length();
	src = srcStr.GetConstStr();

	char *dest;
	dest = (char *)JS_malloc(cx, srcLen + 1);
	JL_CHK( dest );

	const char *it;
	char *it1;
	for ( it = src, it1 = dest; it < src + srcLen; it++ )

		if ( *it == '+' )
			*(it1++) = ' ';
		else
			if ( *it == '%' ) {
				
				if ( it + 3 > src + srcLen ) // %XX is 3 chars length
					goto decoding_error;

				it++;
				char c;
				if ( *it >= '0' && *it <= '9' )
					c = (*it - '0') << 4;
				else
					if ( *it >= 'A' && *it <= 'F' )
						c = (*it - ('A'-10)) << 4;
					else
						if ( *it >= 'a' && *it <= 'f' )
							c = (*it - ('a'-10)) << 4;
						else
							goto decoding_error;

				it++;
				if ( *it >= '0' && *it <= '9' )
					c |= *it - '0';
				else
					if ( *it >= 'A' && *it <= 'F' )
						c |= *it - ('A'-10);
					else
						if ( *it >= 'a' && *it <= 'f' )
							c |= *it - ('a'-10);
						else
							goto decoding_error;

				*(it1++) = c;
			} else
				*(it1++) = *it;

	*it1 = '\0';
	//*JL_RVAL = STRING_TO_JSVAL( JL_NewString(cx, dest, it1-dest ) ); // do not include the '\0' in the string length
	*JL_RVAL = STRING_TO_JSVAL( JLStr(dest, it1-dest, true).GetJSString(cx) );
	return JS_TRUE;

decoding_error:
	JS_free(cx, dest);
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	REVISION(JL_SvnRevToInt("$Revision$"))
	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( Accept )
		FUNCTION( GetParam )
		FUNCTION( Read )
		FUNCTION( Write )
		FUNCTION( Flush )
		FUNCTION( ShutdownPending )
		FUNCTION( Log )

		FUNCTION( URLEncode )
		FUNCTION( URLDecode )

	END_STATIC_FUNCTION_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
//		PROPERTY_READ( params )
	END_STATIC_PROPERTY_SPEC

END_STATIC



/*

#include <jsobj.h>

#include "static.h"

#include "fastcgi.h"

// http://www.fastcgi.com/devkit/doc/fcgi-spec.html

BEGIN_STATIC

DEFINE_FUNCTION( ParseHeader ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	JSObject *record;
	if ( argc >= 2 ) {
		JL_S_ASSERT_OBJECT(argv[1]);
		record = JSVAL_TO_OBJECT(argv[1]);
	} else
		record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	char *data;
	J_JSVAL_TO_STRING( argv[0], data );
	FCGI_Header *header = (FCGI_Header *)data;

	JS_DefineProperty(cx, record, "version", INT_TO_JSVAL( header->version ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "type", INT_TO_JSVAL( header->type ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "requestId", INT_TO_JSVAL( (header->requestIdB1 << 8) + header->requestIdB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "contentLength", INT_TO_JSVAL( (header->contentLengthB1 << 8) + header->contentLengthB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
	JL_BAD;
}

DEFINE_FUNCTION( ParseBeginRequestBody ) {

	JL_S_ASSERT_ARG_MIN( 1 );

	JSObject *record;
	if ( argc >= 2 ) {
		JL_S_ASSERT_OBJECT(argv[1]);
		record = JSVAL_TO_OBJECT(argv[1]);
	} else
		record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	char *data;
	J_JSVAL_TO_STRING( argv[0], data );

	FCGI_BeginRequestBody *beginRequestBody = (FCGI_BeginRequestBody *)data;
	JS_DefineProperty(cx, record, "rule", INT_TO_JSVAL( (beginRequestBody->roleB1 << 8) + beginRequestBody->roleB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "keepConn", BOOLEAN_TO_JSVAL( beginRequestBody->flags & FCGI_KEEP_CONN == 0 ? JSVAL_TRUE : JSVAL_FALSE ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
	JL_BAD;
}


//DEFINE_FUNCTION( ParseEndRequest ) {
//
//	JL_S_ASSERT_ARG_MIN( 1 );
//
//	JSObject *record;
//	if ( argc >= 2 ) {
//		JL_S_ASSERT_OBJECT(argv[1]);
//		record = JSVAL_TO_OBJECT(argv[1]);
//	} else
//		record = JS_NewObject(cx, NULL, NULL, NULL);
//	*rval = OBJECT_TO_JSVAL( record );
//
//	char *data;
//	J_JSVAL_TO_STRING( argv[0], data );
//
//	FCGI_EndRequestBody *endRequestBody = (FCGI_EndRequestBody *)data;
//	unsigned long appStatus = ((endRequestBody->appStatusB3 & 0x7f) << 24) + (endRequestBody->appStatusB2 << 16) + (endRequestBody->appStatusB1 << 8) + endRequestBody->appStatusB0;
//	jsval appStatusVal;
//	JL_NewNumberValue(cx, appStatus, &appStatusVal);
//	JS_DefineProperty(cx, record, "appStatus", appStatusVal, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
//	JS_DefineProperty(cx, record, "protocolStatus", INT_TO_JSVAL( endRequestBody->protocolStatus ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
//	return JS_TRUE;
//}


DEFINE_FUNCTION( ParsePairs ) { // arguments: data [, paramObject ]

	JL_S_ASSERT_ARG_MIN( 1 );

	char *tmp;
	int length;
	J_JSVAL_TO_STRING_AND_LENGTH( argv[0], tmp, length );
	unsigned char *data = (unsigned char *)tmp;
	unsigned char *start = data;

	char buf[128];
	JSObject *params;
	if ( argc >= 2 ) {
		JL_S_ASSERT_OBJECT(argv[1]);
		params = JSVAL_TO_OBJECT(argv[1]);
	} else
		params = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( params );

	while ( data < start + length ) {

		unsigned long nameLength;
		if ( data[0] >> 7 == 0 ) {
			nameLength = data[0];
			data += 1;
		} else {
			nameLength = ((data[0] & 0x7f) << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
			data += 4;
		}

		unsigned long valueLength;
		if ( data[0] >> 7 == 0 ) {
			valueLength = data[0];
			data += 1;
		} else {
			valueLength = ((data[0] & 0x7f) << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
			data += 4;
		}

		char *name;
		if ( nameLength <= sizeof(buf)-1 ) // optimization
			name = buf;
		else
			name = (char*)JS_malloc(cx, nameLength +1);
		name[nameLength] = '\0';
		memcpy(name, data, nameLength);
		data += nameLength;
		JSString *value = JS_NewDependentString(cx, JS_ValueToString(cx, argv[0]), data-start, valueLength);
		JS_DefineProperty(cx, params, name, STRING_TO_JSVAL( value ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE);
		if ( nameLength > sizeof(buf)-1 )
			JS_free(cx, name);
		data += valueLength;
	}
	return JS_TRUE;
	JL_BAD;
}


//FCGI_GET_VALUES


DEFINE_FUNCTION( MakeHeader ) { // type, requestId, contentLength
	
	JL_S_ASSERT_ARG_MIN( 3 );

	FCGI_Header *record = (FCGI_Header*) JS_malloc(cx, sizeof(FCGI_Header));
	JL_CHK( record );

	record->version = 1;

	JL_S_ASSERT_INT(argv[0]);
	record->type = JSVAL_TO_INT(argv[0]);

	JL_S_ASSERT_INT(argv[1]);
	unsigned short requestId = JSVAL_TO_INT(argv[1]);
	record->requestIdB0 = requestId & 0xFF;
	record->requestIdB1 = (requestId >> 8) & 0xFF;

	JL_S_ASSERT_INT(argv[2]);
	unsigned short contentLength = JSVAL_TO_INT(argv[2]);
	record->contentLengthB0 = contentLength & 0xFF;
	record->contentLengthB1 = (contentLength >> 8) & 0xFF;

	record->paddingLength = 0;
	record->reserved = 0;

	JSString *jsstr = JL_NewString(cx, (char*)record, sizeof(FCGI_Header));
	JL_CHK( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}


DEFINE_FUNCTION( MakeEndRequestBody ) {

	JL_S_ASSERT_ARG_MIN( 2 );

	FCGI_EndRequestBody *body = (FCGI_EndRequestBody*) JS_malloc(cx, sizeof(FCGI_EndRequestBody));
	JL_CHK( body );

	unsigned long appStatus;
	JL_CHK( JL_JsvalToNative(cx, argv[0], &appStatus) );
	body->appStatusB0 = appStatus & 0xFF;
	body->appStatusB1 = (appStatus >> 8) & 0xFF;
	body->appStatusB2 = (appStatus >> 16) & 0xFF;
	body->appStatusB3 = (appStatus >> 24) & 0xFF;

	JL_S_ASSERT_NUMBER(argv[1]);
	body->protocolStatus = JSVAL_TO_INT(argv[1]);

	JSString *jsstr = JL_NewString(cx, (char*)body, sizeof(FCGI_EndRequestRecord));
	JL_CHK( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);
	return JS_TRUE;
	JL_BAD;
}



DEFINE_FUNCTION( MakePairs ) {

	JL_S_ASSERT_ARG_MIN( 1 );
	JL_S_ASSERT_OBJECT( argv[0] );

	int bufferPos = 0;
	int bufferLength = 256;
	unsigned char *buffer = (unsigned char *)JS_malloc(cx, bufferLength);

	JSObject *pairs = JSVAL_TO_OBJECT( argv[0] );
	JSIdArray *pairsArray = JS_Enumerate(cx, pairs);
	for ( int i=0; i<pairsArray->length; i++ ) {
		
		jsval keyVal;
		JL_CHK( JS_IdToValue(cx, pairsArray->vector[i], &keyVal) );
		char *key;
		unsigned long keyLen;
		J_JSVAL_TO_STRING_AND_LENGTH( keyVal, key, keyLen );

		jsval valueVal;
		OBJ_GET_PROPERTY(cx, pairs, pairsArray->vector[i], &valueVal);
		char *value;
		unsigned long valueLen;
		J_JSVAL_TO_STRING_AND_LENGTH( valueVal, value, valueLen );

		if ( bufferLength - bufferPos < keyLen + valueLen + 8 ) { // max
			bufferLength += keyLen + valueLen + 8 + 256;
			buffer = (unsigned char *)JS_realloc(cx, buffer, bufferLength);
		}
		
		if ( keyLen > 127 ) {
			
			buffer[bufferPos++] = ( (keyLen >> 24) & 0xFF ) | 0x80;
			buffer[bufferPos++] = (keyLen >> 16) & 0xFF;
			buffer[bufferPos++] = (keyLen >>  8) & 0xFF;
			buffer[bufferPos++] = keyLen & 0xFF;
		} else
			buffer[bufferPos++] = keyLen & 0x7F;

		if ( valueLen > 127 ) {

			buffer[bufferPos++] = ( (valueLen >> 24) & 0xFF ) | 0x80;
			buffer[bufferPos++] = (valueLen >> 16) & 0xFF;
			buffer[bufferPos++] = (valueLen >>  8) & 0xFF;
			buffer[bufferPos++] = valueLen & 0xFF;
		} else
			buffer[bufferPos++] = valueLen & 0x7F;

		if ( bufferLength - bufferPos < valueLen ) {
			bufferLength += valueLen + 256;
			buffer = (unsigned char *)JS_realloc(cx, buffer, bufferLength);
		}

		memcpy(buffer+bufferPos, key, keyLen);
		bufferPos += keyLen;

		memcpy(buffer+bufferPos, value, valueLen);
		bufferPos += valueLen;
	}
	JS_DestroyIdArray(cx, pairsArray);
	*rval = STRING_TO_JSVAL( JL_NewString(cx, (char*)buffer, bufferPos) );
	return JS_TRUE;
	JL_BAD;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( ParseHeader )
		FUNCTION( ParseBeginRequestBody )
		FUNCTION( ParsePairs )
		FUNCTION( MakeHeader )
		FUNCTION( MakeEndRequestBody )
		FUNCTION( MakePairs )
	END_STATIC_FUNCTION_SPEC

END_STATIC



DEFINE_FUNCTION( ParseRecord ) {

	// http://www.fastcgi.com/devkit/doc/fcgi-spec.html

	JL_S_ASSERT_ARG_MIN( 1 );

	char *tmp;
	J_JSVAL_TO_STRING( argv[0], tmp );

	unsigned char *data = (unsigned char *)tmp;

	unsigned char *start = data;
	
	FCGI_Header *header = (FCGI_Header *)data;

	JSObject *record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	JS_DefineProperty(cx, record, "version", INT_TO_JSVAL( header->version ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "type", INT_TO_JSVAL( header->type ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

	JS_DefineProperty(cx, record, "requestId", INT_TO_JSVAL( (header->requestIdB1 << 8) + header->requestIdB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

	unsigned short contentLength = (header->contentLengthB1 << 8) + header->contentLengthB0;
	JS_DefineProperty(cx, record, "contentLength", INT_TO_JSVAL( contentLength ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);

	data += FCGI_HEADER_LEN;

	switch ( header->type ) {
		case FCGI_BEGIN_REQUEST: { // 1

			FCGI_BeginRequestBody *beginRequestBody = (FCGI_BeginRequestBody *)data;
			JS_DefineProperty(cx, record, "rule", INT_TO_JSVAL( (beginRequestBody->roleB1 << 8) + beginRequestBody->roleB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
			JS_DefineProperty(cx, record, "keepConn", BOOLEAN_TO_JSVAL( beginRequestBody->flags & FCGI_KEEP_CONN == 0 ? JSVAL_TRUE : JSVAL_FALSE ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
			break;
		}

		case FCGI_ABORT_REQUEST: { // 2
			break;
		}

		case FCGI_END_REQUEST: { // 3

			FCGI_EndRequestBody *endRequestBody = (FCGI_EndRequestBody *)data;
			unsigned long appStatus = ((endRequestBody->appStatusB3 & 0x7f) << 24) + (endRequestBody->appStatusB2 << 16) + (endRequestBody->appStatusB1 << 8) + endRequestBody->appStatusB0;
			jsval appStatusVal;
			JL_NewNumberValue(cx, appStatus, &appStatusVal);
			JS_DefineProperty(cx, record, "appStatus", appStatusVal, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
			JS_DefineProperty(cx, record, "protocolStatus", INT_TO_JSVAL( endRequestBody->protocolStatus ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
			break;
		}

		case FCGI_PARAMS: { // 4

			char buf[128];
			JSObject *params = JS_DefineObject(cx, record, "params", NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
			unsigned char *paramStart = data;
			
			while ( data - paramStart < contentLength ) {

				unsigned long nameLength;
				if ( data[0] >> 7 == 0 ) {
					nameLength = data[0];
					data += 1;
				} else {
					nameLength = ((data[0] & 0x7f) << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
					data += 4;
				}

				unsigned long valueLength;
				if ( data[0] >> 7 == 0 ) {
					valueLength = data[0];
					data += 1;
				} else {
					valueLength = ((data[0] & 0x7f) << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
					data += 4;
				}

				char *name;
				if ( nameLength <= sizeof(buf)-1 ) // optimization
					name = buf;
				else
					name = (char*)JS_malloc(cx, nameLength +1);
				name[nameLength] = '\0';
				memcpy(name, data, nameLength);
				data += nameLength;
				JSString *value = JS_NewDependentString(cx, JS_ValueToString(cx, argv[0]), data-start, valueLength);
				JS_DefineProperty(cx, params, name, STRING_TO_JSVAL( value ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE);
				if ( nameLength > sizeof(buf)-1 )
					JS_free(cx, name);
				data += valueLength;
			}
			
			break;
		}
	}


	return JS_TRUE;
	JL_BAD;
}

*/
