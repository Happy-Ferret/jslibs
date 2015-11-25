<b>If something seems wrong or incomplete, please enter <a href='#commentform.md'>a comment at the bottom of this page</a>.</b><br /><br />- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsdebug/) - [main](JSLibs.md) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsdebug/qa.js) -
# jsdebug module #

> Various debug tools.




---

## jsdebug static members ##
- [top](#jsdebug_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsdebug/static.cpp?r=2557) -

### Static functions ###

#### <font color='white' size='1'><b>DumpStats</b></font> ####
> <b>DumpStats</b>( [[.md](.md) filename [[.md](.md), type [[.md](.md), ...] ] ] )
> > type: 'gc' | 'arena' | 'atom' | 'global' | variable

#### <font color='white' size='1'><b>DumpHeap</b></font> ####

> <b>DumpHeap</b>( [[.md](.md) filename [[.md](.md), startThing [[.md](.md), thingToFind [[.md](.md), maxDepth [[.md](.md), thingToIgnore] ] ] ] ] )
> ##### note: #####
> > This function in only available in DEBUG mode.

#### <font color='white' size='1'><b>TraceGC</b></font> ####

> <b>TraceGC</b>( [[.md](.md) filename ] )
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>currentMemoryUsage</b></font> ####

> <b>currentMemoryUsage</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>peakMemoryUsage</b></font> ####

> <b>peakMemoryUsage</b>
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>privateMemoryUsage</b></font> ####

> <b>privateMemoryUsage</b>
> > <b><font color='red'>TBD</font></b>

### Static properties ###

#### <font color='white' size='1'><b>gcNumber</b></font> ####

> <b>gcNumber</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Number of times when GC was invoked.

#### <font color='white' size='1'><b>gcMallocBytes</b></font> ####

> <b>gcMallocBytes</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the amount of bytes mallocated by the JavaScript engine. It is incremented each time the JavaScript engine allocates memory.

#### <font color='white' size='1'><b>gcBytes</b></font> ####

> <b>gcBytes</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > It is the total amount of memory that the GC uses now and right after the last GC.

#### <font color='white' size='1'><b>gcZeal</b></font> ####

> <b>gcZeal</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Enable GC zeal, a testing and debugging feature that helps find GC-related bugs in JSAPI applications.
> > Level of garbage collection: 0 for normal, 1 for very frequent GC, 2 for extremely frequent GC.

> ##### note: #####
> > This function in only available in DEBUG mode.

#### <font color='white' size='1'><b>DisableJIT</b></font> ####

> <sub>integer</sub> <b>DisableJIT</b>()
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>GetObjectPrivate</b></font> ####

> <sub>integer</sub> <b>GetObjectPrivate</b>()
> > <b><font color='red'>TBD</font></b>

#### <font color='white' size='1'><b>scriptFilenameList</b></font> ####

> <sub>Array</sub> <b>scriptFilenameList</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the list of all detected and active scripts.

#### <font color='white' size='1'><b>currentFilename</b></font> ####

> <sub>Array</sub> <b>currentFilename</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the filename of the script being executed.
> > ##### note: #####
> > > The current filename is also available using: `StackFrameInfo(stackSize-1).filename`

#### <font color='white' size='1'><b>GetActualLineno</b></font> ####

> <sub>integer</sub> | _undefined_ <b>GetActualLineno</b>( filename, lineno )
> <sub>integer</sub> | _undefined_ <b>GetActualLineno</b>( function, relativeLineno )
> > Transform a random line number into an actual line number or _undefined_ if the file number cannot be reached.
> > ##### example: #####
```
  1.  var i = 0;
  2.
  3.  i++;

  GetActualLineno('test.js', 2); // returns: 3
  GetActualLineno('nofile.js', 2); // returns: undefined
```

#### <font color='white' size='1'><b>stackSize</b></font> ####

> <sub>Array</sub> <b>stackSize</b>  ![http://jslibs.googlecode.com/svn/wiki/readonly.png](http://jslibs.googlecode.com/svn/wiki/readonly.png)
> > Is the number of stack frames. 0 is the older stack frame index. The current stack frame index is (stackSize-1).

#### <font color='white' size='1'><b>StackFrameInfo</b></font> ####

> <sub>Object</sub> <b>StackFrameInfo</b>( frameLevel )
> > Returns an object that describes the given stack frame index.
> > 0 is the older stack frame index. The current (last) stack frame index is (stackSize-1). See Locate() function for more details. <br />
> > The object contains the following properties:
    * filename: filename of the script being executed.
    * lineno: line number of the script being executed.
    * callee: function being executed.
    * baseLineNumber: first line number of the function being executed.
    * lineExtent: number of lines of the function being executed.
    * scope: scope object.
    * this: this object.
    * argv: argument array of the function being executed.
    * rval: return value.
    * isNative: the frame is running native code.
    * isConstructing: frame is for a constructor invocation.
    * isEval: frame for eval.
    * isAssigning: a complex op is currently assigning to a property.

#### <font color='white' size='1'><b>EvalInStackFrame</b></font> ####

> <sub>Object</sub> <b>EvalInStackFrame</b>( code, frameLevel )
> Evaluates code in the given stack frame.
> 0 is the older stack frame index. The current (last) stack frame index is (stackSize-1). See Locate() function for more details.

#### <font color='white' size='1'><b>Locate</b></font> ####
> <sub>Object</sub> <b>Locate</b>( [[.md](.md)frameIndex] );
> > Returns the current script name and line number.
> > 0 is the older (the first) stack frame index. The current stack frame index is Locate() or Locate(stackSize-1).
> > Negative numbers are interpreted as starting from the current stack frame.<br />
> > The function returns _undefined_ if the given stack frame is not defined.
> > ##### note: #####
> > > `Locate(-1) == Locate(stackSize-2)`

> > ##### example: #####
> > (file debug.js)
```
   1 LoadModule('jsstd');
   2 LoadModule('jsdebug');
   3
   4 function test2() {
   5
   6 Print( stackSize, ' - ', Locate(stackSize-1), '\n' ); // prints: 3 - debug.js,6
   7 Print( Locate(-1), ' - ', Locate(-2), ' - ', Locate(-3), '\n' ); // prints: debug.js,13 - debug.js,16 - undefined
   8 Print( Locate(0), ' - ', Locate(1), ' - ', Locate(2), '\n' ); // prints: debug.js,16 - debug.js,13 - debug.js,8
   9 }
  10
  11 function test() {
  12
  13  test2();
  14 }
  15
  16 test();
```

#### <font color='white' size='1'><b>DefinitionLocation</b></font> ####

> <sub>Array</sub> <b>DefinitionLocation</b>( value )
> > Try to find the definition location of the given value.

#### <font color='white' size='1'><b>PropertiesList</b></font> ####

> <sub>Array</sub> <b>PropertiesList</b>( object [[.md](.md), followPrototypeChain = false ] )
> > Returns an array of properties name.

#### <font color='white' size='1'><b>PropertiesInfo</b></font> ####

> <sub>Array</sub> <b>PropertiesInfo</b>( object [[.md](.md), followPrototypeChain = false ] )
> Returns an array of properties information.


---

## class jsdebug::Debugger ##
- [top](#jsdebug_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jsdebug/Debugger.cpp?r=2290) -

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####
> <i><b>constructor</b></i>()
> > Creates a new debugger object.
> > When the program have to break because of a breakpoint, an error, a step, ... , the debugger calls its user-defined .onBreak function with the following arguments:
    * filename
    * lineno
    * breakOrigin
    * stackFrameIndex
    * hasException
    * exception
    * rval
    * isEnteringFunction


> For further information about the break context, you can call StackFrame(stackFrameIndex).

> ##### example: #####
```
  var dbg = new Debugger();
  dbg.onBreak = function(filename, lineno, breakOrigin, stackFrameIndex, hasException, exception, rval, isEnteringFunction) {

   Print( 'break at '+ filename +':'+ lineno + ' because '+breakOrigin ,'\n' );
```

### Methods ###

#### <font color='white' size='1'><b>ToggleBreakpoint</b></font> ####
> <sub>integer</sub> <b>ToggleBreakpoint</b>( polarity, filename, lineno )
> <sub>integer</sub> <b>ToggleBreakpoint</b>( polarity, function, relativeLineno )
> > Add or remove a breakpoint on the given filename:line or function.
> > The function returns the actual line number where the breakpoint has been added or removed.
> > If the file connot be reached, the function call failed with an "Invalid location" error.
> > An existing breakpoint can be overwritten by a new breakpoint.
> > Removing an nonexistent breakpoint does not matter.

#### <font color='white' size='1'><b>HasBreakpoint</b></font> ####

> <sub>boolean</sub> <b>HasBreakpoint</b>( filename, lineno )
> <sub>boolean</sub> <b>HasBreakpoint</b>( function, relativeLineno )
> > Returns _true_ if the given line (actual line number) has a breakpoint, otherwise returns `ALSE`.

#### <font color='white' size='1'><b>ClearBreakpoints</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>ClearBreakpoints</b>()
> > Unconditionally remove all breakpoints.

### Properties ###

#### <font color='white' size='1'><b>interruptCounterLimit</b></font> ####

> <sub>integer</sub> | _undefined_ <b>interruptCounterLimit</b>

#### <font color='white' size='1'><b>breakOnError</b></font> ####
> <sub>Array</sub> <b>breakOnError</b>
> > Sets whether the debugger breaks on errors.

#### <font color='white' size='1'><b>breakOnException</b></font> ####

> <sub>Array</sub> <b>breakOnException</b>
> > Sets whether the debugger breaks on exceptions (throw).

#### <font color='white' size='1'><b>breakOnDebuggerKeyword</b></font> ####

> <sub>Array</sub> <b>breakOnDebuggerKeyword</b>
> > Sets whether the debugger breaks when encounters the 'debugger' keyword.

#### <font color='white' size='1'><b>breakOnExecute</b></font> ####

> <sub>Array</sub> <b>breakOnExecute</b>
> > Sets whether the debugger breaks when a script is about to be executed.

#### <font color='white' size='1'><b>breakOnFirstExecute</b></font> ####

> <sub>Array</sub> <b>breakOnFirstExecute</b>
> > Sets whether the debugger breaks on the first script execution.

#### <font color='white' size='1'><b>excludedFileList</b></font> ####

> <sub>Array</sub> <b>excludedFileList</b>
> > Is the list of filename where the debugger never breaks.


---

- [top](#jsdebug_module.md) - [main](JSLibs.md) -
