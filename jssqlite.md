<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jssqlite/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jssqlite/qa.js) -
# jssqlite module #




---

## class jssqlite::Database ##
- [top](#jssqlite_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssqlite/database.cpp?r=2555) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>( [[.md](.md)fileName] [[.md](.md), flags] )
> > Creates a new Database object.
> > ##### arguments: #####
      1. <sub>string</sub> _fileName_: is the file name of the database, or an empty string for a temporary database.
> > > > If omitted or _undefined_, an in-memory database is created.
      1. <sub>enum</sub> _flags_: can be one of the following constant:
        * <b><code>READONLY</code></b>
        * <b><code>READWRITE</code></b>
        * <b><code>CREATE</code></b>
        * <b><code>DELETEONCLOSE</code></b>
        * <b><code>EXCLUSIVE</code></b>
        * <b><code>MAIN_DB</code></b>
        * <b><code>TEMP_DB</code></b>
        * <b><code>TRANSIENT_DB</code></b>
        * <b><code>MAIN_JOURNAL</code></b>
        * <b><code>TEMP_JOURNAL</code></b>
        * <b><code>SUBJOURNAL</code></b>
        * <b><code>MASTER_JOURNAL</code></b>

> > ##### example: #####
```
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
```

### Methods ###

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the database and all its opened [Result](Result.md) objects.
> > ##### note: #####
> > > It is recommended to close all [Result](Result.md) objects before closing the database.

#### <font color='white' size='1'><b>Query</b></font> ####

> <sub>Result</sub> <b>Query</b>( sqlStr [[.md](.md), map ] )
> > Evaluates a SQL string and returns a Result object ready to be executed.
> > ##### arguments: #####
      1. <sub>string</sub> _sqlStr_:
      1. <sub>Object</sub> _map_: _map_ is bind to the SQL statement and can be access using '@' char ( see. **Exec** ). If you create new properties on the [Result](Result.md) object, you can access then in the _sqlStr_ using ':' char. '?' allows you access the _map_ as an array ( see examples ).
> > > > ##### example 1: #####
```
    var res = db.Query('SELECT :test1 + 5');
    res.test1 = 123;
    Print( res.Row().toSource() ); // Prints: [128]
```
> > > > ##### example 2: #####
```
    var res = db.Query('SELECT @test2 + 5', {test2:6});
    Print( res.Row().toSource() ); // Prints: [11]
```
> > > > ##### example 3: #####
```
    var res = db.Query('SELECT ? + ?', [ 4, 5 ]);
    Print( res.Row().toSource() ); // Prints: [9]
```

> > ##### return value: #####
> > > A new Result object.

> > ##### <font color='red'>beware</font>: #####
> > > There are some limitation in variable bindings. For example, they cannot be used to specify a table name.
> > > `db.Query('SELECT * FROM ?', ['myTable']);` will failed with this exception: `SQLite error 1: near "?": syntax error`

> > ##### example 1: #####
```
  var result = db.Query('SELECT name FROM table WHERE id=:userId' );
  result.userId = 1341;
  Print( result.Col(0) );
```
> > ##### example 2: #####
```
  var result = db.Query('SELECT name FROM table WHERE id=@userId', { userId:1341 } );
  Print( result.Col(0) );
```
> > ##### example 3: #####
```
  var result = db.Query('SELECT ? FROM table WHERE id=?', ['name', 1341] ); // array-like objects {0:'name', 1:1341, length:2} works too.
  Print( result.Col(0) );
```

#### <font color='white' size='1'><b>Exec</b></font> ####

> <sub>value</sub> <b>Exec</b>( sqlStr [[.md](.md), map ] )
> > Evaluates a SQL string and return the result in one operation.
> > ##### arguments: #####
      1. <sub>string</sub> _sqlStr_: is the SQL statement.
      1. <sub>Object</sub> _map_: if given, this argument is bind (as a key:value variable map) to the SQL statement.
> > > > ##### example: #####
```
    db.Exec('PRAGMA user_version = @ver', { ver:5 } );
```

> > ##### return value: #####
> > > returns the first line and first column of the result.

> > ##### details: #####
> > > [sqlite documentation](http://www.sqlite.org/capi3ref.html#sqlite3_bind_blob)

> > ##### example: #####
```
  var version = db.Exec('PRAGMA user_version');
  db.Exec('PRAGMA user_version = 5');
```

### Properties ###

#### <font color='white' size='1'><b>lastInsertRowid</b></font> ####

> <b>lastInsertRowid</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the rowid of the most recent successful INSERT into the database from the database connection shown in the first argument. If no successful inserts have ever occurred on this database connection, zero is returned.
> > ##### details: #####
> > > [sqlite documentation](http://www.sqlite.org/capi3ref.html#sqlite3_last_insert_rowid)

#### <font color='white' size='1'><b>changes</b></font> ####

> <b>changes</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > is the number of database rows that were changed or inserted or deleted by the most recently completed SQL statement on the connection specified by the first parameter. Only changes that are directly specified by the INSERT, UPDATE, or DELETE statement are counted.
> > ##### details: #####
> > > [sqlite documentation](http://www.sqlite.org/capi3ref.html#sqlite3_changes)

### Static Properties ###

#### <font color='white' size='1'><b>version</b></font> ####

> <b>version</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Hold the current version of the database engine.

#### <font color='white' size='1'><b>memoryUsed</b></font> ####

> <b>memoryUsed</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the amount of memory currently checked out.

### Remarks ###
  * Add SQL functions implemented in JavaScript.
> > Any function properties stored to a [Database](Database.md) object can be used in the SQL string.
> > ##### example: #####
```
  var db = new Database('myDatabase');
  db.multBy10 = function(a) { return a * 10 }
  Print( db.Exec('SELECT multBy10(123)') ); // prints: 1230
```

### Note ###

> jslibs Blob object is interpreted as a blob database type.

### Examples ###
> ##### example 1: #####
```
 Print('database version: ' + Database.version ,'\n' );

 var obj = { foo:Blob('qqwe\00\00fv1234') };
 Print( 'testFunc = ' + db.Exec('SELECT length(:foo)', obj  ) ,'\n' );
```
> ##### example 2: #####
```
 LoadModule('jsstd');
 LoadModule('jssqlite');

 try {

  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  db.Exec('insert into t1 (a,b,c) values (5,6,7)');
  db.Exec('insert into t1 (a,b,c) values (2,3,4)');
  db.Exec('insert into t1 (a,b,c) values ("a","b","c")');

  var res = db.Query('SELECT a,c from t1');

  Print( res.Row().toSource(), '\n' );
  Print( res.Row().toSource(), '\n' );
  Print( res.Row().toSource(), '\n' );

  } catch ( ex if ex instanceof SqliteError ) {

   Print( 'SQLite error '+ex.code+': '+ex.text+'\n' );
  }
```


---

## class jssqlite::Result ##
- [top](#jssqlite_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jssqlite/result.cpp?r=2555) -
> A Result object is used to store a compiled SQL statement ready to be executed.<br />
> When a statement has been prepared with Database.**Query** function, you need to execute it ( with **Step** function ) before any data can be read.
> However, some properties (like **columnCount**, ... ) can be read before the first **Step** has been done.<br />
> A result has the ability to be iterated through a  _for each..in_  loop (_for..in_ loop is note supported).
> ##### example 1: #####
```
  var db = new Database(); // in-memory database
  db.Exec('create table t1 (name,value);');
  db.Exec('insert into t1 (name,value) values ("red","#F00")');
  db.Exec('insert into t1 (name,value) values ("green","#0F0")');
  db.Exec('insert into t1 (name,value) values ("blue","#00F")');
  
  for each ( row in db.Query('SELECT * from t1') )
   Print( row.name + ' = ' + row.value, '\n' );
```
> > prints:
```
  red = #F00
  green = #0F0
  blue = #00F
```


> ##### example 2: #####
```
  Print( [ color.name for each ( color in db.Query('SELECT * from t1') ) ] ); // prints: red,green,blue
```

> ##### note: #####
> > You cannot construct this class.

### Methods ###

#### <font color='white' size='1'><b>Close</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Close</b>()
> > Close the current Result object.

#### <font color='white' size='1'><b>Step</b></font> ####

> <sub>boolean</sub> <b>Step</b>()
> > Executes one step in the previously evaluated SQL statement.
> > ##### return value: #####
> > > returns true if another row is ready. false if the last line has been reached.

#### <font color='white' size='1'><b>Col</b></font> ####

> <sub>value</sub> <b>Col</b>( colIndex )
> > Returns the current value of the _colIndex_ <sup>th</sup> column.
> > ##### arguments: #####
      1. <sub>integer</sub> _colIndex_

#### <font color='white' size='1'><b>Row</b></font> ####

> <sub>value</sub> <b>Row</b>( [[.md](.md)namedRows = false] )
> > Executes one step of the the current SQL statement and returns the resulting row of data.
> > ##### arguments: #####
      1. <sub>boolean</sub> _namedRows_: if true, the function returns an objet containing {columnName:value} pair. else it returns an array of value.
> > ##### note: #####
> > > The **Step** function is internally called before each **Row** call.

#### <font color='white' size='1'><b>Reset</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Reset</b>()
> > Resets the current Result object to its initial state.

### Properties ###

#### <font color='white' size='1'><b>columnCount</b></font> ####

> <sub>integer</sub> <b>columnCount</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Hold the number of columns of the current [Result](Result.md)

#### <font color='white' size='1'><b>columnNames</b></font> ####

> <sub>Array</sub> <b>columnNames</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Hold an <sub>Array</sub> that contain the index:name of the columns.
> > ##### example: #####
```
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnNames.toSource(), '\n' ); // prints: ["a", "c"]
```

#### <font color='white' size='1'><b>columnIndexes</b></font> ####

> <sub>Object</sub> <b>columnIndexes</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Hold an <sub>Object</sub> that contain the name:index of the columns.
> > ##### example: #####
```
  var db = new Database();
  db.Exec('create table t1 (a,b,c);');
  var res = db.Query('SELECT a,c from t1');
  Print( res.columnIndexes.toSource(), '\n' ); // prints: ({a:0, c:1})
```

#### <font color='white' size='1'><b>expired</b></font> ####

> <sub>boolean</sub> <b>expired</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)   ![http://jslibs.googlecode.com/svn/wiki/deprecated.png](http://jslibs.googlecode.com/svn/wiki/deprecated.png)
> > Indicates if the SQL statement must be re-evaluated.


---

## class jssqlite::SqliteError ##
- [top](#jssqlite_module.md) -


> Its aim is to be throw as an exception on any SQLite runtime error.
> ##### note: #####
> > You cannot construct this class.<br />

### Properties ###

#### <font color='white' size='1'><b>code</b></font> ####

> <sub>integer</sub> <b>code</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

#### <font color='white' size='1'><b>text</b></font> ####
> <sub>string</sub> <b>text</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)

### Exemple ###
```
try {

  db.Exec('yfiqwygqiwye'); // generate an error

} catch ( ex if ex instanceof SqliteError ) {

   Print( 'SqliteError: ' + ex.text + '('+ex.code+')', '\n' );
} catch( ex ) {

   throw ex;
}
```


---

- [top](#jssqlite_module.md) - [main](JSLibs.md) -
