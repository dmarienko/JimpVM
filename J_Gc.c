/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine Garbage collection methods
  \  | | | | |    * 08-jul-2005: start researches
                  * 20-mar-2006: fixed some bugs and got stable version
				 
*/

#include "J_Global.h"
#include "J_Memory.h"


#define IS_OBJECT(n) (n>=0 && n<vmhSize)
#define IS_MARK(n) vmHeap[n].marked==1
#define MARK(n)    vmHeap[n].marked=1

extern J_Thread* mainThread;

int GC_DBG = 0;


/**
 *  Walk throught the all object's fields
 */
void jWalkObjectsFields(J_Class* jc,J_Object obj){
  int i;
  J_ClassFields* field;
  J_UtfString fname, fdesc;
  
  for(i=0;i<jc->numVars;i++){
	field = &(jc->fields[i]);
	fname = jGetUtfString(jc,field->nameIdx);
	fdesc = jGetUtfString(jc,field->descrIdx);

   /* if not simple type - object */
	if(fdesc.str[0]=='L' || fdesc.str[0]=='['){
	  J_Object fobj = vmHeap[obj].ptr[field->var.varOffs+1].obj;
	  
	  if(IS_OBJECT(fobj)){
		J_UtfString tmp;
		J_Class* fjc = J_OBJ_CLASS(fobj);
		
		if(IS_MARK(fobj)) continue;
		
		if(GC_DBG){
		  tmp = jGetClassName(jc,jc->thisClassIdx);
		  printf("-->[%s%s]",fname.str,fdesc.str);
		}
 
		if(fjc!=NULL){
		  vmHeap[fobj].marked = 1; 
		  jWalkObjectsFields(fjc,fobj);
		} else {
		 /* if array */
		  walkArray(fobj);
		  MARK(fobj);
		}
	  
	  }
	}
	if(GC_DBG){printf("\n");}
  }
}


/**
 *  Walk throught the all array's elements
 */
void walkArray(J_Object obj){
  int len = J_ARRAY_LEN(obj), i;
  uint32_t type = J_ARRAY_TYPE(obj);
 
  if(IS_MARK(obj)) return;

  if(GC_DBG)
	printf("(arr:%d,%d)",type,len);

  if(type==1){					/* array of objects */
	J_Var* objects = J_ARRAY_START(obj);
	for(i=0;i<len;i++){
	  J_UtfString tmp;
	  J_Class* jc = J_OBJ_CLASS(objects[i].obj);
	 /* interested situation : if array of objects already created but still not filled fully ! */
	  if(jc==NULL) continue;
	  
	  if(GC_DBG){
		tmp = jGetClassName(jc,jc->thisClassIdx);
		tmp.str[tmp.len]=0; 
		printf("=>[%s : %d]",tmp.str,objects[i].obj);
	  } 

	  jWalkObjectsFields(jc,objects[i].obj);
	  MARK(obj);
	  MARK(objects[i].obj);
	}
  }
  if(GC_DBG) printf("\n");
}


/**
 *  Mark all accessible objects
 */
void jGC_mark(J_Var* thStack, int thStackPtr){
  int i;
  
 /* Walk through the thread's stack */
  for(i=0;i<thStackPtr;i++){
	J_Object obj = thStack[i].obj;	

	if(IS_OBJECT(obj)){
	  J_UtfString tmp;
	  J_Class* jc = J_OBJ_CLASS(obj);

	  if(IS_MARK(obj)) continue;
	  
	 /* if null it's array */
	  if(jc==NULL){
		walkArray(obj);
		continue;
	  }
	  
	  if(GC_DBG){
		tmp = jGetClassName(jc,jc->thisClassIdx);
		tmp.str[tmp.len]=0; 	  
		printf("{%s}",tmp.str);
	  }
	  
	 /* mark object */
	  MARK(obj);
 
	 /* look on object's fields */
	  jWalkObjectsFields(jc,obj);
	  
	  if(GC_DBG) printf("\n");	  
	}
  }
}


/**
 *  Sweep (free) all unused objects from the heap
 */
void jGC_sweep(){
  int i, killed = 0, alive = 0;
  for(i=0;i<vmhSize;i++){
	if(vmHeap[i].used){
	  J_UtfString tmp;
	  J_Class* jc = J_OBJ_CLASS(i);
	  
	  if(!vmHeap[i].marked){
		
		if(GC_DBG & 0x02)
		  if(jc!=NULL){tmp = jGetClassName(jc,jc->thisClassIdx); printf("DESTROY object[%d] : %s\n",i,tmp.str);}
  
        freeObject(i);
		killed++;				/* some statistic */
		continue;
	  }

	  if(jc==NULL){
		if(GC_DBG)
		  printf("alive array[%d]: len = %d, type = %d\n",i,J_ARRAY_LEN(i),J_ARRAY_TYPE(i));
	  } else {
		if(GC_DBG){
		  tmp = jGetClassName(jc,jc->thisClassIdx);
		  printf("alive object[%d] : %s\n",i,tmp.str);
		}
	  }
	}
	vmHeap[i].marked = 0;
	alive++;					/* some statistic */
  }

  if(GC_DBG) printf("GC: killed: %d, alive: %d\n",killed,alive);
}


/**
 * Gagbage collector
 *  NOT WORKED PROPERLY WITH SIMPLE ARRAYS (not object's arrays !) - 20-mar-2006: fixed
 */
J_Error_t jGC(){
  static int n_gc = 0;

GC_DBG = 0;
 
  if(GC_DBG){
	J_UtfString  methodName = jGetUtfString(mainThread->currClass,mainThread->currMethod->nameIdx);
	J_UtfString  tmp = jGetClassName(mainThread->currClass,mainThread->currClass->thisClassIdx);	
	printf("\033[2J");
	printf("----------------< START GC: %d from method %s.%s [%d] >-------------------\n",n_gc,tmp.str,methodName.str,mainThread->currClass->obj);
  }

 /* mark all objects on extended stack */
  jGC_mark(exStack,exStackPtr);
  
 /* mark all objects on main thread stack */
  jGC_mark(mainThread->thStack,mainThread->thStackPtr);
  
 /* sweep garbage */
  jGC_sweep();
  
  if(GC_DBG)
	printf("----------------< EXIT GC >-------------------\n");

  n_gc++;
  return EOK;
}
