#define JS_HAS_RESERVED_JAVA_KEYWORDS   1
#define JS_HAS_RESERVED_ECMA_KEYWORDS   1

#define JS_VERSION 180

#define JS_HAS_STR_HTML_HELPERS 0       /* has str.anchor, str.bold, etc. */
#define JS_HAS_PERL_SUBSTR      1       /* has str.substr */
#define JS_HAS_OBJ_PROTO_PROP   1       /* has o.__proto__ etc. */
#define JS_HAS_OBJ_WATCHPOINT   1       /* has o.watch and o.unwatch */
#define JS_HAS_EVAL_THIS_SCOPE  1       /* Math.eval is same as with (Math) */
#define JS_HAS_SHARP_VARS       1       /* has #n=, #n# for object literals */
#define JS_HAS_SCRIPT_OBJECT    1       /* has (new Script("x++")).exec() */
#define JS_HAS_XDR              1       /* has XDR API and internal support */
#define JS_HAS_XDR_FREEZE_THAW  0       /* has XDR freeze/thaw script methods */
#define JS_HAS_TOSOURCE         1       /* has Object/Array toSource method */
#define JS_HAS_DEBUGGER_KEYWORD 1       /* has hook for debugger keyword */
#define JS_HAS_CATCH_GUARD      1       /* has exception handling catch guard */
#define JS_HAS_SPARSE_ARRAYS    0       /* array methods preserve empty elems */
#define JS_HAS_GETTER_SETTER    1       /* has JS2 getter/setter functions */
#define JS_HAS_UNEVAL           1       /* has uneval() top-level function */
#define JS_HAS_CONST            1       /* has JS2 const as alternative var */
#define JS_HAS_FUN_EXPR_STMT    1       /* has function expression statement */
#define JS_HAS_LVALUE_RETURN    1       /* has o.item(i) = j; for native item */
#define JS_HAS_NO_SUCH_METHOD   1       /* has o.__noSuchMethod__ handler */
#define JS_HAS_XML_SUPPORT      1       /* has ECMAScript for XML support */
#define JS_HAS_ARRAY_EXTRAS     1       /* has indexOf and Lispy extras */
#define JS_HAS_GENERATORS       1       /* has yield in generator function */
#define JS_HAS_BLOCK_SCOPE      1       /* has block scope via let/arraycomp */
#define JS_HAS_DESTRUCTURING    1       /* has [a,b] = ... or {p:a,q:b} = ... */
#define JS_HAS_GENERATOR_EXPRS  1       /* has (expr for (lhs in iterable)) */
#define JS_HAS_EXPR_CLOSURES    1       /* has function (formals) listexpr */

/* Feature-test macro for evolving destructuring support. */
#define JS_HAS_DESTRUCTURING_SHORTHAND 1

/*
jsscript.c
jsscript.c(897) : error C2198: 'js_NewObject' : too few arguments for call
make[1]: *** [WINNT5.1_OPT.OBJ/jsscript.obj] Error 2
make[1]: Leaving directory `/cygdrive/d/Franck/Mes documents/DEV/my_projects/jslibs/js/src'
make: *** [all] Error 2

workaround: #undef JS_HAS_SCRIPT_OBJECT

wait fix for JS_HAS_XDR_FREEZE_THAW 1
*/