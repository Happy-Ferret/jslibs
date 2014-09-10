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


#pragma once

DECLARE_CLASS( BlobStream )
DECLARE_CLASS( Database )

#define MAX_FUNCTION_ARG 64

struct DatabasePrivate {
	sqlite3 *db;
	void *stmtList;
	void *blobList;
	void *fctpvList;
	JSContext *tmpcx;
};

struct FunctionPrivate : public jl::CppAllocators {
	DatabasePrivate *dbpv;
	JS::Heap<JSObject*> obj;
	JS::Heap<JS::Value> fval;
};


////


struct SqliteTargetResult {
	
	sqlite3_context *_pCtx;
	
	SqliteTargetResult( sqlite3_context *pCtx )
	: _pCtx( pCtx ) {
	}

	sqlite3 *
	getDatabase() {

		return sqlite3_context_db_handle( _pCtx );
	}

	int
	setNull() {

		sqlite3_result_null( _pCtx );
		return SQLITE_OK;
	}

	int
	setInt64( sqlite3_int64 i ) {

		sqlite3_result_int64( _pCtx, i );
		return SQLITE_OK;
	}

	int
	setDouble( double d ) {

		sqlite3_result_double( _pCtx, d );
		return SQLITE_OK;
	}

	int
	setText( const char *txt, int size, void( *xDel )(void *) ) {

		sqlite3_result_text( _pCtx, txt, size, xDel );
		return SQLITE_OK;
	}

	int
	setText16( const void *txt16, int size, void( *xDel )(void *) ) {

		sqlite3_result_text16( _pCtx, txt16, size, xDel );
		return SQLITE_OK;
	}

	int
	setZeroblob( int n ) {

		sqlite3_result_zeroblob( _pCtx, n );
		return SQLITE_OK;
	}

	int
	setBlob( const void* z, int n, void( *xDel )(void *) ) {

		sqlite3_result_blob( _pCtx, z, n, xDel );
		return SQLITE_OK;
	}
};


struct SqliteTargetBind {

	sqlite3_stmt *_pStmt;
	int _param;

	SqliteTargetBind( sqlite3_stmt *pStmt, int param )
	: _pStmt( pStmt ), _param( param ) {
	}

	sqlite3 *
	getDatabase() {

		return sqlite3_db_handle( _pStmt );
	}

	int
	setNull() {

		return sqlite3_bind_null( _pStmt, _param );
	}

	int
	setInt64( sqlite3_int64 i ) {

		return sqlite3_bind_int64( _pStmt, _param, i );
	}

	int
	setDouble( double d ) {

		return sqlite3_bind_double( _pStmt, _param, d );
	}

	int
	setText( const char *txt, int size, void( *xDel )(void *) ) {

		return sqlite3_bind_text( _pStmt, _param, txt, size, xDel );
	}

	int
	setText16( const void *txt16, int size, void( *xDel )(void *) ) {

		return sqlite3_bind_text16( _pStmt, _param, txt16, size, xDel );
	}

	int
	setZeroblob( int n ) {

		return sqlite3_bind_zeroblob( _pStmt, _param, n );
	}

	int
	setBlob( const void* z, int n, void( *xDel )(void *) ) {

		return sqlite3_bind_blob( _pStmt, _param, z, n, xDel );
	}
};


template <class T>
ALWAYS_INLINE bool
jsvalToSqlite( JSContext *cx, T sqliteTarget, IN JS::HandleValue val ) {

	int sqliteStatus;

	if ( val.isNullOrUndefined() ) {

		// http://www.sqlite.org/nulls.html
		sqliteStatus = sqliteTarget.setNull();
	} else if ( val.isBoolean() ) {

		sqliteStatus = sqliteTarget.setInt64( val.toBoolean() ? 1 : 0 );
	} else if ( val.isInt32() ) {

		sqliteStatus = sqliteTarget.setInt64( val.toInt32() );
	} else if ( val.isDouble() ) {

//		if ( jl::isInBounds<sqlite3_int64, double>( val.toDouble() ) )
//			sqliteStatus = sqliteTarget.setInt( (sqlite3_int64)val.toDouble());
//		else
			sqliteStatus = sqliteTarget.setDouble( val.toDouble() );
	} else if ( val.isString() ) {

		JS::AutoCheckCannotGC nogc;
		jl::BufString buf;
		JL_CHK( jl::getValue( cx, val, &buf ) );
		// beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
		if ( buf.isWide() )
			sqliteStatus = sqliteTarget.setText16( buf.toData<const jschar*>(), buf.length() * buf.charSize(), SQLITE_STATIC );
		else
			sqliteStatus = sqliteTarget.setText( buf.toData<const char*>(), buf.length(), SQLITE_STATIC );
	} else if ( jl::isData( cx, val ) ) {

		JS::AutoCheckCannotGC nogc;
		jl::BufString buf;
		JL_CHK( jl::getValue( cx, val, &buf ) );
		// beware: assume that the string is not GC while SQLite is using it. else use SQLITE_TRANSIENT
		size_t len = buf.length();
		if ( len == 0 )
			sqliteStatus = sqliteTarget.setZeroblob( 0 );
		else
			sqliteStatus = sqliteTarget.setBlob( buf.toBytes(), len, SQLITE_STATIC );
	} else {

		//jl::getPrimitive( cx, val, val );
		JL_ERR( E_RETURNVALUE, E_DATATYPE, E_NOTSUPPORTED );
	}

	if ( sqliteStatus != SQLITE_OK )
		JL_CHK( SqliteThrowError( cx, sqliteTarget.getDatabase() ) );

	return true;
	JL_BAD;
}



ALWAYS_INLINE bool
SqliteToJsval( JSContext *cx, sqlite3_value *value, OUT JS::MutableHandleValue rval ) {

	switch ( sqlite3_value_type( value ) ) {

	case SQLITE_INTEGER:
		JL_CHK( jl::setValue( cx, rval, sqlite3_value_int64( value ) ) );
		break;
	case SQLITE_FLOAT:
		JL_CHK( jl::setValue( cx, rval, sqlite3_value_double( value ) ) );
		break;
	case SQLITE_TEXT:
		// see sqlite3_value_text16le()
		JL_CHK( jl::setValue( cx, rval, jl::strSpec( (const jschar*)sqlite3_value_text16( value ), sqlite3_value_bytes16( value ) / sizeof( jschar ) ) ) );
		break;
	case SQLITE_BLOB:
		JL_CHK( BlobCreateCopy( cx, sqlite3_value_blob( value ), sqlite3_value_bytes( value ), rval ) );
		break;
	case SQLITE_NULL:
		rval.setNull();
		break;
	default:
		JL_ERR( E_DATATYPE, E_NOTSUPPORTED );
	}
	return true;
	JL_BAD;
}
