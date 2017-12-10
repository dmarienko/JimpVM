/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Native threads implementation 
*/

#include "thread.h"
#include "vmmemory.h"

pthread_mutex_t Gmutex = PTHREAD_RMUTEX_INITIALIZER;
pthread_mutex_t Glock  = PTHREAD_RMUTEX_INITIALIZER;

JIMP_thread_s 	*Gthread;
volatile uint16	 Gprio;

/* Signal handler */
void __sig_handler ( int __sig ) {

	switch( __sig ) {

	  case THREAD_STOP_SIG: 
			pause();

	  case THREAD_CONT_SIG:
			break;

	  case THREAD_CHNG_PRIO: {
			setprio( 0, Gprio ); 
		 	break;
	  }

	  case THREAD_KILL_SIG: {
			JIMP_thread_s *th = Gthread;
		  //  memcpy( th, Gthread, 4 );
			__remove_all_locks( th );
			pthread_exit( NULL );
		 	break;
	  }
	}
}

void __add_sig_handler() {

	struct sigaction act;
	sigset_t set;
	sigemptyset( &set );	
	sigaddset( &set, SIGUSR1 );

	act.sa_flags = 0;
	act.sa_mask = set;
	act.sa_handler = &__sig_handler;
	sigaction( THREAD_STOP_SIG, &act, NULL );
	sigaction( THREAD_CONT_SIG, &act, NULL );
	sigaction( THREAD_CHNG_PRIO, &act, NULL );
	sigaction( THREAD_KILL_SIG, &act, NULL );
}


void __thread_start( JIMP_thread_s *ts ) {

   /* We unlock thread's lock */
	pthread_mutex_unlock( &ts->s_mutex );
}

void __thread_set_start_lock( JIMP_thread_s *ts ) {

   /* We lock thread's lock */
	pthread_mutex_lock( &ts->s_mutex );
}

void __thread_suspend( JIMP_thread_s *ts ) {

	pthread_kill( ts->t, THREAD_STOP_SIG );
}

void __thread_resume( JIMP_thread_s *ts ) {

	pthread_kill( ts->t, THREAD_CONT_SIG );
}

void __thread_set_priority( JIMP_thread_s *ts, int prio ) {

   /* Very bad - must be mutexed ! */
	Gprio = 5*prio; /* remap [1,10] to [5,50] */
	pthread_kill( ts->t, THREAD_CHNG_PRIO );
}

void __thread_stop( JIMP_thread_s *ts ) {

	//__remove_all_locks( ts );
	Gthread  = ts;
	pthread_kill( ts->t, THREAD_KILL_SIG );
	//pthread_abort( ts->t );

 /* Try to destroy stack area for this thread */
	free( ts->VM_stack );
	free( ts );
}

/* Execute run() method of the target */
void *__thread_exec( JIMP_thread_s *vmt ) {

	JIMP_var param[1];

  /* For suspend, resume methods */ 
	__add_sig_handler();

  /* Wait for the start() signal */
	pthread_mutex_lock( &vmt->s_mutex );

  /* Exec run() method */
	param[0].obj = vmt->obj;
	JIMP_exec_method( JIMP_OBJ_CLASS( vmt->obj ), vmt->run_method, param, 1, vmt );

	pthread_mutex_unlock( &vmt->s_mutex );

  /* Try to destroy stack area for this thread */
	free( vmt->VM_stack );
	free( vmt );
}

void __thread_sleep( long msec ) {

	//if( usleep( msec * 1000 ) ) perror( "__thread_sleep> error executing usleep\n" );
	usleep( msec * 1000 );
}

/* Initialiazing mutexes */
void __init_thread_mutexes( JIMP_thread_s *vmt ){

	vmt->s_mutex = Gmutex;
	vmt->o_lock = Glock;
}

