DECLARE_CLASS( Result )

#define SLOT_RESULT_DATABASE 0
#define SLOT_RESULT_BINDING_UP_TO_DATE 1

JSBool SqliteToJsval( JSContext *cx, sqlite3_value *value, jsval *rval );
JSBool SqliteSetupBindings( JSContext *cx, sqlite3_stmt *pStmt, JSObject *objAt, JSObject *objColon );
JSBool SqliteColumnToJsval( JSContext *cx, sqlite3_stmt *pStmt, int iCol, jsval *rval );
