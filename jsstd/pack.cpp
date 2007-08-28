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

#include "../common/jsNativeInterface.h"

#include "../common/jsConversionHelper.h"

#include "buffer.h"

#include "limits.h"

static Endian systemEndian;

JSBool DetectSystemEndian(JSContext *cx, JSObject *obj) {

	systemEndian = DetectSystemEndianType();
	return JS_TRUE;
}

JSBool CheckSystemTypesSize(JSContext *cx, JSObject *obj) {

	RT_ASSERT( sizeof(char) == 1 && sizeof(short) == 2 && sizeof(long) == 4, "The system has no suitable type for using Pack class." );
	return JS_TRUE;
}

// cf. _swab()
// 16 bits: #define SWAP_BYTES(X)           ((X & 0xff) << 8) | (X >> 8)
// 32 bits swap: #define SWAP_BYTE(x) ((x<<24) | (x>>24) | ((x&0xFF00)<<8) | ((x&0xFF0000)>>8))
#define BYTE_SWAP(ptr,a,b) { register char tmp = ((char*)ptr)[a]; ((char*)ptr)[a] = ((char*)ptr)[b]; ((char*)ptr)[b] = tmp; }

void ShortToNetworkShort( void *pval ) {

	if ( systemEndian == LittleEndian )
		BYTE_SWAP( pval, 0, 1 )
}

void LongToNetworkLong( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 3 )
		BYTE_SWAP( pval, 1, 2 )
	}
}

void LLongToNetworkLLong( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 7 )
		BYTE_SWAP( pval, 1, 6 )
		BYTE_SWAP( pval, 2, 5 )
		BYTE_SWAP( pval, 3, 4 )
	}
}

void NetworkShortToShort( void *pval ) {

	if ( systemEndian == LittleEndian )
		BYTE_SWAP( pval, 0, 1 )
}

void NetworkLongToLong( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 3 )
		BYTE_SWAP( pval, 1, 2 )
	}
}

void NetworkLLongToLLong( void *pval ) {

	if ( systemEndian == LittleEndian ) {

		BYTE_SWAP( pval, 0, 7 )
		BYTE_SWAP( pval, 1, 6 )
		BYTE_SWAP( pval, 2, 5 )
		BYTE_SWAP( pval, 3, 4 )
	}
}


BEGIN_CLASS( Pack )

DEFINE_FINALIZE() {

	JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, JSVAL_VOID);
}


