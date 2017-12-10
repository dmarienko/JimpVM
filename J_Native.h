/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine native functions
  \  | | | | |    * 10-oct-2004: started
                  
*/

#ifndef __J_NATIVE_H
#define __J_NATIVE_H

#include "J_Global.h"

typedef struct _J_FuncNativeHash {
  uint32_t         fHash;
  char*            fname;
  J_NativeFunction fptr;
} J_FuncNativeHash;

#define MAX_NATIVE_NAME_LEN  256

void  jInitializeNativeFunctions();
J_Var loadFile(J_UtfString className,uint32_t* classSize);
void  jLoadNativeFunction(J_Class* jc,uint16_t midx);

typedef void     J_Void;
typedef int32_t  J_Int;
typedef float    J_Float;
typedef J_Object J_String;
typedef uint8_t  J_Byte;
typedef char     J_Char;
typedef int16_t	 J_Short;
typedef uint8_t  J_Boolean;

J_Var callJNIMethod(J_Var* ostack,J_Object this,J_Class* jc,J_NativeFunction* f,int args,int need_ret);

#endif
