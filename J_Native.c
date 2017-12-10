/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine native functions
  \  | | | | |    * 10-oct-2004: started
                  * 12-feb-2005: new native interface realised
*/
#include "J_Types.h"
#include "J_Native.h"

/* Autogenerated file - here described all native functions */
#include "natives.h" 


/** Method for native loading class file 
 *  pointer to loaded class in memory return in pc member of J_Var
 *  size return in the <classSize> variable
 *  If some errors was occured NULL return
 */
J_Var loadFile(J_UtfString className,uint32_t* classSize){
  J_Var r;
  char* fullName;

  *classSize = 0;
  if(className.len<=0){
	err(ERR_WARNING,"Empty class name\n");
	r.pc = NULL;
	return r;
  }

  fullName = (char*) malloc(className.len+7);
  memcpy(fullName,className.str,className.len);
  memcpy(fullName+className.len,".class\x0",7);
 
#ifndef JIMP_OS
  { 
	int fd;
    struct stat buf;

   // Try to open file and allocate memory
	fd = open(fullName,O_RDONLY);
	if(fd<0 || fstat(fd,&buf)<0){
	  err(ERR_WARNING,"Error opening %s file\n",fullName);
	  r.pc = NULL;
	  goto LBL_ERR;
	}
	*classSize = buf.st_size;
	r.pc = (uint8_t*) calloc(sizeof(uint8_t),*classSize);
	if(r.pc==NULL){
	  err(ERR_WARNING,"Error allocate buffer for %s\n",fullName);
	  close(fd);
	  goto LBL_ERR;
	}

	if(read(fd,r.pc,*classSize)!=(*classSize))
	  err(ERR_WARNING,"Error reading %s\n",fullName);
	close(fd);
  };

#else
 // Not realised yet
#endif

 LBL_ERR:
  free(fullName);
  return r;
}


/**
 * This function initialize native functions table
 *  and generate hashe code for each 
 */
void jInitializeNativeFunctions(){
  J_FuncNativeHash* p;
  for(p=funcHash;p->fptr!=NULL;p++)
	p->fHash = hash((uint8_t*) p->fname);
}


/**
 *  Find native function and init method
 */
void jLoadNativeFunction(J_Class* jc,uint16_t midx){
  int i = 0;
  uint32_t h;
  char tmp[MAX_NATIVE_NAME_LEN];
  J_UtfString name, desc, cname;
  J_ClassMethods* m = &jc->methods[midx];
  J_FuncNativeHash* p;
  
  cname = jGetClassName(jc,jc->thisClassIdx);
  name  = jGetUtfString(jc,m->nameIdx);
  desc  = jGetUtfString(jc,m->descrIdx);

 /* Make full method name:
  * for example native method 'int valueOf(String s)'
  *  of 'java.lang.String' class will translated into
  * 'java/lang/String.valueOf(Ljava/lang/String;)I' string
  * NOTE: to avoid a sprintf use memcpy function instead
  */
  i = cname.len;
  memcpy(tmp,cname.str,i);
  tmp[i++] = '.';
  memcpy(tmp+i,name.str,name.len);
  i += name.len;
  memcpy(tmp+i,desc.str,desc.len);
  i += desc.len;
  tmp[i] = 0;
  
 /* get hash code and try to find such method */
  h = hash((uint8_t*)tmp);
  for(p=funcHash;p->fptr!=NULL;p++)
	if(p->fHash==h){
	  m->fnative = (J_NativeFunction) p->fptr;
	  return;
	}
  dbg(ERR_WARNING,"native method %s not found",tmp);
}


/**
 *  Try to call native function 'func' with 'args' arguments placed onto the 'stack'
 */
J_Var callJNIMethod(J_Var* ostack,J_Object this,J_Class* jc,J_NativeFunction* func,int args,int need_ret){
  J_Var  ret;
  J_Var* opntr = ostack + args;
  J_Object* this2 = this;
  int i;

  for(i = 0; i < args; i++)
	asm volatile ("pushq %0" :: "m" (*--opntr) : "rsp");

  if(jc){
  	asm volatile ("pushq %0" :: "p" (this2) : "rsp");
	args++;
  }
    
  if(need_ret) ret = (J_Var) (*(uint32_t (*)())func)();
  else (*(void (*)())func)();
	
  asm volatile ("addq %0,%%rsp" :: "r" ((args) * sizeof(uint32_t)) : "cc", "rsp");
  return ret;
}