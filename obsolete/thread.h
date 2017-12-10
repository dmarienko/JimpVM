#ifndef __THREAD_H
#define __THREAD_H

#include "jimpclass.h"

typedef struct __JIMP_lock {

	JIMP_obj obj_ref;
	int32	 n_times_locked;
	
	struct __JIMP_lock *next;
	
} JIMP_lock;

typedef struct __JIMP_wait_queue {
   
	JIMP_obj	th_obj;	/* Object - thread owner */	
	pthread_t   th;	/* Waiting thread  */
	uint16		prio;	/* ??? */
	
	struct __JIMP_wait_queue *next;
	
} JIMP_wait_queue;

typedef struct __JIMP_thread_s {

  /* Object - thread owner */
	JIMP_obj	obj;

  /* Thread context */ 	
	pthread_t   t;
	pthread_attr_t attr;

  /* All thread's locks - for thread only */ 
	JIMP_lock  lock;

  /* Who is owner of this lock */
	JIMP_obj	towner_obj;
	pthread_t   towner;
	
  /* Thread's wait queue - for Object */
	JIMP_wait_queue wq;

  /* Run method */
 	JIMP_class_methods *run_method;

  /* Stack section */
	JIMP_var   *VM_stack;
	uint32		VM_stack_size;
	uint32		VM_stack_ptr;
  /* Startup mutex - for thread */
	pthread_mutex_t s_mutex;

  /* Object lock - for object  */
	pthread_mutex_t o_lock;  

} JIMP_thread_s; 

void __sig_handler( int __sig );
void __add_sig_handler();

int  __thread_init( JIMP_obj obj );
void __thread_sleep( long msec );
void __thread_start( JIMP_thread_s *ts );
void __thread_suspend( JIMP_thread_s *ts );
void __thread_resume( JIMP_thread_s *ts );

void __init_thread_mutexes( JIMP_thread_s *vmt );

void  __thread_lock( JIMP_obj obj, JIMP_thread_s *own_thread );
void  __thread_unlock( JIMP_obj obj, JIMP_thread_s *own_thread );

/* Signals defines */
#define THREAD_STOP_SIG		SIGUSR1
#define THREAD_CONT_SIG		SIGUSR2
#define THREAD_CHNG_PRIO    NSIG-1
#define THREAD_KILL_SIG     NSIG-2

/* Thread stack size in JIMP_var units */
#define THREAD_STACK_SIZE	1024

#endif
