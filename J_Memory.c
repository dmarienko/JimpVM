/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine memory functions
  \  | | | | |    * 06-mar-2005: started new version
                  * 
				                
*/

#include "J_Memory.h"


/**
 *  Allocate object on the heap
 */
J_Object allocateObject(J_Class* jc,uint32_t objSize){
  int i, pos = vmhTop + 1;
 L_SEEK_AGAINE:
  if(pos>=vmhSize || vmHeap[pos].used){
	pos = -1;
	for(i=0;i<vmhSize;i++){
	  if(!vmHeap[i].used){ pos = i;	break; }
	}
  }
  
  if(pos<0){ 					/* need gc here */
	jGC();
	pos = vmhSize;
	goto L_SEEK_AGAINE;
  }

 /* For all non array objects this function
  *  called as 'allocateObject(...., 1);' and we must
  *  define object size
  */
  if(objSize<sizeof(J_Var)) 
	objSize = (jc->numVars+1)*sizeof(J_Var);

 /* Allocate object */
  vmHeap[pos].ptr = (J_Var*) calloc(1,objSize);
  if(vmHeap[pos].ptr==NULL)
	err(ERR_FATAL,"unable allocate memory for heap entry\n");

  vmHeap[pos].ptr->classRef = jc;
  vmHeap[pos].used = 1;
  vmHeap[pos].marked = 0;

  return (vmhTop = pos);
}


/**
 * Try to free object
 */
void freeObject(J_Object obj){
  if(!vmHeap[obj].used){
	err(ERR_FATAL,"freeObject> attempting free an empty object" );
	return;
  }
  
  vmHeap[obj].used = 0;
  vmHeap[obj].marked = 0;
  
  if(vmHeap[obj].ptr!=NULL)
	free(vmHeap[obj].ptr);

  vmhTop = obj;
 //if(obj-1>=1) vmhTop = obj-1;
  return;
}


/**
 * Allocate memory for thread's structure and stack
 */
J_Thread* allocateThreadMemory(uint32_t stackSize){
  J_Thread* t = (J_Thread*) malloc(sizeof(J_Thread));
  if(t==NULL)
	err(ERR_FATAL,"not enought memory for allocation");
  t->thStack = (J_Var*) malloc(stackSize * sizeof(J_Var));
  if(t->thStack==NULL)
	err(ERR_FATAL,"not enought memory for allocation");
  t->thStackSize = stackSize;
  t->thStackPtr = 0;
  return t;
}


/**
 * Attempt to free thread's structure
 */
void freeThreadMemory(J_Thread* t){
  if(t!=NULL){
	if(t->thStack!=NULL) free(t->thStack);
	free(t);
  }
}


/**
 * Push object on extended stack
 */
int jExtPushObj(J_Object obj){
  if(exStackPtr > exStackSize){
	err(ERR_FATAL,"Not enought extended stack space\n");
	return -1;
  }
  exStack[exStackPtr++].obj = obj;
  return 1;
}


/**
 * Pop object from extended stack
 */
J_Object jExtPopObj(){
  if((exStackPtr - 1)<0){
	err(ERR_FATAL,"Wrong pop operation on extended stack\n");
	return -1;
  }
  return exStack[--exStackPtr].obj;
}


/**
 * Initialize virtual machine object's heap
 */
J_Error_t initializeHeap(uint32_t heapSize){
  vmHeap = (J_HeapSlot*) calloc(heapSize,sizeof(J_HeapSlot));
  if(!vmHeap){
	err(ERR_FATAL,"Can't allocate virtual heap (%d units)\n",heapSize);
	return ENOMEM;
  }
  vmhTop = -1;
  vmhSize = heapSize;

 /* Initialize extended stack */
  exStack = (J_Var*) calloc(EXT_STACK_SIZE,sizeof(J_Var));
  exStackSize = EXT_STACK_SIZE;
  exStackPtr = 0;
	
  return EOK;
}

