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

#include "queue.h"

DECLARE_CLASS( Database )

#define SLOT_SQLITE_DATABASE_STATEMENT_STACK 0


extern jl::Queue *dbContextList;

struct DbContext {
	sqlite3 *db;
	JSContext *cx;
	JSObject *obj;
};

DbContext* AddDbContext(sqlite3 *db);
DbContext* GetDbContext(sqlite3 *db);
void RemoveDbContext(sqlite3 *db);

