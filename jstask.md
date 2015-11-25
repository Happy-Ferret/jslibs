
---

## class jstask::Task ##
- [top](#jstask_module.md) -
[revision](http://code.google.com/p/jslibs/source/browse/trunk/./src/jstask/task.cpp?r=2557) -
> With multicore CPUs becoming prevalent, splitting computationally expensive tasks is a good way to obtain better over-all performance.
> The aim of the Task class is to make use of CPU resources for doing computation task like decompressing data, decoding/computing images, create cryptographic keys, processing audio data, complex query on large database ...
> A task can also manage asynchronous I/O, but it is not recommended. Poll() function is a preferred way to manage asynchronous I/O.

> The "new Task(taskFunc);" expression creates a new thread (or more), and a new JavaScript runtime that runs in the thread, then the thread is waiting for a request.
> Each time the Request(req) function is called, the request req is stored in a queue and the thread is unlocked and call the function taskFunc with the first queued request.
> When the taskFunc has finish, its return value is stored in the response queue and the thread returns in the "waiting for a request" state.
> Responses are retrieved from the response queue using the Response() function.

> ##### notes: #####
> > Creating new tasks are expensive operating system calls. Tasks may have to be reused over the time.
> > The function taskFunc is completely isolated from the main program, Request()/Response() API is the only way to exchange data.
> > If no response are pending, the Response() function will block until a response is available.
> > The request/response queue size is only limited by the available amount of memory.
> > If an error or an exception occurs while a request is processed, the exception is stored as the response and raised again when the Response() function is called.
> > The execution context of the taskFunc function is reused, this mean that the global object can be used to store data.
> > The second argument of the taskFunc(req, index) is the index of the current request. This value can be used for initialization purpose.
> > eg.
```
  function MyTask( req, idx ) {
   
   if ( idx == 0 ) {
    LoadModule('jsio');
   }
   ...
  }
```

#### <font color='white' size='1'><i><b>constructor</b></i></font> ####

> <i><b>constructor</b></i>( taskFunc [[.md](.md) , priority = 0 ] )
> > Creates a new Task object from the given function.
> > ##### arguments: #####
      1. <sub>`UN`</sub> _taskFunc_: the JavaScrip function that will bi run as thread.
      1. <sub>integer</sub> _priority_: 0 is normal, -1 is low, 1 is high.
> > > The _taskFunc_ prototype is: `function( request, index )`.

#### <font color='white' size='1'><b>Request</b></font> ####

> <font color='gray' size='1'><sub>void</sub></font> <b>Request</b>( [[.md](.md)data] )
> > Send data to the task. This function do not block. If the task is already processing a request, next requests are automatically queued.

#### <font color='white' size='1'><b>Response</b></font> ####

> data <b>Response</b>()
> > Read a response from the task. If no response is pending, the function wait until a response is available.

#### <font color='white' size='1'><b>pendingRequestCount</b></font> ####

> <sub>integer</sub> <b>pendingRequestCount</b>
> > Is the number of requests that haven't be processed by the task yet. The request being processed is not included in this count.

#### <font color='white' size='1'><b>processingRequestCount</b></font> ####

> <sub>integer</sub> <b>processingRequestCount</b>
> > Is the number of requests that are currently processed by the task.

#### <font color='white' size='1'><b>pendingResponseCount</b></font> ####

> <sub>integer</sub> <b>pendingResponseCount</b>
> > Is the number of available responses that has already been processed by the task.

#### <font color='white' size='1'><b>idle</b></font> ####

> <sub>integer</sub> <b>idle</b>
> > Is the current state of the task. true if there is no request being processed, if request and response queues are empty and if there is no pending error.

### Examples ###

> ##### example 1: #####
```
 LoadModule('jsstd');
 LoadModule('jstask');

 function MyTask( request ) {

  var sum = 0;
  for ( var i = 0; i < request; i++) {

   sum = sum + i;
  }
  return sum;
 }

 var myTask = new Task(MyTask);

 for ( var i = 0; i < 100; i++ ) {

  myTask.Request(i);
 }

 while ( !myTask.idle )
  Print( myTask.Response(), '\n' );
```
