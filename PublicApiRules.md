## How the jslibs API should behave ##

  * jslibs API behavior must be the same in safeMode and unsafeMode.
> > The difference between these two mode is the number of checks and assertions.

  * Don't break the program on a non-fatal errors, just report a warning.
> > eg. Closing a file descriptor that is already closed.