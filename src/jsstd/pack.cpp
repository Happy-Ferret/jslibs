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


DECLARE_CLASS( Buffer )
#define SLOT_PACK_BUFFEROBJECT 0
#include "buffer.h"

#include "limits.h"

inline JSBool JL_JsvalToSInt8( JSContext *cx, jsval val, int8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-128) || v > (127);
		*result = (int8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-128) || d > (double)(127);
		*result = (int8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int8_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt8( JSContext *cx, jsval val, uint8_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > (0xff);
		*result = (uint8_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xff);
		*result = (uint8_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint8_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (uint8_t)s[0];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt16( JSContext *cx, jsval val, int16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFF - 1) || v > (0x7FFF);
		*result = (int16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFF - 1) || d > (double)(0x7FFF);
		*result = (int16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int16_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0]<<8 | s[1];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt16( JSContext *cx, jsval val, uint16_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (0) || v > 0xffff;
		*result = (uint16_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xffff);
		*result = (uint16_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint16_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = s[0]<<8 | s[1];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt24( JSContext *cx, jsval val, int32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFFFFL - 1) || v > (0x7FFFFFL);
		*result = (int32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFFFFL - 1) || d > (double)(0x7FFFFFL);
		*result = (int32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < 3 )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = ((s[0]<<8 | s[1])<<8) | s[2];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToUInt24( JSContext *cx, jsval val, uint32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		unsigned int v = JSVAL_TO_INT( val );
		*outOfRange = v < 0 || v > (0xFFFFFFUL);
		*result = (uint32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < 0 || d > (double)(0xFFFFFFUL) || d != (double)(int32_t)d;
		*result = (uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < 3 )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = ((s[0]<<8 | s[1])<<8) | s[2];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


inline JSBool JL_JsvalToSInt32( JSContext *cx, jsval val, int32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = v < (-0x7FFFFFFFL - 1) || v > (0x7FFFFFFFL);
		*result = (int32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (double)(-0x7FFFFFFFL - 1) || d > (double)(0x7FFFFFFFL);
		*result = (int32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int32_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt32( JSContext *cx, jsval val, uint32_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		long v = JSVAL_TO_INT( val ); // beware: int31 ! not int32
		*outOfRange = v < (0);// || v > ULONG_MAX; <- this case is impossible
		*result = (uint32_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < (0) || d > (double)(0xFFFFFFFFUL);
		*result = (uint32_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(uint32_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3];
		*outOfRange = *result < 0;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}


// range if jsval is a jsdouble: +/-2^53
inline JSBool JL_JsvalToSInt64( JSContext *cx, jsval val, int64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (int64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < -MAX_INT_TO_DOUBLE || d > MAX_INT_TO_DOUBLE || d != (double)(int64_t)d;
		*result = (int64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int64_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((((((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3]<<8) | s[4]<<8) | s[5]<<8) | s[6]<<8) | s[7];
		*outOfRange = false;
	} else {
		
		return JS_FALSE;
	}
	return JS_TRUE;
}

inline JSBool JL_JsvalToUInt64( JSContext *cx, jsval val, uint64_t *result, bool *outOfRange ) {

	if ( JSVAL_IS_INT( val ) ) {

		int v = JSVAL_TO_INT( val );
		*outOfRange = false;
		*result = (uint64_t)v;
	} else if ( JSVAL_IS_DOUBLE( val ) ) {

		double d = JSVAL_TO_DOUBLE(val);
		*outOfRange = d < 0 || d > MAX_INT_TO_DOUBLE || d != (double)(int64_t)d;
		*result = (uint64_t)d;
	} else if ( JSVAL_IS_STRING( val ) ) { // using system byte order

		JLStr str;
		if ( !JL_JsvalToNative(cx, val, &str) || !str.IsSet() || str.Length() < sizeof(int64_t) )
			return JS_FALSE;
		const char *s = str.GetConstStr();
		*result = (((((((s[0]<<8 | s[1])<<8) | s[2])<<8 | s[3]<<8) | s[4]<<8) | s[5]<<8) | s[6]<<8) | s[7];
		*outOfRange = *result < 0;
	} else
		return JS_FALSE;
	return JS_TRUE;
}


/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 Pack is a class that helps to convert binary data into Integer, Real or String and to write an integer in a binary data string.
 The Pack class manages the system endian or network endian.
**/
BEGIN_CLASS( Pack )

DEFINE_FINALIZE() {

	if ( JL_GetHostPrivate(cx)->canSkipCleanup )
		return;

	JL_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, JSVAL_VOID); // (TBD) check if it is legal
}

/**doc
$TOC_MEMBER $INAME
 $INAME( buffer [ , useNetworkEndian ] )
  Constructs a Pack object from a Buffer object. This is the only way to read or write binary data.
  $H arguments
   $ARG Buffer buffer
**/
DEFINE_CONSTRUCTOR() {

	JL_S_ASSERT_CONSTRUCTING();
	JL_DEFINE_CONSTRUCTOR_OBJ;

	JL_S_ASSERT_ARG_RANGE(1,2);

	JL_S_ASSERT_OBJECT( JL_ARG(1) );
	JL_S_ASSERT_CLASS( JSVAL_TO_OBJECT( JL_ARG(1) ), JL_CLASS(Buffer) );
	JL_CHK( JL_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, JL_ARG(1)) );

	bool useNetworkEndian;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &useNetworkEndian) );
	else
		useNetworkEndian = false;
	JL_SetPrivate(cx, obj, (void*)(size_t)(useNetworkEndian ? 2 : 0));

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
$TOC_MEMBER $INAME
 $INT $INAME( size, [isSigned = false], [isNetworkEndian = false] )
  Read an integer on the current stream. cf. systemIntSize property.$LF
  Supported sizes are 1, 2, 3, 4, 8 (for 8-bit, 16-bit, 24-bit, 32-bit, 64-bit). 64-bit values range is [-2^53-1, 2^53-1].$LF
  Returns $UNDEF if the buffer does not contain enough data to read the integer.
**/
DEFINE_FUNCTION( ReadInt ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_RANGE(1, 3);

	jsval bufferVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	JL_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &size) );

	bool isSigned;
	if ( JL_ARG_ISDEF(2) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &isSigned) );
	else
		isSigned = false;

	bool netConv;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &netConv) );
	else
		netConv = (size_t)JL_GetPrivate(cx, obj) != 0;

	uint8_t data[8]; // = { 0 };
	memset(data, 0, sizeof(data));

	size_t amount;
	amount = size;
	JL_CHK( ReadRawDataAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we have read.

		JL_CHK( UnReadRawDataChunk(cx, bufferObject, (char*)data, amount) ); // incompatible with NIStreamRead
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(int8_t):
			*JL_RVAL = INT_TO_JSVAL( isSigned ? *(int8_t*)data : *(uint8_t*)data );
			break;
		case sizeof(int16_t):
			if (netConv)
				Network16ToHost16(data);
			if ( isSigned ) {
				int16_t val = *(int16_t*)data;
				*JL_RVAL = INT_TO_JSVAL( val );
			} else {
				uint16_t val = *(uint16_t*)data;
				*JL_RVAL = INT_TO_JSVAL( val );
			}
			break;
		case 3: // 24-bit
			if (netConv)
				Network24ToHost24(data);
			if ( isSigned ) {

				int32_t val = (int32_t)(*(uint32_t*)data << 8) >> 8;
				if ( val >= JSVAL_INT_MIN && val <= JSVAL_INT_MAX )
					*JL_RVAL = INT_TO_JSVAL( val );
				else
					JL_CHK( JL_NewNumberValue(cx, val, JL_RVAL) );

			} else {

				uint32_t val = *(uint32_t*)data;
				if ( val <= JSVAL_INT_MAX )
					*JL_RVAL = INT_TO_JSVAL( val );
				else
					JL_CHK( JL_NewNumberValue(cx, val, JL_RVAL) );
			}
			break;
		case sizeof(int32_t):
			if (netConv)
				Network32ToHost32(data);
			if ( isSigned ) {

				int32_t val = *(int32_t*)data;
				if ( val >= JSVAL_INT_MIN && val <= JSVAL_INT_MAX )
					*JL_RVAL = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					JL_CHK( JL_NewNumberValue(cx, val, JL_RVAL) );
			} else {

				uint32_t val = *(uint32_t*)data;
				if ( val <= JSVAL_INT_MAX )
					*JL_RVAL = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					JL_CHK( JL_NewNumberValue(cx, val, JL_RVAL) );
			}
			break;
		case sizeof(int64_t):
			if (netConv)
				Network64ToHost64(data);

			if ( *(int64_t*)data > MAX_INT_TO_DOUBLE && *(int64_t*)data < -MAX_INT_TO_DOUBLE )
				JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);

			if ( isSigned ) {

				int64_t val = *(int64_t*)data;
				JL_CHK( JL_NewNumberValue(cx, (jsdouble)val, JL_RVAL) );
			} else {

				uint64_t val = *(uint64_t*)data;
				JL_CHK( JL_NewNumberValue(cx, (jsdouble)val, JL_RVAL) );
			}
			break;
		default:
			JL_CHK( UnReadRawDataChunk(cx, bufferObject, (char*)data, amount) ); // incompatible with NIStreamRead
			JL_REPORT_ERROR("Unsupported data type.");
	}
	return JS_TRUE;
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $VOID $INAME( intValue, size, [isSigned = false [, isNetworkEndian = false]] )
  Write an integer to the current stream. cf. systemIntSize property.$LF
  Supported sizes are 1, 2, 3, 4, 8 (for 8-bit, 16-bit, 24-bit, 32-bit, 64-bit). 64-bit values range is [-2^53-1, 2^53-1].
**/
DEFINE_FUNCTION( WriteInt ) { // incompatible with NIStreamRead

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG_RANGE(1, 4);

	jsval bufferVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	JL_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	jsval jsvalue;
	jsvalue = JL_ARG(1);

	size_t size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(2), &size) );

	bool isSigned;
	if ( JL_ARG_ISDEF(3) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(3), &isSigned) );
	else
		isSigned = false;

	bool netConv;
	if ( JL_ARG_ISDEF(4) )
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(4), &netConv) );
	else
		netConv = (size_t)JL_GetPrivate(cx, obj) != 0;

	uint8_t data[8]; // = { 0 };
	memset(data, 0, sizeof(data));

	bool outOfRange;
	outOfRange = false;

	switch (size) {
		case sizeof(int8_t):
			if ( isSigned )
				JL_CHK( JL_JsvalToSInt8(cx, jsvalue, (int8_t*)data, &outOfRange) );
			else
				JL_CHK( JL_JsvalToUInt8(cx, jsvalue, (uint8_t*)data, &outOfRange) );
			break;
		case sizeof(int16_t):
			if ( isSigned )
				JL_CHK( JL_JsvalToSInt16(cx, jsvalue, (int16_t*)data, &outOfRange) );
			else
				JL_CHK( JL_JsvalToUInt16(cx, jsvalue, (uint16_t*)data, &outOfRange) );
			if ( netConv )
				Host16ToNetwork16(data);
			break;
		case 3: // 24-bit
			if ( isSigned )
				JL_CHK( JL_JsvalToSInt24(cx, jsvalue, (int32_t*)data, &outOfRange) );
			else
				JL_CHK( JL_JsvalToUInt24(cx, jsvalue, (uint32_t*)data, &outOfRange) );
			if ( netConv )
				Host24ToNetwork24(data);
			break;
		case sizeof(int32_t):
			if ( isSigned )
				JL_CHK( JL_JsvalToSInt32(cx, jsvalue, (int32_t*)data, &outOfRange) );
			else
				JL_CHK( JL_JsvalToUInt32(cx, jsvalue, (uint32_t*)data, &outOfRange) );
			if ( netConv )
				Host32ToNetwork32(data);
			break;
		case sizeof(int64_t):
			if ( isSigned )
				JL_CHK( JL_JsvalToSInt64(cx, jsvalue, (int64_t*)data, &outOfRange) );
			else
				JL_CHK( JL_JsvalToUInt64(cx, jsvalue, (uint64_t*)data, &outOfRange) );
			if ( netConv )
				Host64ToNetwork64(data);
			break;
		default:
			JL_REPORT_ERROR("Unsupported data type.");
	}

	if ( outOfRange )
		JL_REPORT_ERROR_NUM(cx, JLSMSG_VALUE_OUTOFRANGE);

	JL_CHK( WriteRawDataChunk(cx, bufferObject, size, (char*)data) );
	
	*JL_RVAL = JSVAL_VOID;
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $REAL $INAME( size )
  Read a 4-byte single precision real (float) or a 8-byte double precision real (double) on the current stream.$LF
  Returns $UNDEF if the buffer does not contain enough data to read the integer.
**/
DEFINE_FUNCTION( ReadReal ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();
	JL_S_ASSERT_ARG(1);

	jsval bufferVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	JL_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t amount, size;
	JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &size) );
	uint8_t data[16];
	amount = size;
	JL_CHK( ReadRawDataAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		JL_CHK( UnReadRawDataChunk(cx, bufferObject, (char*)data, amount) ); // incompatible with NIStreamRead
		*JL_RVAL = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(float):
			JL_CHK( JL_NewNumberValue(cx, *((float*)data), JL_RVAL) );
			break;
		case sizeof(double):
			JL_CHK( JL_NewNumberValue(cx, *((double*)data), JL_RVAL) );
			break;
		default:
			*JL_RVAL = JSVAL_VOID;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
$TOC_MEMBER $INAME
 $STR $INAME( length )
  Read a string of the specifiex _length_ on the current stream.
**/
DEFINE_FUNCTION( ReadString ) {

	JL_DEFINE_FUNCTION_OBJ;
	JL_S_ASSERT_THIS_CLASS();

	jsval bufferVal;
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	JL_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	if ( JL_ARG_ISDEF(1) ) {

		size_t amount;
		JL_CHK( JL_JsvalToNative(cx, JL_ARG(1), &amount) );
//		JL_S_ASSERT( (int)amount >= 0, "Invalid amount" );
		JL_S_ASSERT_ERROR_NUM( (int)amount >= 0, JLSMSG_VALUE_OUTOFRANGE );
		JL_CHK( ReadDataAmount(cx, bufferObject, amount, JL_RVAL) );
	} else {

		// get a chunk
		// check if it contains '\0'
		// else store it and get a new chunk...
		// if it contains '\0',
		// create a buffer of the right size,
		// strcat all stored chunks, and unread the remainder if any
		// else if no '\0' and the buffer is empty, unread all stored chunks
	}

	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $BOOL *useNetworkEndian*
**/
DEFINE_PROPERTY_SETTER( useNetworkEndian ) {

	JL_USE(id);

	JL_S_ASSERT_THIS_CLASS();
	bool useNetworkEndian;
	JL_CHK( JL_JsvalToNative(cx, *vp, &useNetworkEndian) );
	JL_SetPrivate(cx, obj, (void*)(size_t)(useNetworkEndian ? 2 : 0));
	return JS_TRUE;
	JL_BAD;
}

DEFINE_PROPERTY_GETTER( useNetworkEndian ) {

	JL_USE(id);

	JL_S_ASSERT_THIS_CLASS();
	return JL_NativeToJsval(cx, (size_t)JL_GetPrivate(cx, obj) != 0, vp);
	JL_BAD;
}


/**doc
$TOC_MEMBER $INAME
 $TYPE Buffer *buffer*
  Is the current Buffer object.
**/
DEFINE_PROPERTY( buffer ) {

	JL_USE(id);

	JL_S_ASSERT_THIS_CLASS();
	JL_CHK( JL_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, vp ) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static properties ===
**/

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is the size (in Byte) of a system int.
**/
DEFINE_PROPERTY( systemIntSize ) {

	JL_USE(id);
	JL_USE(obj);
	JL_USE(cx);

	*vp = INT_TO_JSVAL( sizeof(size_t) );
	return JS_TRUE;
}

/**doc
$TOC_MEMBER $INAME
 $INAME $READONLY
  Is $TRUE if the system endian is BigEndian else is $FALSE.
**/
DEFINE_PROPERTY( systemIsBigEndian ) {

	JL_USE(id);
	JL_USE(obj);
	JL_USE(cx);

	*vp = BOOLEAN_TO_JSVAL( JLHostEndian == JLBigEndian );
	return JS_TRUE;
}

DEFINE_INIT() {

	JL_USE(obj);
	JL_USE(proto);
	JL_USE(sc);

	JL_S_ASSERT( sizeof(int8_t) == 1 && sizeof(int16_t) == 2 && sizeof(int32_t) == 4 && sizeof(int64_t) == 8, "The system has no suitable type for using Pack class." );
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(JL_SvnRevToInt("$Revision$"))
	HAS_PRIVATE
	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_INIT

	BEGIN_FUNCTION_SPEC
		FUNCTION(ReadInt)
		FUNCTION(ReadReal)
		FUNCTION(ReadString)
		FUNCTION(WriteInt)
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY(useNetworkEndian)
		PROPERTY_READ(buffer)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(systemIntSize)
		PROPERTY_READ(systemIsBigEndian)
	END_STATIC_PROPERTY_SPEC

//	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
