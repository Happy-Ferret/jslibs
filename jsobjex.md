- [source](http://jslibs.googlecode.com/svn/trunk/./src/jsobjex/) - [main](JSLibs.md) -
# jsobjex module #
> jsobjex is a module that contains only ObjEx class.


---

## jsobjex::ObjEx class ##
- [src](http://jslibs.googlecode.com/svn/trunk/./src/jsobjex/objex.cpp) - [top](#jsobjex_module.md) -
> This class give the ability to spy properties changes. One can listen for add, del, set and set events on the object.
> It is also possible to store an hidden auxiliary value that can be access using ObjEx.Aux( _ObjEx object_ ) static function.

  * **_constructor_**( [addCallback](addCallback.md), [delCallback](delCallback.md), [getCallback](getCallback.md), [setCallback](setCallback.md) [, auxObject] )
> > Constructs a ObjEx object.
> > ##### arguments: #####
      1. <sub>Function</sub> _addCallback_: called when the property is added to the object.
      1. <sub>Function</sub> _delCallback_: called when the property is deleted form the object.
      1. <sub>Function</sub> _getCallback_: called when the property is read form the object.
      1. <sub>Function</sub> _setCallback_: called when the property is changed. This include when the property is added.
      1. <sub>Value</sub> _auxObject_:
> > ##### note: #####
> > > addCallback, delCallback, getCallback, setCallback: can be set to 

&lt;undefined&gt;

 value.

> > ##### behavior: #####
> > > addCallback, delCallback, getCallback, setCallback functions are called according to the operation done on the object.
> > > These callback functions are called with the following arguments:
> > > > propertyName, propertyValue, auxObject, callbackIndex
      * propertyName : the name of the property being handled.
      * propertyValue : the value of the property being handled.
      * auxObject : the _auxObject_ provided to the constructor.
      * callbackIndex : an integer that has the folowing value: 0 for addCallback, 1 for delCallback, 2 for getCallback, 3 for setCallback.

> > ##### note: #####
> > > addCallback callback function is called only when the property is being added, in opposition to _setCallback_ that is called each time the value is changed.

### Static functions ###

  * **Aux**( objex [, newAux] )

> > Returns the _auxObject_ stored in the _objex_.
> > If newAux is given, it replaces the current auxiliary value of _objex_.

### Example ###
```
function addCallback( name, value ) {

   Print('adding ' + name + ' = ' + value);
}

var obj = new ObjEx( addCallback, undefined, undefined, undefined, null );

obj.foo = 123;
obj.foo = 456;
```
prints:
```
adding foo = 123
```

### Example ###


> http://jsircbot.googlecode.com/svn/trunk/dataObject.js



---

- [top](#jsobjex_module.md) - [main](JSLibs.md) - [source](http://jslibs.googlecode.com/svn/trunk/./src/jsobjex/) - [QA](http://jslibs.googlecode.com/svn/trunk/./src/jsobjex/qa.js) -
