#ifndef __VMMEMORY_H
#define __VMMEMORY_H

#include "jimptypes.h"
#include "jimpclass.h"
#include "thread.h"

typedef struct __JIMP_heap
{
	JIMP_var	  *__ptr;
    uint8		  __used;
	JIMP_obj	  tmp;
	JIMP_thread_s *vm_t;
} JIMP_heap;

uchar	 *__class_memory;
uint32	  __used_class_memory;
uint32    __sz_class_memory;

/* virtual machine stack */
JIMP_var *VM_stack;
uint32   VM_stack_size; /* in JIMP_var units */
uint32   VM_stack_ptr;

/* iextended stack */
JIMP_var *EX_stack;
uint32   EX_stack_size; /* in JIMP_var units */
uint32   EX_stack_ptr;

/* virtual machine heap  */
JIMP_heap *heap;
uint32	 __top_heap;
uint32	 __sz_heap;

uchar    *alloc_class_memory( uint32 size );
uint32   alloc_heap_memory( uint32 heap_size );
uint32   alloc_vm_stack( uint32 sz_stack );
JIMP_obj alloc_object( JIMP_class *jc, uint32 ns );
void     free_object( JIMP_obj obj );
int	     ex_push_obj( JIMP_obj obj );
JIMP_obj ex_pop_obj();

#endif
