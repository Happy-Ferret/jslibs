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

// exnType:
//  JSEXN_NONE = -1
//  JSEXN_ERR
//  JSEXN_INTERNALERR
//  JSEXN_EVALERR      // regarding the eval function.
//  JSEXN_RANGEERR     // when a number is not within the correct range allowed.
//  JSEXN_REFERENCEERR // when a non-existent variable is referenced. A ReferenceError is thrown when trying to dereference a variable that has not been declared.
//  JSEXN_SYNTAXERR    // when trying to interpret syntactically invalid code. A SyntaxError is thrown when the JavaScript engine encounters tokens or token order that does not conform to the syntax of the language when parsing code.
//  JSEXN_TYPEERR      // when a value is not of the expected type.
//  JSEXN_URIERR       // when a malformed URI is encountered.


// more generic errors/warnings
JLMSG_DEF( JLSMSG_PROGRAM_STOPPED,        JSEXN_NONE, "the program has been stopped", 0)
JLMSG_DEF( JLSMSG_NEED_CONSTRUCT,         JSEXN_TYPEERR, "object must be constructed", 0)
JLMSG_DEF( JLSMSG_NOT_IMPLEMENTED,        JSEXN_INTERNALERR, "not implemented", 0)
JLMSG_DEF( JLSMSG_OS_ERROR,               JSEXN_INTERNALERR, "OS: {0}", 1)
JLMSG_DEF( JLSMSG_RUNTIME_ERROR,          JSEXN_INTERNALERR, "runtime error: {0}", 1)
JLMSG_DEF( JLSMSG_LOGIC_ERROR,            JSEXN_ERR, "logic error: {0}", 1)
JLMSG_DEF( JLSMSG_RANGE_ERROR,            JSEXN_RANGEERR, "{0}", 1)
JLMSG_DEF( JLSMSG_TYPE_ERROR,             JSEXN_TYPEERR, "{0}", 1)
JLMSG_DEF( JLSMSG_INVALID_RESOURCE,       JSEXN_INTERNALERR, "object state is invalid", 0)

// specialized errors/warnings
JLMSG_DEF( JLSMSG_TOO_FEW_ARGUMENTS,      JSEXN_RANGEERR, "not enough arguments (min is {0})", 1)
JLMSG_DEF( JLSMSG_TOO_MANY_ARGUMENTS,     JSEXN_RANGEERR, "too many arguments (max is {0})", 1)
JLMSG_DEF( JLSMSG_INVALID_ARGUMENT_COUNT, JSEXN_RANGEERR, "invalid argument count (need {0})", 1)

JLMSG_DEF( JLSMSG_VALUE_OUTOFRANGE,       JSEXN_RANGEERR, "value out of range {0}", 1)
JLMSG_DEF( JLSMSG_LOSS_OF_PRECISION,      JSEXN_ERR, "loss of data or precision", 0)
JLMSG_DEF( JLSMSG_EXPECT_TYPE,            JSEXN_TYPEERR, "{0} is expected", 1)
JLMSG_DEF( JLSMSG_INVALID_INSTANCE,       JSEXN_TYPEERR, "{0} instance is expected", 1)
JLMSG_DEF( JLSMSG_FAIL_TO_CONVERT_TO,     JSEXN_INTERNALERR, "fail to convert to {0}", 1)
JLMSG_DEF( JLSMSG_NEED_DEFINED,           JSEXN_TYPEERR, "value must be defined", 0)
JLMSG_DEF( JLSMSG_INVALID_INHERITANCE,    JSEXN_TYPEERR, "object must be a {0}", 1)
JLMSG_DEF( JLSMSG_INVALIDATED_OBJECT,     JSEXN_INTERNALERR, "{0} object invalidated", 1)
JLMSG_DEF( JLSMSG_IMMUTABLE_OBJECT,       JSEXN_TYPEERR, "{0} is read-only", 1)



/* errors to manage:

- Internal error: <what>.
- Missing <something> (an identified thing is missing to continue properly)
- Failed to <do something> ( the underlying native or spidermonkey API failed )
- Unable to <do something>, <why> ( logical error, misuse of jslibs API, Object cannot be constructed )
- Invalid operation: <what> ( API misuse )

- Invalid or uninitialized object (Method called on the wrong object.)
- Invalid argument count. ( misuse of jslibs API )
- Invalid argument value. ( misuse of jslibs API )
- Invalid data type <type> ( invalid type, Array expected )
- conversion failed / failed to convert to <type> ( Unable to convert to boolean )
- Value out of range: <info?>
- Limit reach: <what>

- Object cannot be constructed.
- Construction form is required.

- Not implemented yet.
- Available in Debug mode only.
*/
