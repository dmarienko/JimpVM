/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine arrays functions
  \  | | | | |    * 06-mar-2005: started new version
                  * 
				                
*/

#include "J_Global.h"
#include "J_Memory.h"


uint16_t arrayType(char c){
  switch(c){
  case 'L': // object
	return 1;
  case '[': // array
	return 2;
  case 'Z': // boolean
	return 4;
  case 'B': // byte
	return 8;
  case 'C': // char
	return 5;
  case 'S': // short
	return 9;
  case 'F': // float
	return 6;
  case 'I': // int
	return 10;
  case 'D': // double
	return 7;
  case 'J': // long
	return 11;
  }
  return 0;
}


int32_t arrayTypeSize(uint32_t type){
  switch(type){
  case 1:  /* object */
  case 2:  /* array */
	return 4;
  case 4: /* boolean */
  case 8: /* byte */
	return 1;
  case 5:  /* char */
  case 9:  /* short */
	return 2;
  case 6:  /* float */
  case 10: /* int */
	return 4;
  case 7:  /* double */
  case 11: /* long */
	return 8;
  }
  return 0;
}


uint32_t arraySize(uint32_t type,uint32_t len){
  int32_t typesize, size;
  
  typesize = arrayTypeSize(type);
  size = (3*sizeof(J_Var))+(typesize*len);
  
 /* align to 4 byte boundary */
  size = (size+3)&~3;
  return size;
}


/**
 * Create one dimensional array object
 */
J_Object createArray(uint32_t type,uint32_t len){
  J_Object obj = allocateObject(NULL,arraySize(type,len));
  if(!obj) return 0;
  
  J_ARRAY_TYPE(obj) = type;
  J_ARRAY_LEN(obj) = len;
  
  return obj;
}


/**
 * Create multi dimensional array object
 */
J_Object createMultiArray(uint32_t ndim,char* description,J_Var* sizes){
  J_Object  arrayObj, subArray;
  J_Object* itemStart;
  int32_t   i, len, type;

  len = sizes[0].vInt;
  type = arrayType(description[0]);
  arrayObj = createArray(type,len);
  
  if(!arrayObj) return 0;
  if((type!=2) || (ndim<=1)) // if type is not array and not multidimension
	return arrayObj;

 /* prevent from gc() */	
  jExtPushObj(arrayObj);

 /* create subarrays */
  for(i=0;i<len;i++){
	subArray = createMultiArray(ndim-1,description+1,sizes+1);
	itemStart = (J_Object*) J_ARRAY_START(arrayObj);
	itemStart[i] = subArray;
  }
  
  jExtPopObj();
  return arrayObj;
}


/** 
 *  Check is its array compatible with description
 */
int compatibleArray(J_Object obj,J_UtfString arrayName){
  J_Class* jc;

  jc = J_OBJ_CLASS(obj);
  if(jc!=NULL) return 0; // source is not array
  if(J_ARRAY_TYPE(obj) != arrayType(arrayName.str[1]))
	return 0;
  return 1;
}


uint8_t jCopyArray(J_Object src,uint32_t bsrc,J_Object dst,uint32_t bdst,uint32_t len){
  uint8_t  r = 0;
  uint32_t t, tsz;
  uint8_t* from,* to;
  
  if((!src)||(!dst)){
	err(ERR_FATAL,"copyArray> can't copy null array");
	return r; 
  }

 /* Check len to copy */
  if(len==0)
	return r;

  if((J_OBJ_CLASS(src)!=NULL) || (J_OBJ_CLASS(dst)!=NULL))
	return r;

 /* Check src/dst compatibility */
  t = J_ARRAY_TYPE(src);
  if(t!=J_ARRAY_TYPE(dst))
	return r;
 
 /* Check array's lenghts */
  if(bsrc+len>J_ARRAY_LEN(src))
	return r;
 
  if(bdst+len>J_ARRAY_LEN(dst))
	return r;
 
  tsz = arrayTypeSize(t);
  from = (uint8_t*) J_ARRAY_START(src) + (tsz*bsrc);
  to =   (uint8_t*) J_ARRAY_START(dst) + (tsz*bdst);
 
 /* copy here */ 
  memcpy((uint8_t*)to,(uint8_t*)from,len*tsz);
  r = 1;
  return r;
}


