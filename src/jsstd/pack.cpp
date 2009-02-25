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
#include "pack.h"

//#include "../common/jsNativeInterface.h"

#include "../common/jsConversionHelper.h"

#include "buffer.h"

// #include <limits.h> // included by ../common/platform.h

static Endian systemEndian; // it's safe to use static keyword.

JSBool DetectSystemEndian(JSContext *cx) {

	systemEndian = DetectSystemEndianType();
	return JS_TRUE;
}

JSBool CheckSystemTypesSize(JSContext *cx) {

	J_S_ASSERT( sizeof(int8_t) == 1 && sizeof(int16_t) == 2 && sizeof(int32_t) == 4, "The system has no suitable type for using Pack class." );
	return JS_TRUE;
JL_BAD;
}

// cf. _swab()
// 16 bits: #define SWAP_BYTES(X)           ((X & 0xff) << 8) | (X >> 8)
// 32 bits swap: #define SWAP_BYTE(x) ((x<<24) | (x>>24) | ((x&0xFF00)<<8) | ((x&0xFF0000)>>8))
#define BYTE_SWAP(ptr,a,b) { register char tmp = ((int8_t*)ptr)[a]; ((int8_t*)ptr)[a] = ((int8_t*)ptr)[b]; ((int8_t*)ptr)[b] = tmp; }

void Host16ToNetwork16( void *pval ) {

	if ( systemEndian == LittleEndian )
		BYTE_SWAP( pval, 0, 1 )
}

void Host32ToNetwork32( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 3 )
		BYTE_SWAP( pval, 1, 2 )
	}
}

void Host64ToNetwork64( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 7 )
		BYTE_SWAP( pval, 1, 6 )
		BYTE_SWAP( pval, 2, 5 )
		BYTE_SWAP( pval, 3, 4 )
	}
}


void Network16ToHost16( void *pval ) {

	if ( systemEndian == LittleEndian )
		BYTE_SWAP( pval, 0, 1 )
}

void Network32ToHost32( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 3 )
		BYTE_SWAP( pval, 1, 2 )
	}
}

void Network64ToHost64( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 7 )
		BYTE_SWAP( pval, 1, 6 )
		BYTE_SWAP( pval, 2, 5 )
		BYTE_SWAP( pval, 3, 4 )
	}
}



/**doc
$CLASS_HEADER
$SVN_REVISION $Revision$
 Pack is a class that helps to convert binary data into Integer, Real or String and to write an integer in a binary data string.
 The Pack class manages the system endian or network endian.
**/
BEGIN_CLASS( Pack )

DEFINE_FINALIZE() {

	JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, JSVAL_VOID);
}

