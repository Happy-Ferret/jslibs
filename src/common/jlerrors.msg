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

DEF( ERRNO_1,        "(error number: %d)", JSEXN_NONE )
DEF( NUM_1,          "%d", JSEXN_NONE )
DEF( STR_1,          "%s", JSEXN_NONE )
DEF( NAME_1,         "'%s'", JSEXN_NONE )
DEF( COMMENT_1,      "(%s)", JSEXN_NONE )
DEF( COMMENT_2,      "(%s %s)", JSEXN_NONE )
DEF( COMMENT_BEGIN,  "(", JSEXN_NONE )
DEF( COMMENT_END,    ")", JSEXN_NONE )
DEF( INTERVAL_NUM_2, "%d to %d", JSEXN_NONE )
DEF( INTERVAL_STR_2, "%s to %s", JSEXN_NONE )
DEF( DETAILS,        ":", JSEXN_NONE )
DEF( SEP,            ",", JSEXN_NONE )
DEF( OR,             "or", JSEXN_NONE )

DEF( DATA,           "data", JSEXN_NONE )
DEF( DATATYPE,       "data type", JSEXN_TYPEERR )
DEF( DATASIZE,       "data size", JSEXN_RANGEERR )
DEF( ARGC,           "number of arguments", JSEXN_RANGEERR )
DEF( ARG,            "argument", JSEXN_NONE )
DEF( PARAM,          "parameter", JSEXN_NONE )
DEF( PARAMTYPE,      "type of the parameter", JSEXN_NONE )
DEF( VALUE,          "value", JSEXN_NONE )
DEF( VALUELENGTH,    "value length", JSEXN_NONE )
DEF( ARRAYLENGTH,    "length of the array", JSEXN_RANGEERR )
DEF( ARGVALUE,       "value of argument", JSEXN_NONE )
DEF( RETURNVALUE,    "return value", JSEXN_NONE )
DEF( THISOBJ,        "current object", JSEXN_NONE )
DEF( OBJ,            "object", JSEXN_NONE )
DEF( CLASS,          "class", JSEXN_NONE )
DEF( PROP,           "property", JSEXN_NONE )
DEF( API,            "API", JSEXN_NONE )
DEF( FUNC,           "function", JSEXN_NONE )
DEF( CALL,           "function call", JSEXN_NONE )
DEF( FILE,           "file", JSEXN_NONE )
DEF( FILENAME,       "filename", JSEXN_NONE )
DEF( MODULE,         "module", JSEXN_NONE )
DEF( LOCATION,       "location", JSEXN_NONE )
DEF( SCRIPT,         "script", JSEXN_NONE )
DEF( HOST,           "jslibs host", JSEXN_NONE )
DEF( JSLIBS,         "jslibs", JSEXN_NONE )
DEF( INTERFACE,      "interface", JSEXN_NONE )
DEF( THISOPERATION,  "this operation", JSEXN_NONE )
DEF( LIB,            "library", JSEXN_NONE )
DEF( OS,             "operating system", JSEXN_INTERNALERR )

//
DEF( RANGE,          "must be between", JSEXN_RANGEERR )
DEF( MIN,            "must be >=", JSEXN_RANGEERR )
DEF( MAX,            "must be <=", JSEXN_RANGEERR )
DEF( EQUALS,         "must be equal to", JSEXN_RANGEERR )
DEF( LENGTH,         "must have a length of", JSEXN_RANGEERR )
DEF( PRECISION,      "has lost precision", JSEXN_RANGEERR )
DEF( TYPE,           "type must be", JSEXN_TYPEERR )
DEF( INSTANCE,       "must be an instance of", JSEXN_TYPEERR )
DEF( INHERIT,        "must inherit from", JSEXN_TYPEERR )
DEF( CONVERT,        "fails to convert to", JSEXN_TYPEERR )
DEF( DEFINED,        "must be defined", JSEXN_TYPEERR )
DEF( NOTFOUND,       "cannot be found", JSEXN_REFERENCEERR )
DEF( NOTSUPPORTED,   "is not supported", JSEXN_INTERNALERR )
DEF( CONSTRUCT,      "must be constructed", JSEXN_TYPEERR )
DEF( NOTCONSTRUCT,   "cannot be constructed", JSEXN_TYPEERR )
DEF( CREATE,         "cannot be created", JSEXN_INTERNALERR )
DEF( INIT,           "fails to initialize", JSEXN_INTERNALERR )
DEF( FIN,            "fails to finalize", JSEXN_INTERNALERR )
DEF( INVALID,        "is invalid", JSEXN_ERR )
DEF( ENCODING,       "encoding", JSEXN_ERR )
DEF( FORMAT,         "has an unsupported format", JSEXN_ERR )
DEF( VERSION,        "has an invalid version", JSEXN_ERR )
DEF( ACCESS,         "is not accessible", JSEXN_TYPEERR )
DEF( READ,           "cannot be read", JSEXN_TYPEERR )
DEF( WRITE,          "cannot be write", JSEXN_TYPEERR )
DEF( FAILURE,        "failed", JSEXN_ERR )
DEF( OPEN,           "is open", JSEXN_TYPEERR )
DEF( CLOSED,         "is closed", JSEXN_TYPEERR )
DEF( DISABLED,       "is disabled", JSEXN_TYPEERR )
DEF( NOTINIT,        "is not initialized", JSEXN_TYPEERR )
DEF( OPERATION,      "fails to perform the operation", JSEXN_ERR )
DEF( STATE,          "state is invalid", JSEXN_ERR )
DEF( COMPILE,        "cannot be compiled", JSEXN_ERR )
DEF( INTERNAL,       "encountered an internal failure", JSEXN_INTERNALERR )
DEF( IGNORED,        "will be ignored", JSEXN_TYPEERR )
DEF( REQUIRED,       "is required", JSEXN_TYPEERR )
DEF( NOTIMPLEMENTED, "is not implemented", JSEXN_INTERNALERR )

//
DEF( TY_UNDEFINED,     "undefined", JSEXN_TYPEERR )
DEF( TY_PRIMITIVE,     "a non-object", JSEXN_TYPEERR )
DEF( TY_OBJECT,        "an object", JSEXN_TYPEERR )
DEF( TY_STRINGDATA,    "a string or data", JSEXN_TYPEERR )
DEF( TY_STRING,        "a string", JSEXN_TYPEERR )
DEF( TY_DATA,          "data", JSEXN_TYPEERR )
DEF( TY_CHAR,          "a 1-char string", JSEXN_TYPEERR )
DEF( TY_INTEGER,       "an integer", JSEXN_TYPEERR )
DEF( TY_INTEGERDOUBLE, "integer < 2^53 and > -2^53", JSEXN_TYPEERR )
DEF( TY_BOOLEAN,       "a boolean", JSEXN_TYPEERR )
DEF( TY_NUMBER,        "a number", JSEXN_TYPEERR )
DEF( TY_ARRAY,         "an array", JSEXN_TYPEERR )
DEF( TY_NARRAY_1,      "a %d-element array", JSEXN_TYPEERR )
DEF( TY_FUNC,          "a function", JSEXN_TYPEERR )
DEF( TY_VECTOR,        "a vector", JSEXN_TYPEERR )
DEF( TY_NVECTOR_1,     "a %d-element vector", JSEXN_TYPEERR )
