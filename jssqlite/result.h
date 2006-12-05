DECLARE_CLASS( Result )

#define SLOT_RESULT_DATABASE 0

JSBool SqliteColumnToJsval( JSContext *cx, sqlite3_stmt *pStmt, int iCol, jsval *rval );
