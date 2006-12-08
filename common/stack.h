// very simple stack functions

inline void StackInit( void **stack ) {
	
	*stack = NULL;
}

inline void* StackData( const void * const *stack ) {

  return ((void**)*stack)[1];
}

inline void* StackPrev( const void * const *stack ) {

  return ((void**)*stack)[0];
}

inline void StackPush( void **stack, void *ptr ) {

  void **newItem = (void**)malloc( sizeof( void* ) * 2 ); // create a new item for the list ( pointer, pointer )
  newItem[0] = *stack; // chain the list
  newItem[1] = ptr; // store the address of the new allocated memory
  *stack = newItem; // store the new start of the list in *list
}


inline void* StackPop( void **stack ) {

  void *ptr, **item = (void**)*stack;
  *stack = item[0];
  ptr = item[1];
  free( item );
  return ptr;
}

/*
     NULL
     ^
    [0][a]
    ^
   [0][b]
   ^
  [0][c] ptr
  ^
 [0][d]
 ^
[0][e] stack

*/

// C Operator Precedence and Associativity: http://www.difranco.net/cop2220/op-prec.htm
inline void StackReverse( void **stack ) {

	for ( void *tmp, *ptr = *stack; *(void**)ptr != NULL; ) {

		tmp = *stack;
		*stack = *(void**)ptr;
		*(void**)ptr = *(void**)*stack;
		*(void**)*stack = tmp;
	}
}


inline void StackRemove( void **stack, void *data ) {

	for ( ; *stack; stack = (void**)*stack )
		if ( StackData(stack) == data ) {
			
			void *tmp = *stack;
			*stack = StackPrev(stack);
			free(tmp);
			return;
		}
}

inline bool StackIsEmpty( void * const *stack ) {

	return *stack == NULL;
}

inline bool StackLength( void * const *stack ) {

	int length = 0;
	for ( ; *stack; stack = (void**)*stack, length++ );
	return length;
}


inline bool StackHas( void * const *stack, const void *data ) {

	for ( ; *stack; stack = (void**)*stack )
		if ( StackData(stack) == data )
			return true;
  return false;
}


inline bool StackFreeContent( void **stack ) {

	while ( *stack )
      free( StackPop( stack ) );
}

inline bool StackFree( void **stack ) {

	while ( *stack )
      StackPop( stack );
}