/* Implementation init(..) function */
int __thread_init( JIMP_obj obj ) {

	JIMP_thread_s *vmt;
	JIMP_class    *jc, *vc;
	JIMP_lock	  *lk;
	
	vmt = heap[obj].vm_t;
	if( vmt == NULL ) {
	  printf("__thread_init> NULL pointer to the thread structure \n"); 
	  return 1; 
	}

	vmt->VM_stack = ( JIMP_var* ) malloc( THREAD_STACK_SIZE * sizeof( JIMP_var ) );
	if( vmt->VM_stack == NULL ) { 
      printf("__thread_init> not enought space for thread stack\n"); 
	  free( vmt );
  	  return 1; 
	} 

	vmt->VM_stack_ptr = 0;
	vmt->VM_stack_size = THREAD_STACK_SIZE;	
	vmt->obj = obj;

	jc = JIMP_OBJ_CLASS( obj );
	if( jc == NULL ) { 
	  printf("__thread_init> class pointer is null cannot initialize\n");
	  free( vmt->VM_stack );
	  free( vmt );
	  return 1;
	}

	vmt->run_method = JIMP_get_method( jc, createUTF("run"), createUTF("()V"), vc );
	if( vmt->run_method == NULL ) {
	  printf( "__thread_init> cannot find method run()\n");
	  free( vmt->VM_stack );
	  free( vmt );
	  return 1;
	}

   /* Create thread's locks list */
	lk = &(vmt->lock);
	lk->obj_ref = 0;
	lk->next = NULL;
	
   /* Set thread's attributes */	
	pthread_attr_init( &vmt->attr );
	pthread_attr_setdetachstate( &vmt->attr, PTHREAD_CREATE_JOINABLE );
	pthread_attr_setinheritsched( &vmt->attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy (&vmt->attr, SCHED_RR);
	vmt->attr.param.sched_priority = 10;
	vmt->towner = 0;

   /* Create thread in wait mode */
	__thread_set_start_lock( vmt );
	pthread_create( &vmt->t, &vmt->attr, &__thread_exec, vmt );

	return 0;
}


/* Set lock to obj by thread and add lock to the lock list 
**  monitorenter opcode
*/

void __thread_lock( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_thread_s *obj_th;
	obj_th = heap[obj].vm_t;

	__add_lock( obj, own_thread );	
	pthread_mutex_lock( &obj_th->o_lock ); 

   /* Who is owner of this lock */
	heap[obj].vm_t->towner = own_thread->t;
	heap[obj].vm_t->towner_obj = own_thread->obj;
}

/* Set lock to obj by thread and DO NOT add lock to the lock list */
void __thread_lock_nadd( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_thread_s *obj_th;
	obj_th = heap[obj].vm_t;

	pthread_mutex_lock( &obj_th->o_lock ); 

   /* Who is owner of this lock */
	heap[obj].vm_t->towner = own_thread->t;
	heap[obj].vm_t->towner_obj = own_thread->obj;
}


/* Unlock object but lock not remove */
void __thread_unlock_nr( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_thread_s *obj_th;
	obj_th = (JIMP_thread_s*) heap[obj].vm_t;
	
	pthread_mutex_unlock( &obj_th->o_lock );
}

/* Unlock object and remove lock 
**  monitorexit opcode
*/
void __thread_unlock( JIMP_obj obj, JIMP_thread_s *own_thread ) {

	JIMP_thread_s *obj_th;
	obj_th = (JIMP_thread_s*) heap[obj].vm_t;
	
   /* If __remove_lock() return 1 - do not free mutex ! It's was doing early ! */	
	if( !__remove_lock( obj, own_thread ) ) 
		pthread_mutex_unlock( &obj_th->o_lock );
}

/* Implementation wait() function */
void __thread_wait( JIMP_obj obj ) {

	int32 		  nlock, ret_lock;
	JIMP_thread_s *jt;
	JIMP_obj t_owner_obj = heap[obj].vm_t->towner_obj;
	jt = (JIMP_thread_s*)(heap[t_owner_obj].vm_t);

  /* Add thread to wait queue of the object */
	if( !_add_wait_thread( (JIMP_wait_queue*)&(heap[obj].vm_t->wq), jt ) ){
	  printf("wait> cannot add current thread to the wait queue - not enought memory\n");
	  return;
	} 
	
  /* Remove all locks for its object */
	nlock = __unmark_all_locks_for_obj( obj, jt );
	if( nlock == 0 ) { printf( "wait> Object was not locked. Waiting unpossible\n" ); return; }  

  /* Wait for notify, notifyAll */
	pause(); 
	
  /* Was notified */	
	if( !_remove_wait_thread( (JIMP_wait_queue*)&(heap[obj].vm_t->wq), jt ) ) {
	  printf("wait> cannot remove thread from the wait queue - not such entry\n");
	}

  /* Restore all locks */
	 ret_lock = __restore_all_locks_for_obj( obj, jt ); 
	 if( ret_lock < 0 ){
	   printf( "wait> locks for this object was not unlocked\n" );
	   return;
	 }  
	 
	 if( ret_lock == 0 ){
	   printf( "wait> not found any unmarked locks\n" );
	   return;
	 }  
}


/* notify all threads which waiting for obj  */
int __notify_all_threads_for_obj( JIMP_obj obj ) {

	JIMP_wait_queue *w = (JIMP_wait_queue*)&(heap[obj].vm_t->wq);
	while( w ){
	  if( ( w->th > 0 ) && ( w->th_obj > 0 ) )pthread_kill( w->th, THREAD_CONT_SIG );
	  w = w->next;
	}
}

/* Implementation notify() function */
void __thread_notify( JIMP_obj obj ) {
 /* Temporary - to all threads !!! */
	__notify_all_threads_for_obj( obj );
}

/* Implementation notifyAll() function */
void __thread_notifyAll( JIMP_obj obj ) {
	__notify_all_threads_for_obj( obj );
}
