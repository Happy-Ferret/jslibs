DECLARE_CLASS( SqliteError );

#define SLOT_SQLITE_ERROR_CODE 0
#define SLOT_SQLITE_ERROR_TEXT 1

JSBool SqliteThrowError( JSContext *cx, int errorCode, const char *errorMsg );