/**doc
 * $INAME( buffer )
  Constructs a Pack object from a Buffer object. This is the only way to read or write binary data.
  $H arguments
   $ARG Buffer buffer
**/
DEFINE_CONSTRUCTOR() {

	J_S_ASSERT_CONSTRUCTING();
	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN( 1 );
	J_S_ASSERT_OBJECT( J_ARG(1) );
	J_S_ASSERT_CLASS( JSVAL_TO_OBJECT( J_ARG(1) ), classBuffer );
	J_CHK( JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, J_ARG(1)) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
=== Methods ===
**/

/**doc
 * $INT $INAME( size, [isSigned = false], [isNetworkEndian = false] )
  Read an integer on the current stream. cf. systemIntSize property.
**/
DEFINE_FUNCTION( ReadInt ) {

	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);

	jsval bufferVal;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	J_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t size;
	J_CHK( JsvalToUInt(cx, J_ARG(1), &size) );

	bool isSigned;
	if ( J_ARG_ISDEF(2) )
		J_CHK( JsvalToBool(cx, J_ARG(2), &isSigned) );
	else
		isSigned = false;

	bool netConv;
	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToBool(cx, J_ARG(3), &netConv) );
	else
		netConv = false;

	uint8_t data[8]; // = { 0 };
	memset(data, 0, sizeof(data));

	size_t amount;
	amount = size;
	J_CHK( ReadRawAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we have read.

		J_CHK( UnReadRawChunk(cx, bufferObject, (char*)data, amount) ); // incompatible with NIStreamRead
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(int8_t):
			*rval = INT_TO_JSVAL( isSigned ? *(int8_t*)data : *(uint8_t*)data );
			break;
		case sizeof(int16_t):
			if (netConv)
				Network16ToHost16(data);
			if ( isSigned ) {
				int16_t val = *(int16_t*)data;
				*rval = INT_TO_JSVAL( val );
			} else {
				uint16_t val = *(uint16_t*)data;
				*rval = INT_TO_JSVAL( val );
			}
			break;
		case sizeof(int32_t):
			if (netConv)
				Network32ToHost32(data);
			if ( isSigned ) {

				int32_t val = *(int32_t*)data;
				if ( val >> JSVAL_INT_BITS == 0 ) // check if we can store the value in a simple JSVAL_INT
					*rval = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					J_CHK( JS_NewNumberValue(cx, val, rval) );
			} else {

				uint32_t val = *(uint32_t*)data;
				if ( val >> (JSVAL_INT_BITS-1) == 0 ) // check if we can store the value in a simple JSVAL_INT ( -1 because the sign )
					*rval = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					J_CHK( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		case sizeof(int64_t):
			if (netConv)
				Network64ToHost64(data);
			if ( isSigned ) {

				int64_t val = *(int64_t*)data;
				J_CHK( JS_NewNumberValue(cx, val, rval) );
			} else {

				uint64_t val = *(uint64_t*)data;
				J_CHK( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		default:
			J_REPORT_ERROR("Unable to manage this size.");
	}
	return JS_TRUE;
	JL_BAD;
}

#ifdef DEBUG
DEFINE_FUNCTION( Test ) {

	int32_t c;
	bool isOutOfRange;
	JsvalToSInt32(cx, J_ARG(1), &c, &isOutOfRange);

	return JS_TRUE;
}
#endif

/**doc
 * $VOID $INAME( intValue, [isSigned = false [, isNetworkEndian = false]] )
  Write an integer to the current stream. cf. systemIntSize property.
**/
DEFINE_FUNCTION( WriteInt ) { // incompatible with NIStreamRead

	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(2);

	jsval bufferVal;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	J_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	jsval jsvalue;
	jsvalue = J_ARG(1);

	size_t size;
	J_CHK( JsvalToUInt(cx, J_ARG(2), &size) );

	bool isSigned;
	if ( J_ARG_ISDEF(3) )
		J_CHK( JsvalToBool(cx, J_ARG(3), &isSigned) );
	else
		isSigned = false;

	bool netConv;
	if ( J_ARG_ISDEF(4) )
		J_CHK( JsvalToBool(cx, J_ARG(4), &netConv) );
	else
		netConv = false;

	uint8_t data[8]; // = { 0 };
	memset(data, 0, sizeof(data));

	bool outOfRange;
	outOfRange = false;

	switch (size) {
		case sizeof(int8_t):
			if ( isSigned )
				J_CHK( JsvalToSInt8(cx, jsvalue, (int8_t*)data, &outOfRange) );
			else
				J_CHK( JsvalToUInt8(cx, jsvalue, (uint8_t*)data, &outOfRange) );
			break;
		case sizeof(int16_t):
			if ( isSigned )
				J_CHK( JsvalToSInt16(cx, jsvalue, (int16_t*)data, &outOfRange) );
			else
				J_CHK( JsvalToUInt16(cx, jsvalue, (uint16_t*)data, &outOfRange) );
			if ( netConv )
				Host16ToNetwork16(data);
			break;
		case sizeof(int32_t):
			if ( isSigned )
				J_CHK( JsvalToSInt32(cx, jsvalue, (int32_t*)data, &outOfRange) );
			else
				J_CHK( JsvalToUInt32(cx, jsvalue, (uint32_t*)data, &outOfRange) );
			if ( netConv )
				Host32ToNetwork32(data);
			break;
		case sizeof(int64_t):
			// (TBD) implement it
			// break;
		default:
			J_REPORT_ERROR("Unable to manage this size.");
	}

	J_S_ASSERT( !outOfRange, "Value size too big to be stored." );

	J_CHK( WriteRawChunk(cx, bufferObject, size, (char*)data) );

	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $REAL $INAME( size )
  Read a 4-byte single precision real (float) or a 8-byte double precision real (double) on the current stream.
**/
DEFINE_FUNCTION( ReadReal ) {

	J_S_ASSERT_THIS_CLASS();
	J_S_ASSERT_ARG_MIN(1);

	jsval bufferVal;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	J_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t size;
	size = JSVAL_TO_INT( J_ARG(1) );

	uint8_t data[16];
	size_t amount;
	amount = size;
	J_CHK( ReadRawAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		J_CHK( UnReadRawChunk(cx, bufferObject, (char*)data, amount) ); // incompatible with NIStreamRead
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(float):
			J_CHK( JS_NewNumberValue(cx, *((float*)data), rval) );
			break;
		case sizeof(double):
			J_CHK( JS_NewNumberValue(cx, *((double*)data), rval) );
			break;
		default:
			*rval = JSVAL_VOID;
	}
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $STR $INAME( length )
  Read a string of the specifiex _length_ on the current stream.
**/
DEFINE_FUNCTION( ReadString ) {

	J_S_ASSERT_THIS_CLASS();
	jsval bufferVal;
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	J_S_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject;
	bufferObject = JSVAL_TO_OBJECT( bufferVal );

	if ( J_ARG_ISDEF(1) ) {

		size_t amount;
		J_CHK( JsvalToUInt(cx, J_ARG(1), &amount) );
		J_S_ASSERT( amount >= 0, "Invalid amount" );
		J_CHK( ReadAmount(cx, bufferObject, amount, rval) );
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
 * $TYPE Buffer *buffer*
  Is the current Buffer object.
**/
DEFINE_PROPERTY( buffer ) {

	J_S_ASSERT_THIS_CLASS();
	J_CHK( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, vp ) );
	return JS_TRUE;
	JL_BAD;
}


/**doc
=== Static properties ===
**/

/**doc
 * $INAME $READONLY
  Is the size (in Byte) of a system int.
**/
DEFINE_PROPERTY( systemIntSize ) {

	J_S_ASSERT_THIS_CLASS();
	*vp = INT_TO_JSVAL( sizeof(int) );
	return JS_TRUE;
	JL_BAD;
}

/**doc
 * $INAME $READONLY
  Is $TRUE if the system endian is BigEndian else is $FALSE.
**/
DEFINE_PROPERTY( systemBigEndian ) {

	*vp = BOOLEAN_TO_JSVAL( systemEndian == BigEndian );
	return JS_TRUE;
	JL_BAD;
}

JSBool Init(JSContext *cx, JSObject *obj) {

	J_CHK( DetectSystemEndian(cx) );
	J_CHK( CheckSystemTypesSize(cx) );
	return JS_TRUE;
	JL_BAD;
}

CONFIGURE_CLASS

	REVISION(SvnRevToInt("$Revision$"))

	HAS_CONSTRUCTOR
	HAS_FINALIZE
	HAS_INIT

	BEGIN_FUNCTION_SPEC
		FUNCTION(ReadInt)
		FUNCTION(ReadReal)
		FUNCTION(ReadString)
		FUNCTION(WriteInt)
#ifdef DEBUG
		FUNCTION(Test)
#endif
	END_FUNCTION_SPEC

	BEGIN_PROPERTY_SPEC
		PROPERTY_READ(buffer)
	END_PROPERTY_SPEC

	BEGIN_STATIC_PROPERTY_SPEC
		PROPERTY_READ(systemIntSize)
		PROPERTY_READ(systemBigEndian)
	END_STATIC_PROPERTY_SPEC

//	HAS_PRIVATE
	HAS_RESERVED_SLOTS(1)

END_CLASS
