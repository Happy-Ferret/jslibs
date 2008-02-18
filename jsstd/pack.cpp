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

// #include <limits.h> // included by ../common/platform.h

static Endian systemEndian;

JSBool DetectSystemEndian(JSContext *cx, JSObject *obj) {

	systemEndian = DetectSystemEndianType();
	return JS_TRUE;
}

JSBool CheckSystemTypesSize(JSContext *cx, JSObject *obj) {

	RT_ASSERT( sizeof(int8_t) == 1 && sizeof(int16_t) == 2 && sizeof(int32_t) == 4, "The system has no suitable type for using Pack class." );
	return JS_TRUE;
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

	uint8_t data[8] = { 0 };

	size_t amount = size;
	RT_CHECK_CALL( ReadRawAmount(cx, bufferObject, &amount, (char*)data) );
	if ( amount < size ) { // not enough data to complete the requested operation, then unread the few data we have read.

		RT_CHECK_CALL( UnReadRawChunk(cx, bufferObject, (char*)data, amount) );
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
					RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			} else {

				uint32_t val = *(uint32_t*)data;
				if ( val >> (JSVAL_INT_BITS-1) == 0 ) // check if we can store the value in a simple JSVAL_INT ( -1 because the sign )
					*rval = INT_TO_JSVAL( val );
				else // if not, we have to create a new number
					RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		case sizeof(int64_t):
			if (netConv)
				Network64ToHost64(data);
			if ( isSigned ) {
				
				int64_t val = *(int64_t*)data;
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			} else {

				uint64_t val = *(uint64_t*)data;
				RT_CHECK_CALL( JS_NewNumberValue(cx, val, rval) );
			}
			break;
		default:
			REPORT_ERROR("Unable to manage this size.");
	}
	return JS_TRUE;
}



DEFINE_FUNCTION( Test ) {

	int32_t c;
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

	uint8_t data[8] = { 0 };

	bool outOfRange = false;

	switch (size) {
		case sizeof(int8_t):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt8(cx, jsvalue, (int8_t*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt8(cx, jsvalue, (uint8_t*)data, &outOfRange) );
			break;
		case sizeof(int16_t):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt16(cx, jsvalue, (int16_t*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt16(cx, jsvalue, (uint16_t*)data, &outOfRange) );
			if ( netConv )
				Host16ToNetwork16(data);
			break;
		case sizeof(int32_t):
			if ( isSigned )
				RT_CHECK_CALL( JsvalToSInt32(cx, jsvalue, (int32_t*)data, &outOfRange) );
			else
				RT_CHECK_CALL( JsvalToUInt32(cx, jsvalue, (uint32_t*)data, &outOfRange) );
			if ( netConv )
				Host32ToNetwork32(data);
			break;
		case sizeof(int64_t):
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

	uint8_t data[16];
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
