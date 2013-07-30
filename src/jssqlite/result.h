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

DECLARE_CLASS( Result )

#define SLOT_RESULT_DATABASE 0
#define SLOT_RESULT_BINDING_UP_TO_DATE 1
#define SLOT_RESULT_QUERY_ARGUMENT_OBJECT 2

JSBool SqliteToJsval( JSContext *cx, sqlite3_value *value, OUT jsval &rval );
JSBool SqliteSetupBindings( JSContext *cx, sqlite3_stmt *pStmt, JSObject *argObj, JSObject *curObj );
