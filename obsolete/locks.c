/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Locks
*/

#include "thread.h"
#include "vmmemory.h"

/* Add lock reference to the thread's locks list */
void __add_lock( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p;
	
	/* Check if lock on this object is exist ? */
	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;
	while( l ){
	  if( l->obj_ref == obj ) { l->n_times_locked++; return; }
	  l = l->next;
	  p = p->next;
	}
	
	l = (JIMP_lock*) malloc( sizeof(  JIMP_lock ) );
	p->next = l;
	l->obj_ref = obj;
	l->n_times_locked = 1;
	l->next = NULL;
}

/* Remove lock reference from the thread's locks list */
int __remove_lock( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p, *n;
	int  do_not_mutexes;
	
	/* Try to find lock */
	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;

	while( l ){
	  if( l->obj_ref == obj ) {  
	  
	  	do_not_mutexes = 0;
	    if( l->n_times_locked < 0 ) do_not_mutexes = 1;
		
	  	l->n_times_locked--; 
		if( l->n_times_locked <= 0 ){
		  n = l->next;
		  p->next = n;
		  free( l );  
		/* Nobody own of its lock */
		  heap[ obj ].vm_t->towner = 0;
		  heap[ obj ].vm_t->towner_obj = 0;
		  return do_not_mutexes;
		}
		return do_not_mutexes; 
	  }
	  p = p->next;
	  l = l->next;
	}
	
	return 0; 
}

/* Remove all locks for all object locked by this thread */
void __remove_all_locks( JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p;
	int n_locks;

	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;
	
	while( l ){
	  n_locks = abs(l->n_times_locked);
	  while( n_locks > 0 ) { n_locks--; __thread_unlock( l->obj_ref, own_thread );}
	  p = (JIMP_lock *)&(own_thread->lock);
	  l = p->next;
	}
}

/* Remove all locks for this object locked by obj thread */
int __remove_all_locks_for_obj( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p;
	int n_locks, ret_nlock;

	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;
	
	while( l ){
	  if(l->obj_ref == obj) {
	    ret_nlock = n_locks = abs(l->n_times_locked);
	    while( n_locks > 0 ) { n_locks--; __thread_unlock( l->obj_ref, own_thread );}
		return ret_nlock;
	  }
	  p = (JIMP_lock *)&(own_thread->lock);
	  l = p->next;
	}
	return 0;
}

/* Unmark all locks for obj object locked by this thread but do not remove */
int __unmark_all_locks_for_obj( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p;
	int n_locks, ret_nlock;

	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;
	
	while( l ){
	  if(l->obj_ref == obj) {
	    ret_nlock = n_locks = l->n_times_locked;
	    while( n_locks > 0 ) { n_locks--; __thread_unlock_nr( l->obj_ref, own_thread );}
		
	   /* Mark locks unlocked */	
		l->n_times_locked = -ret_nlock;
		return ret_nlock;
	  }
	  p = (JIMP_lock *)&(own_thread->lock);
	  l = p->next;
	}
	return 0;
}


/* Restore all locks for obj object locked by this thread */
int __restore_all_locks_for_obj( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_lock *l, *p;
	int n_locks, ret_nlock;

	p = (JIMP_lock *)&(own_thread->lock);
	l = p->next;
	
	while( l ){
	  if(l->obj_ref == obj) {
	    n_locks = l->n_times_locked;
	
	/* Locks was not unmarked - error */
		if( n_locks > 0 ) return -1;
		ret_nlock = n_locks = abs( n_locks );
	    while( n_locks > 0 ) { n_locks--; __thread_lock_nadd( l->obj_ref, own_thread ); }
		
	   /* Mark locks locked */	
		l->n_times_locked = ret_nlock;
		return ret_nlock;
	  }
	  p = (JIMP_lock *)&(own_thread->lock);
	  l = p->next;
	}
	return 0;
}

/* View all locks for this thread */
void see_locks( JIMP_thread_s *own_thread ) {

	JIMP_lock *l;
	l = (JIMP_lock *)&(own_thread->lock);
	
	printf("----- LOCKS DUMP -----\n");
	while( l ){
	  printf("| lock=%x, lock->next=%x, lock->obj_ref=%d, lock->n_times_locked=%d |\n", l, l->next, l->obj_ref, l->n_times_locked  );
	  l = l->next;
	}
}
	
