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

#include "global.h"

#include "fastcgi.h"


BEGIN_STATIC

DEFINE_FUNCTION( ParseHeader ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *record;
	if ( argc >= 2 ) {
		RT_ASSERT_OBJECT(argv[1]);
		record = JSVAL_TO_OBJECT(argv[1]);
	} else
		record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	char *data;
	RT_JSVAL_TO_STRING( argv[0], data );
	FCGI_Header *header = (FCGI_Header *)data;

	JS_DefineProperty(cx, record, "version", INT_TO_JSVAL( header->version ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "type", INT_TO_JSVAL( header->type ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "requestId", INT_TO_JSVAL( (header->requestIdB1 << 8) + header->requestIdB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "contentLength", INT_TO_JSVAL( (header->contentLengthB1 << 8) + header->contentLengthB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

DEFINE_FUNCTION( ParseBeginRequest ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *record;
	if ( argc >= 2 ) {
		RT_ASSERT_OBJECT(argv[1]);
		record = JSVAL_TO_OBJECT(argv[1]);
	} else
		record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	char *data;
	RT_JSVAL_TO_STRING( argv[0], data );

	FCGI_BeginRequestBody *beginRequestBody = (FCGI_BeginRequestBody *)data;
	JS_DefineProperty(cx, record, "rule", INT_TO_JSVAL( (beginRequestBody->roleB1 << 8) + beginRequestBody->roleB0 ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "keepConn", BOOLEAN_TO_JSVAL( beginRequestBody->flags & FCGI_KEEP_CONN == 0 ? JSVAL_TRUE : JSVAL_FALSE ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}

/*
DEFINE_FUNCTION( ParseEndRequest ) {

	RT_ASSERT_ARGC( 1 );

	JSObject *record;
	if ( argc >= 2 ) {
		RT_ASSERT_OBJECT(argv[1]);
		record = JSVAL_TO_OBJECT(argv[1]);
	} else
		record = JS_NewObject(cx, NULL, NULL, NULL);
	*rval = OBJECT_TO_JSVAL( record );

	char *data;
	RT_JSVAL_TO_STRING( argv[0], data );

	FCGI_EndRequestBody *endRequestBody = (FCGI_EndRequestBody *)data;
	unsigned long appStatus = ((endRequestBody->appStatusB3 & 0x7f) << 24) + (endRequestBody->appStatusB2 << 16) + (endRequestBody->appStatusB1 << 8) + endRequestBody->appStatusB0;
	jsval appStatusVal;
	JS_NewNumberValue(cx, appStatus, &appStatusVal);
	JS_DefineProperty(cx, record, "appStatus", appStatusVal, NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	JS_DefineProperty(cx, record, "protocolStatus", INT_TO_JSVAL( endRequestBody->protocolStatus ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT);
	return JS_TRUE;
}
*/

DEFINE_FUNCTION( ParseParams ) {

	RT_ASSERT_ARGC( 1 );

	char *tmp;
	int length;
	RT_JSVAL_TO_STRING_AND_LENGTH( argv[0], tmp, length );
	unsigned char *data = (unsigned char *)tmp;
	unsigned char *start = data;

	char buf[128];
	JSObject *params;
	if ( argc >= 2 ) {
		RT_ASSERT_OBJECT(argv[1]);
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
}


//FCGI_GET_VALUES




DEFINE_FUNCTION( MakeStdoutHeader ) {
	
	RT_ASSERT_ARGC( 2 );

	FCGI_Header *record = (FCGI_Header*) JS_malloc(cx, sizeof(FCGI_Header));
	RT_ASSERT_ALLOC( record );

	record->version = 1;
	record->type = FCGI_STDOUT;

	RT_ASSERT_INT(argv[0]);
	unsigned short requestId = JSVAL_TO_INT(argv[0]);

	record->requestIdB0 = requestId & 0xFF;
	record->requestIdB1 = requestId >> 8;

	RT_ASSERT_INT(argv[1]);
	unsigned short contentLength = JSVAL_TO_INT(argv[1]);
	record->contentLengthB0 = contentLength & 0xFF;
	record->contentLengthB1 = contentLength >> 8;

	record->paddingLength = 0;
	record->reserved = 0;

	JSString *jsstr = JS_NewString(cx, (char*)record, sizeof(FCGI_Header));
	RT_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);

	return JS_TRUE;
}


DEFINE_FUNCTION( MakeEndRequestHeader ) {

	RT_ASSERT_ARGC( 3 );

	FCGI_EndRequestRecord *record = (FCGI_EndRequestRecord*) JS_malloc(cx, sizeof(FCGI_EndRequestRecord));
	RT_ASSERT_ALLOC( record );

	record->header.version = 1;
	record->header.type = FCGI_END_REQUEST;

	RT_ASSERT_INT(argv[0]);
	unsigned short requestId = JSVAL_TO_INT(argv[0]);
	record->header.requestIdB0 = requestId & 0xFF;
	record->header.requestIdB1 = (requestId >> 8) & 0xFF;

	unsigned short contentLength = sizeof(FCGI_EndRequestBody);
	record->header.contentLengthB0 = contentLength & 0xFF;
	record->header.contentLengthB1 = (contentLength >> 8) & 0xFF;

	record->header.paddingLength = 0;
	record->header.reserved = 0;

	unsigned long appStatus;
	RT_JSVAL_TO_UINT32( argv[1], appStatus );
	record->body.appStatusB0 = appStatus & 0xFF;
	record->body.appStatusB1 = (appStatus >> 8) & 0xFF;
	record->body.appStatusB2 = (appStatus >> 16) & 0xFF;
	record->body.appStatusB3 = (appStatus >> 24) & 0xFF;

	RT_ASSERT_NUMBER(argv[2]);
	record->body.protocolStatus = JSVAL_TO_INT(argv[2]);

	JSString *jsstr = JS_NewString(cx, (char*)record, sizeof(FCGI_EndRequestRecord));
	RT_ASSERT_ALLOC( jsstr );
	*rval = STRING_TO_JSVAL(jsstr);

	return JS_TRUE;
}



CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
//		FUNCTION( ParseRecord )
		FUNCTION( ParseHeader )
		FUNCTION( ParseBeginRequest )
//		FUNCTION( ParseEndRequest )
		FUNCTION( ParseParams )
		FUNCTION( MakeStdoutHeader )
		FUNCTION( MakeEndRequestHeader )
	END_STATIC_FUNCTION_SPEC

END_STATIC



/*
DEFINE_FUNCTION( ParseRecord ) {

	// http://www.fastcgi.com/devkit/doc/fcgi-spec.html

	RT_ASSERT_ARGC( 1 );

	char *tmp;
	RT_JSVAL_TO_STRING( argv[0], tmp );

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
			JS_NewNumberValue(cx, appStatus, &appStatusVal);
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
}
*/
