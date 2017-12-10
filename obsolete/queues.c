/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Queues for the waiting threads 
*/

#include "thread.h"
#include "vmmemory.h"

/* Add thread to wait queue */
int _add_wait_thread( JIMP_wait_queue *wq, JIMP_thread_s *jt ) {

	JIMP_wait_queue *l = wq, *p;
	p = l->next;
	
   /* Move to free slot */	
	while( p ){ 
	  p = p->next; 
	  l = l->next; 
	}
	
	p = (JIMP_wait_queue*) malloc( sizeof( JIMP_wait_queue ) );
	if( p == NULL ) return 0;
	
	p->next = NULL;
	p->th_obj = jt->obj;
	p->th = jt->t;
	l->next = p;
	
   /* All Ok */
	return 1;
}


/* Remove thread from the wait queue */
int _remove_wait_thread( JIMP_wait_queue *wq, JIMP_thread_s *jt ) {

	JIMP_wait_queue *l = wq, *p, *n;
	p = l->next;
	
   /* Seek thread's slot */	
	while( p ){ 
	  if( (p->th == jt->t) && (p->th_obj == jt->obj) ) {
	    n = p->next;
		l->next = n;
		free( p );
		return 1;   /* removed OK */
	  }
	  p = p->next; 
	  l = l->next; 
	}
	/* Not found */
	return 0;
}
