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

DECLARE_CLASS( WinError );

#define SLOT_WIN_ERROR_CODE_HI 0
#define SLOT_WIN_ERROR_CODE_LO 1

NEVER_INLINE JSBool FASTCALL
WinNewError( JSContext *cx, DWORD errorCode, jsval *rval );

NEVER_INLINE JSBool FASTCALL
WinThrowError( JSContext *cx, DWORD errorCode );
