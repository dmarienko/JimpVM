#include "../J_Native.h"
#include "../J_Memory.h"


J_Void java_lang_JVKernel_kputs(J_Object this,J_String s) {
  char* c;
  int   i,l;
  J_Object stringArray = J_STRING_ARRAY_OBJ(s);
  c = (char*) J_ARRAY_START(stringArray); 
  l = 2 * J_ARRAY_LEN(stringArray);
  for(i=0;i<l;i+=2) putchar(*(c+i));
}


static char* int2str(int x,char* buf){
  if(x/10) buf=int2str(x/10,buf);
  *buf++ = '0' + x%10;
  *buf = 0;
  return buf;
}


/**
 *  Wrong work for the negative values, must be rewrited
 */
J_String java_lang_JVKernel_toString(J_Object this,J_Int v) {
  J_Var  r;
  char tmp[32];
  int2str(v,tmp);
  r.obj = createString(tmp);
  return r.obj;
}

J_Boolean java_lang_JVKernel_copyArray(J_Object this,J_Object src,J_Int bsrc,J_Object dst,J_Int bdst,J_Int len){
  return jCopyArray(src,bsrc,dst,bdst,len);
}
