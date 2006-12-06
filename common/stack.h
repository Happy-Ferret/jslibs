
// very simple stack functions
static void StackPush( void **stack, void *ptr ) {

  void **newItem = (void**)malloc( sizeof( void* ) * 2 ); // create a new item for the list ( pointer, pointer )
  newItem[0] = *stack; // chain the list
  newItem[1] = ptr; // store the address of the new allocated memory
  *stack = newItem; // store the new start of the list in *list
}


static void* StackPop( void **stack ) {

  void *ptr, **item = (void**)*stack;
  ptr = item[1];
  *stack = item[0];
  free( item );
  return ptr;
}


static void StackRemove( void **stack, void *ptr ) {

  while ( *stack != NULL ) {

    if ( (*(void***)stack)[1] == ptr ) {

      void* item = *stack;
      *stack = **(void***)stack;
      free( item );
      return;
    }
    stack = (void**)*stack;
  }
}


static bool StackHas( void **stack, void *ptr ) {

  while ( *stack != NULL ) {

    if ( (*(void***)stack)[1] == ptr )
      return true;
    stack = (void**)*stack;
  }
  return false;
}


/*
static bool StackFree( void **stack, void *ptr ) {

	while ( *stack )
      free( StackPop( ppList ) );
}
*/