DEFINE_CONSTRUCTOR() {

	RT_ASSERT_CONSTRUCTING( _class );
	RT_ASSERT_ARGC( 1 );
	RT_ASSERT_OBJECT( argv[0] );
	RT_ASSERT_CLASS( JSVAL_TO_OBJECT( argv[0] ), &classBuffer );
	RT_CHECK_CALL( JS_SetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, argv[0]) );
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadInt ) {

	RT_ASSERT_ARGC(1);

	jsval bufferVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	RT_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t size;
	RT_JSVAL_TO_INT32( argv[0], size );
	
	bool isSigned;
	if ( argc >= 2 )
		RT_JSVAL_TO_BOOL( argv[1], isSigned );
	else
		isSigned = false;
	
	bool netConv;
	if ( argc >= 3 )
		RT_JSVAL_TO_BOOL( argv[2], netConv );
	else
		netConv = false;

	unsigned char data[8] = { 0 };

	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we have read.

		RT_CHECK_CALL( UnReadRawChunk(cx, bufferObject, (char*)data, amount) );
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(char):
			*rval = INT_TO_JSVAL( isSigned ? *(signed char*)data : *(unsigned char*)data );
			break;
		case sizeof(short):
			if (netConv)
				NetworkShortToShort(data);
			if ( isSigned ) {
				signed short val = *(signed short*)data;
				*rval = INT_TO_JSVAL( val );
			} else {
				unsigned short val = *(unsigned short*)data;
				*rval = INT_TO_JSVAL( val );
			}
			break;
		case sizeof(long):
			if (netConv)
				NetworkLongToLong(data);
			if ( isSigned ) {

				signed long val = *(signed long*)data;
				if ( val >> JSVAL_INT_BITS == 0 ) // check if we can store the value in a simple JSVAL_INT
					*rval = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			} else {

				unsigned long val = *(unsigned long*)data;
				if ( val >> (JSVAL_INT_BITS-1) == 0 ) // check if we can store the value in a simple JSVAL_INT ( -1 because the sign )
					*rval = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		case sizeof(LLONG):
			if (netConv)
				NetworkLLongToLLong(data);
			if ( isSigned ) {
				
				signed LLONG val = *(signed LLONG*)data;
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			} else {

				unsigned LLONG val = *(unsigned LLONG*)data;
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		default:
			REPORT_ERROR("Unable to manage this size.");
	}
	return JS_TRUE;
}



DEFINE_FUNCTION( Test ) {

	long c;
	bool isOutOfRange;
	JsvalToSInt32(cx, argv[0], &c, &isOutOfRange);

	return JS_TRUE;
}


DEFINE_FUNCTION( WriteInt ) {
	
	RT_ASSERT_ARGC(2);

	jsval bufferVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	RT_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject = JSVAL_TO_OBJECT( bufferVal );

	jsval jsvalue = argv[0];

	size_t size;
	RT_JSVAL_TO_INT32( argv[1], size );
	
	bool isSigned;
	if ( argc >= 3 )
		RT_JSVAL_TO_BOOL( argv[2], isSigned );
	else
		isSigned = false;

	bool netConv;
	if ( argc >= 4 )
		RT_JSVAL_TO_BOOL( argv[3], netConv );
	else
		netConv = false;

	unsigned char data[8] = { 0 };

	bool outOfRange = false;

	switch (size) {
		case sizeof(char):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt8(cx, jsvalue, (char*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt8(cx, jsvalue, (unsigned char*)data, &outOfRange) );
			break;
		case sizeof(short):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt16(cx, jsvalue, (short*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt16(cx, jsvalue, (unsigned short*)data, &outOfRange) );
			if ( netConv )
				ShortToNetworkShort(data);
			break;
		case sizeof(long):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt32(cx, jsvalue, (long*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt32(cx, jsvalue, (unsigned long*)data, &outOfRange) );
			if ( netConv )
				LongToNetworkLong(data);
			break;
		case sizeof(LLONG):
			// (TBD) implement it
			// break;
		default:
			REPORT_ERROR("Unable to manage this size.");
	}

	RT_ASSERT( !outOfRange, "Value size too big to be stored." );

	RT_CHECK_CALL( WriteRawData(cx, bufferObject, size, (char*)data) );

	return JS_TRUE;
}


DEFINE_FUNCTION( ReadReal ) {

	RT_ASSERT_ARGC(1);

	jsval bufferVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	RT_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject = JSVAL_TO_OBJECT( bufferVal );

	size_t size = JSVAL_TO_INT( argv[0] );

	unsigned char data[16];
	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we read

		RT_CHECK_CALL( UnReadRawChunk(cx, bufferObject, (char*)data, amount) );
		*rval = JSVAL_VOID;
		return JS_TRUE;
	}

	switch (size) {
		case sizeof(float):
			RT_CHECK_CALL( JS_NewNumberValue(cx, *((float*)data), rval) );
			break;
		case sizeof(double):
			RT_CHECK_CALL( JS_NewNumberValue(cx, *((double*)data), rval) );
			break;
		default:
			*rval = JSVAL_VOID;
	}
	return JS_TRUE;
}


DEFINE_FUNCTION( ReadString ) {

	jsval bufferVal;
	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, &bufferVal) );
	RT_ASSERT_DEFINED( bufferVal );
	JSObject *bufferObject = JSVAL_TO_OBJECT( bufferVal );

	if ( argc >= 1 ) {
		
		size_t amount;
		RT_JSVAL_TO_INT32( argv[0], amount );
		RT_ASSERT( amount >= 0, "Invalid amount" );
		RT_CHECK_CALL( ReadAmount(cx, bufferObject, amount, rval) );
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
}


DEFINE_PROPERTY( buffer ) {

	RT_CHECK_CALL( JS_GetReservedSlot(cx, obj, SLOT_PACK_BUFFEROBJECT, vp ) );
	return JS_TRUE;
}

DEFINE_PROPERTY( systemIntSize ) {

	*vp = INT_TO_JSVAL( sizeof(int) );
	return JS_TRUE;
}

DEFINE_PROPERTY( systemBigEndian ) {

	*vp = BOOLEAN_TO_JSVAL( systemEndian == BigEndian );
	return JS_TRUE;
}

CONFIGURE_CLASS

	CALL_ON_INIT( DetectSystemEndian )
	CALL_ON_INIT( CheckSystemTypesSize )

	HAS_CONSTRUCTOR
	HAS_FINALIZE

	BEGIN_FUNCTION_SPEC
		FUNCTION(ReadInt)
		FUNCTION(ReadReal)
		FUNCTION(ReadString)

		FUNCTION(WriteInt)

		FUNCTION(Test)

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
