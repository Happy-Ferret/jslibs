#include <stdlib.h>
// very simple stack functions

/* internal
     NULL
     ^
    [0][a]
    ^
   [0][b]
   ^
  [0][c]
  ^
 [0][d]
 ^
[0][e] <- stack
*/

/* test
	void *stack;
	StackInit(&stack);
	StackPush(&stack,(void*)5);
	StackPush(&stack,(void*)6);

	void *p;
	p = StackFind( &stack, (void*)7 );
	p = StackFind( &stack, (void*)6 );
	p = StackFind( &stack, (void*)5 );
	p = StackFind( &stack, (void*)4 );
*/	


inline void StackInit( void **stack ) {
	
	*stack = NULL;
}


inline void* StackData( const void * const * stack ) {

  return ((void**)*stack)[1];
}

inline void StackSetData( const void * const * stack, void *data ) {

  ((void**)*stack)[1] = data;
}


inline void* StackPrev( const void * const * stack ) {

  return ((void**)*stack)[0];
}


inline void StackGoPrev( void ** stack ) {

	*stack = ((void**)*stack)[0];
}


inline bool StackIsEnd( const void * const * stack ) {

	return *stack == NULL;
}


inline int StackLength( const void * const * stack ) {

	register int length;
	for ( length = 0; *stack; stack = (void**)*stack, length++ );
	return length;
}


inline void* StackFind( void * const * stack, const void *data ) {

	register void *it;
	for ( it = *stack; it && StackData(&it) != data; StackGoPrev(&it) );
	return it;
}


inline bool StackHas( void * const * stack, const void *data ) {

	return StackFind(stack, data) != NULL;
}


inline void StackPush( void **stack, void *data ) {

  void **newItem = (void**)malloc( sizeof( void* ) * 2 ); // create a new item for the list ( pointer, pointer )
  newItem[0] = *stack; // chain the list
  newItem[1] = data; // store the address of the new allocated memory
  *stack = newItem; // store the new start of the list in *list
}


inline void* StackPop( void **stack ) {

  void *data, **item = (void**)*stack;
  *stack = item[0]; // keep the chain
  data = item[1];
  free( item );
  return data;
}


inline bool StackReplaceData( void * const *stack, const void *data, void *newData ) {

	void *it;
	if ( it = StackFind( stack, data ) )
		StackSetData( stack, newData );
	return it != NULL;
}


inline bool StackIterate( void * const * stack, void **iterator ) { // usage: for ( void *it = NULL; StackIterate( &stack, &it ); ) // alternate method: for ( void *it = stack; !StackIsEnd(&it); it = StackPrev(&it) )
	
	return *iterator = *iterator == NULL ? *stack : StackPrev(iterator);
}


inline void StackReverse( void **stack ) {

	for ( void *tmp, *ptr = *stack; *(void**)ptr != NULL; ) {

		tmp = *stack;
		*stack = *(void**)ptr;
		*(void**)ptr = *(void**)*stack;
		*(void**)*stack = tmp;
	}
}


inline bool StackRemove( void **stack, void *data ) {

	for ( ; *stack; stack = (void**)*stack )
		if ( StackData(stack) == data ) {
			
			void *tmp = *stack;
			*stack = StackPrev(stack);
			free(tmp);
			return true;
		}
	return false;
}


inline void StackFreeContent( void **stack ) {

	while ( *stack )
      free( StackPop( stack ) );
}


inline void StackFree( void **stack ) {

	while ( *stack )
      StackPop( stack );
}
