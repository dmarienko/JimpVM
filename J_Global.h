/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine definitions
  \  | | | | |    * 09-jun-2004: started new version
                  
*/

#ifndef __J_GLOBAL_H
#define __J_GLOBAL_H

#include "J_Types.h"
#include "hash.h"
#include <stdarg.h>

#ifdef  MAIN
#define EXPORT
#define INIT(x,v) x=v
#else
#define EXPORT extern
#define INIT(x,v) x
#endif


/* define maximal size of array of hashed classes */
#define  MAX_CLASS_HASH  1024

/* define maximal size of the heap */
#define  HEAP_SIZE       1024

/* define size of the stack */
#define  THREAD_STACK_SIZE   1024

/* define size of the extended stack */
#define  EXT_STACK_SIZE   1024


/**
 *  Variable definition
 */
typedef union _J_Var {
  int32_t  vInt;
  float    vFloat;
  int32_t  varOffs;
  void*    classRef;
  uint8_t* pc;
  void*    refValue;
  J_Object obj;
} J_Var;


/**
 *  Native function
 */
typedef J_Var (*J_NativeFunction)(J_Object obj,...);


typedef struct _J_UtfString {
  char*     str;
  uint32_t	len;
} J_UtfString;


/**
 *  Method's attributes
 */
typedef struct _J_MethodsAttr {
  uint16_t  attributeNameIdx;
  uint32_t  len;
} J_MethodsAttr;


/**
 *  Method's exceptions
 */
typedef struct _J_MethodsExceptions {
  uint32_t  len;
  uint16_t  numberOfExceptions;
  uint16_t* exceptionsTable;
} J_MethodsExceptions;


/**
 *  Method's definition
 */
typedef struct _J_ClassMethods {
  uint16_t	accessFlags;
  uint16_t	nameIdx;
  uint16_t	descrIdx;

  uint16_t	paramsCount;
  uint8_t	initMethod;
  uint8_t	isRetValue;

  uint16_t	attributesCount;
  J_MethodsAttr* methodsAttr;
  
  uint16_t  maxStack;
  uint16_t  maxLocals;

 /* Need to add Code & Exceptions table */
  uint32_t  codeLength;
  uint8_t*  code;
  uint16_t  excTableLength;
  uint8_t*  excTable;

 /* Exceptions table for this method */
  J_MethodsExceptions exceptionsAttr;

 /* Native function reference */
  J_NativeFunction fnative;

} J_ClassMethods;

/**
 *  Fields structure definition
 */
typedef struct _J_ClassFields {
  uint16_t  accessFlags;
  uint16_t  nameIdx;
  uint16_t  descrIdx;
  uint8_t   isFinal;
  J_Var     var;
} J_ClassFields;


/**
 *  Thread structure
 */
typedef struct _J_Thread {
  J_Var*   thStack;
  uint32_t thStackSize, thStackPtr;
  J_ClassMethods* currMethod;
  struct _J_Class* currClass;
} J_Thread;


/**
 *  Class definition
 */
typedef struct _J_Class { 
 /* array of this classes superclasses */
  struct _J_Class *superClass;
  uint16_t numberSuperClasses;

  J_Thread* threadContext; 
  J_Object  obj;
    
  uint16_t  thisClassIdx;
  uint16_t  superClassIdx;
  
  uint8_t*  classmem;
  
  uint32_t* consts;
  uint16_t  numConsts;
  
 /* pointer access_flags area */
  uint16_t* accessFlags;
  
  uint16_t  interfacesCount;
  uint16_t* interfaces;

 /* total variables number */
  uint16_t  numVars; 
  uint16_t  fieldsCount;
  J_ClassFields*  fields;
  
  uint16_t  methodsCount;
  J_ClassMethods* methods;
  
  struct _J_Class*  nextClass;
} J_Class;


/**
 *  Platform depend declarations
 */
#ifdef JIMP_OS
#define  free(x)      kfree(x)
#define  malloc(x)    kmalloc(x)
inline void* __zCalloc(uint32_t x,uint32_t y){
  uint8_t* p = kmalloc(x*y);
  kmemset(p,0,x*y);
  return (void*)p;
};
#define calloc(x,y)  __zCalloc(x,y)
#endif


