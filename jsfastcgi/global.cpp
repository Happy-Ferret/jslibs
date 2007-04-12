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

DEFINE_FUNCTION( ParseRecord ) {

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
				
				char *name = (char*)JS_malloc(cx, nameLength +1);
				name[nameLength] = '\0';
				memcpy(name, data, nameLength);
				data += nameLength;
				JSString *value = JS_NewDependentString(cx, JS_ValueToString(cx, argv[0]), data-start, valueLength);
				JS_DefineProperty(cx, params, name, STRING_TO_JSVAL( value ), NULL, NULL, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE);
				JS_free(cx, name);
				data += valueLength;
			}
			
			break;
		}
	}


	return JS_TRUE;
}


CONFIGURE_STATIC

	BEGIN_STATIC_FUNCTION_SPEC
		FUNCTION( ParseRecord )
	END_STATIC_FUNCTION_SPEC

END_STATIC

