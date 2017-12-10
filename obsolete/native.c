/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Native functions interface
*/

#include "jimptypes.h"
#include "jimpclass.h"
#include "vmmemory.h"
#include "native.h"
#include "thread.h"

/* First working function  - it's a history :) */
JIMP_var aaaa( JIMP_var v[] ){

	JIMP_var r;
	printf("%d\n", v[0].int_value );
	return r;
}

JIMP_var kputs( JIMP_var v[] ){

	JIMP_var r;
	JIMP_obj k;
	char   *c;
	int    i,l;

	k = JIMP_STRING_ARRAY_OBJ( v[0].obj );
	c = (char*)JIMP_ARRAY_START(k);
	l = 2*JIMP_ARRAY_LEN( k );
	for(i=0;i<l;i+=2) printf("%c", *(c+i) );
	return r;
}

JIMP_var start0( JIMP_var v[] ) {
	JIMP_var r;
	__thread_start( heap[v[0].obj].vm_t );
	return r;
}

JIMP_var init0( JIMP_var v[] ) {
	JIMP_var r;

	__thread_init( v[0].obj );
	return r;
}

JIMP_var suspend0( JIMP_var v[] ) {
	JIMP_var r;
	__thread_suspend( heap[v[0].obj].vm_t );
	return r;
}

JIMP_var resume0( JIMP_var v[] ) {
	JIMP_var r;
	__thread_resume( heap[v[0].obj].vm_t );
	return r;
}

JIMP_var sleep0( JIMP_var v[] ) {
	JIMP_var r;

	__thread_sleep( v[0].int_value );
	return r;
}

JIMP_var stop0( JIMP_var v[] ) {
	JIMP_var r;

	__thread_stop( heap[v[0].obj].vm_t );
	return r;
}

JIMP_var set_prio0( JIMP_var v[] ) {
	JIMP_var r;
	__thread_set_priority( heap[v[0].obj].vm_t, v[1].int_value );
	return r;
}

JIMP_var copy_arr0( JIMP_var v[] ) {
	JIMP_var r;
	r = copy_array( v );
	return r;
}

JIMP_var to_str_i0( JIMP_var v[] ) {
	JIMP_var r;
	char __t[32];

	r.obj = create_UTF_from_string( createUTF( itoa( v[0].int_value, __t, 10 ) ) );
	return r;
}

JIMP_var to_str_float0( JIMP_var v[] ) {
	JIMP_var r;
	char __t[32];

	sprintf( __t, "%f", v[0].float_value );
	r.obj = create_UTF_from_string( createUTF( __t ) );
	return r;
}

JIMP_var wait0( JIMP_var v[] ) {
	JIMP_var r;

	__thread_wait( v[0].obj );
	return r;
}

JIMP_var notify0( JIMP_var v[] ) {
	JIMP_var r;
	
	__thread_notify( v[0].obj );
	return r;
}

JIMP_var notifyAll0( JIMP_var v[] ) {
	JIMP_var r;

	__thread_notifyAll( v[0].obj );
	return r;
}


NATIVE_FUNCT __nf[] = {
	{ 40200L,  aaaa },    /* aaaa(I)V */
	{ 155546L, kputs },   /* kputs(Ljava/lang/String;)V */
	{ 46408L,  start0 },  /* start()V */
	{ 148250,  init0 },   /* init0(Ljava/lang/Thread;)V */
	{ 52810L,  sleep0 },  /* sleep0(I)V */
	{ 59978L,  suspend0 },/* suspend()V */
	{ 52745L,  resume0 }, /* resume()V */
	{ 42824L,  stop0 },   /* stop0()V */
	{ 95120L,  set_prio0 }, /* setPriority0(I)V */
	{ 299059L, copy_arr0 }, /* copyArray(Ljava/lang/Object;ILjava/lang/Object;II)Z */
	{ 173341L, to_str_i0 }, /* toString(I)Ljava/lang/String; */
	{ 173149L, to_str_float0 }, /* toString(F)Ljava/lang/String; */
	{ 38663L,  wait0 },			/* wait()V */
	{ 53257L,  notify0 },		/* notify()V */
	{ 71244L,  notifyAll0 },	/* notifyAll()V */
	{ 0, NULL }
};

/* Search and run native function */
void JIMP_load_native_funct( JIMP_class *jc, uint16 midx )
{
	int i;
	JIMP_utf_string name, desc;
	JIMP_class_methods *m = &jc->methods[midx];

    name = get_UTF_string( jc, m->name_idx );
    desc = get_UTF_string( jc, m->descr_idx );

   /* Search in the hash table of the native functions */
    i = 0;
    while( __nf[i].f != NULL ) { 
      if( __nf[i].hash == ( gen_hash_code(name) + gen_hash_code(desc) ) ) {
        m->fnative =__nf[i].f;
        break;
      }
      i++;
    }
}