/**
 *  Some global variables
 */
EXPORT uint16_t INIT(j_dbg_filter,DBG_ALL);
EXPORT J_Class* globalHash[MAX_CLASS_HASH];

/**
 *  Functions declarations
 */
void dbg(uint16_t dbglev,const char* fmt,...);
void _err(int errlev,const char* fmt,...);
void printMemory(uint8_t* memory,int n);

/**
 * Some usefull debug things
 */
#ifdef JIMP_OS
#define err(err,x...) {kprintf("'%s' in %s:%d:",__FUNCTION__,__FILE__,__LINE__); _err(err,##x);}
#else
#define err(err,x...) {printf("'%s' in %s:%d:",__FUNCTION__,__FILE__,__LINE__); _err(err,##x);}
#endif

J_UtfString  jUtfCreate(char* plain_string);
J_UtfString* jUtfCreateDup(char* plain_string);
uint32_t     jUtfHash(J_UtfString string);
void         viewConst(J_Class* jc,uint16_t idx);

int16_t  getInt16(uint8_t *b);
int32_t  getInt32(uint8_t *b);
uint8_t  getUint8(uint8_t *b);
uint16_t getUint16(uint8_t *b);
uint32_t getUint32(uint8_t *b);
float    getFloat32(uint8_t *b);
double   getFloat64(uint8_t *b);
int32_t  countParams(J_UtfString desc);
J_Error_t jGetMethod(J_Class* jc,J_UtfString name,J_UtfString desc,J_Class** vc,J_ClassMethods** method);
J_Object createString(char* buf);
int compatible(J_Class* source,J_Class* target,J_Thread* jth);

J_UtfString jGetClassName(J_Class* jc,uint16_t idx);
J_UtfString jGetUtfString(J_Class* jc,uint16_t idx);
J_Var getConstant(J_Class* jc,uint16_t idx);


J_Error_t jGetClass(J_Class** c,J_UtfString name,J_Thread* thContext);
J_Error_t jGetClassByIndex(J_Class* jc,J_Class** c,uint16_t idx,J_Thread* thContext);
J_Error_t jGetField(J_Class* jc,J_ClassFields** field,J_UtfString name,J_UtfString desc);
J_Error_t jGetFieldByIndex(J_Class* jc,J_ClassFields** field,uint16_t idx,J_Thread* thContext);


#define SET_DEBUG_FILTER(x) j_dbg_filter = x
#define OFF_DEBUG_FILTER(x) SET_DEBUG_FILTER(j_dbg_filter & (~x))
#define ON_DEBUG_FILTER(x)  SET_DEBUG_FILTER(j_dbg_filter | x)

/**
 * Get reference to class by object pointer
 */
#define J_OBJ_CLASS(x)      vmHeap[x].ptr[0].classRef


/**
 * Array's macroses
 */ 
#define J_ARRAY_TYPE(x)       vmHeap[x].ptr[1].vInt
#define J_ARRAY_LEN(x)        vmHeap[x].ptr[2].vInt
#define J_ARRAY_START(x)      &(vmHeap[x].ptr[3])
#define J_STRING_ARRAY_OBJ(x) vmHeap[x].ptr[1].obj

/**
 * Arrays functions
 */
uint16_t arrayType(char c);
int32_t  arrayTypeSize(uint32_t type);
uint32_t arraySize(uint32_t type,uint32_t len);
J_Object createArray(uint32_t type,uint32_t len);
J_Object createMultiArray(uint32_t ndim,char* description,J_Var* sizes);
int      compatibleArray(J_Object obj,J_UtfString arrayName);
J_Var    copyArray(J_Var* s);
J_Error_t jExecMethod(J_Class* jc,J_ClassMethods* method,J_Var param[],uint32_t numparams,J_Thread* jth);


#define CLASS_INFO(x) {J_UtfString tmp = jGetClassName(x,x->thisClassIdx); printf("Class %s\n",tmp.str); }

#endif
