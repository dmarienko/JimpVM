/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Garbage collection procedure
*/

#include "jimptypes.h"
#include "jimpclass.h"
#include "vmmemory.h"

/* Testing search and mark */
void mark( JIMP_obj obj ) {
	int i, type, ns, len;
	JIMP_class *jt;
	JIMP_obj   ob;

/* Debug */
	if( heap[obj].__used == 0 ) return;
	if( !obj ) return;
	if( obj > __sz_heap ) return;

heap[obj].__used = 2;
ns = 0;

ns_loop:
	jt =  JIMP_OBJ_CLASS( obj );

  /* Check is it array */	
	if( jt == NULL ){
	  type = JIMP_ARRAY_TYPE( obj );
	/* if array - try mark every element */
	  if( ( type==1 ) || ( type==2 ) ){
		JIMP_obj *arr; 
		arr = (JIMP_obj*) JIMP_ARRAY_START( obj );
		i = 0;
		for( i=0; i < JIMP_ARRAY_LEN(obj); i++ ) {
printf( "mark> [%d]\n", arr[i] );
		  heap[ arr[i] ].__used = 2;
	 	  heap[ns++].tmp = arr[i];
		}
	  }
	} else {
	  len = jt->num_vars;
	  for(i=0; i<len; i++) { 
	    ob = heap[obj].__ptr[i+1].obj;
printf( "mark> [%d]\n", ob );
	    heap[ob].__used = 2; 

		heap[ns++].tmp = ob;
	  }
	}

	if(ns>0){
	  --ns;
	  obj = heap[ ns ].tmp;
	  goto ns_loop;
	}
}

void gc(){
	int i = 0, j, n, type;
	JIMP_class *jt;

JIMP_utf_string cn;
char cname[512];
JIMP_var *v;

printf("\t\t\t--> Garbage collector <--\n");

   /* mark all objects on the stack */
	for( i=0; i < VM_stack_ptr; i++ ) {
	  mark( VM_stack[i].obj );
	}

   /* mark all objects on the extended stack */
	for( i=0; i < EX_stack_ptr; i++ ) {
	  mark( EX_stack[i].obj );
	}

   /* mark all objects on the hash heap - static objects */
	for( i=0; i < MAX_CLASS_HASH; i++ ){
	  jt = Class_Hash[ i ];

	  if( jt != NULL ) {  
	  /* try to do it for all fields */
		for( j=0; j < jt->fields_count; j++ ){
		  JIMP_class_fields *f;
		  f = &(jt->fields[j]);
		  if( f->access_flags & ACCESS_STATIC ) {
		    v = &(f->var);
		    mark( v->obj );
		  }
		}
		jt = jt->next_class;
	  }
	}

/* - - - - - - - - - - - - - - - */

	for( i=1; i<__sz_heap; i++ ) {
      if( (heap[i].__used > 0) && (heap[i].__used != 2) ) {
		free_object( i );   /* try to free it */
	  }
  		else  {
		  heap[i].__used = 1;
		}
	}
}

