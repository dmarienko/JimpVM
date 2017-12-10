/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine definitions
  \  | | | | |    * 17-oct-2004: started new version
                  
*/

#ifndef __J_MEMORY_H
#define __J_MEMORY_H

#include "J_Types.h"
#include "J_Global.h"

typedef struct _J_HeapSlot {
  uint8_t  used;
  uint8_t  marked;
  J_Var*   ptr;
  J_Object tmp;
} J_HeapSlot;

/* uint8_t* __class_memory; */
/* uint32_t __used_class_memory; */
/* uint32_t __sz_class_memory; */

/* virtual machine stack */
//EXPORT J_Var*   vmStack;
//EXPORT uint32_t vmStackSize; /* in J_Var units */
//EXPORT uint32_t vmStackPtr;

/* iextended stack */
EXPORT J_Var*   exStack;
EXPORT uint32_t exStackSize; /* in J_Var units */
EXPORT uint32_t exStackPtr;

/* virtual machine heap  */
EXPORT J_HeapSlot* vmHeap;
EXPORT uint32_t    vmhTop;
EXPORT uint32_t    vmhSize;

/* uint8_t* alloc_class_memory( uint32 size ); */
J_Error_t initializeHeap(uint32_t heapSize);
/* uint32   alloc_vm_stack( uint32 sz_stack ); */
J_Object allocateObject(J_Class* jc,uint32_t ns);
J_Thread* allocateThreadMemory(uint32_t stackSize);
void freeThreadMemory(J_Thread* t);
/* void     free_object( JIMP_obj obj ); */

/* extended stack operations */
int	     jExtPushObj(J_Object obj);
J_Object jExtPopObj();

#endif
