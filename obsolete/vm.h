#ifndef __VM_H
#define __VM_H

#include "jimptypes.h"
#include "jimpclass.h"
#include "thread.h"

extern int JIMP_vm_initialize;

/* Object class - loading directly for increasing performance */
extern JIMP_class *__OBJECTCLASS;

/* String class - loading directly for increasing performance */
extern JIMP_class *__STRINGCLASS;

int JIMP_vm_init( uint32 sz_class_mem, uint32 sz_heap_mem, uint32 sz_stack );
void JIMP_exec_method( JIMP_class *jc, JIMP_class_methods *method, JIMP_var param[], uint32 numparams, JIMP_thread_s *jts );


#endif
