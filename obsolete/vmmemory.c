/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Functions for allocating memory
*/

#include "vmmemory.h"

uchar *alloc_class_memory( uint32 size ) {

	uchar *p;

	// align to 4 byte boundry
	size = (size + 3) & ~3;
	if ( __used_class_memory + size > __sz_class_memory ){
	 printf("alloc_class_memory> not avalaible memory\n");
	 return 0;
	}

	p = &__class_memory[ __used_class_memory ];
	__used_class_memory += size;

	return p;
}


uint32 alloc_heap_memory( uint32 heap_size ) {

	int i;

	heap = ( JIMP_heap* ) malloc( heap_size * sizeof( JIMP_heap ) );
	if( !heap ) {
     printf("alloc_heap_memory> not avalaible memory\n");
     return 0;
    }

	__top_heap = 0;
	__sz_heap = heap_size;
	                         
	// Mark all entries unused
	for(i=1;i<__sz_heap;i++) heap[i].__used = 0;
        
    return 1;
}

uint32 alloc_vm_stack( uint32 sz_stack ) {

/* VM stack allocating */
	if( !(VM_stack = (JIMP_var*)malloc( sz_stack * sizeof( JIMP_var )))) {
     printf("alloc_vm_stack> not avalaible memory\n");
     return 0;
    }

    VM_stack_size = sz_stack;
    VM_stack_ptr  = 0;

/* Extended stack allocating */
	if( !(EX_stack = (JIMP_var*)malloc( sz_stack * sizeof( JIMP_var )))) {
     printf("alloc_vm_stack> not avalaible memory for extended stack\n");
     return 0;
    }

    EX_stack_size = sz_stack;
    EX_stack_ptr  = 0;

    return 1;
}

JIMP_obj alloc_object( JIMP_class *jc, uint32 sz_object ) {

	int i;

	if((__top_heap+1) > __sz_heap) {

	/* Seek any unused entry */
	 for( i=1; i<__sz_heap; i++ ) 
	 if( !heap[i].__used ){
	  __top_heap = i;
	  goto __ret_ok;
	 }

gc();
for( i=1; i<__sz_heap; i++ )
  if( !heap[i].__used ){ __top_heap = i; goto __ret_ok; }

	 printf( "alloc_object> not enought free heap entries\n" );
	 return 0;
	}

	__top_heap++;

__ret_ok:
   /* Для не array как минимум 1 ptr !!! */
	if( sz_object < sizeof( JIMP_var ) ) sz_object = ( jc->num_vars + 1) * sizeof( JIMP_var );

	heap[__top_heap].__ptr = (JIMP_var*) malloc( sz_object );
	if(!(&heap[__top_heap].__ptr[0])) {
	  printf("alloc_object> unable allocate memory for heap entry\n");
	  return 0;
	}

	heap[__top_heap].__ptr[0].class_ref = jc;
    heap[__top_heap].__used = 1;

    heap[__top_heap].vm_t = (JIMP_thread_s*) (JIMP_thread_s*) malloc( sizeof( JIMP_thread_s ) );
printf("alloc (%d)> vm_t = %x\n", __top_heap, heap[__top_heap].vm_t );
	if( heap[__top_heap].vm_t == NULL ) { 
	  printf("alloc_object> not enought space for thread structure \n"); 
	  return 0; 
	}
	
   /* Default owner obj's locks - nobody :) */
	heap[__top_heap].vm_t->towner = 0;
	heap[__top_heap].vm_t->towner_obj = 0;
	__init_thread_mutexes( heap[__top_heap].vm_t );

	return __top_heap;
}

void free_object( JIMP_obj __obj ) {
	JIMP_var *_v;
	JIMP_thread_s *_t;

	if(__obj > __sz_heap){
	 printf( "free_object> wrong object pointer\n" );
	 return;
	}
  
	if( !heap[__obj].__used ){
	 printf( "free_object> cannot free empty entry\n" );
	 return;
	}

	heap[__obj].__used = 0; 
/*
	if( heap[__obj].__ptr != NULL ) free( &heap[__obj].__ptr[0] );
	if( heap[__obj].vm_t != NULL) free( &heap[__obj].vm_t[0] );
*/
	if( heap[__obj].__ptr != NULL ) {
	  _v = &(heap[__obj].__ptr[0]);  
printf("free(%d)> __ptr to kill = %x\n", __obj, _v);
	  free( _v );
	}
	if( heap[__obj].vm_t != NULL) {
	  _t = heap[__obj].vm_t; 
printf("free(%d)> vm_t to kill = %x\n", __obj, _t);
printf("mprobe(vm_t) = %d\n", mprobe( _t ) );
	  free( _t );	
printf("OK\n");
	}

	if(__obj - 1 >= 1) __top_heap = __obj - 1;

	return;
}

int ex_push_obj( JIMP_obj obj ) {

	if( EX_stack_ptr > EX_stack_size ) {
	  printf("ex_push_obj> not enought extended stack space\n");
	  return -1;
	}	

	EX_stack[EX_stack_ptr++].obj = obj;
	return 1;
}

JIMP_obj ex_pop_obj() {
	int __s = EX_stack_ptr - 1; 
	if( __s < 0 ){
	  printf("ex_pop_obj> wrong pop operation\n");
	  return -1;
	}
	return EX_stack[--EX_stack_ptr].obj;
}
