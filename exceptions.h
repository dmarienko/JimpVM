#ifndef __EXCEPTIONS_H
#define __EXCEPTIONS_H

#include <setjmp.h>

#ifdef MAIN
jmp_buf __except;
#else
extern jmp_buf __except;
#endif

/**
 * Exceptions structure definition
 */
typedef struct _Exception_t {
  int   code;
  char* message;
  void* data;
} Exception_t;

#ifdef JIMP_OS
#error "Exceptions yet not realized in JimpOS !!!"
#else
#define SET_RUNTIMEXCEPTION_POINT { uint32_t __rt_e_pt;\
   if((__rt_e_pt=setjmp(__except))){ \
   Exception_t* __ex = (Exception_t*) __rt_e_pt; \
   err(ERR_FATAL,"Unhandled runtime exception '%s' [%d] was occured\n",__ex->message,__ex->code); exit(1); } } 

#define  UNKNOWN(e)  ; break; default: e = __err_code;
#define  TRY         { int __err_code = 0; \
                       Exception_t* __ex = (Exception_t*) setjmp(__except); \
                       __err_code = (__ex) ? __ex->code : 0; \
                       switch(__err_code){ case 0:
#define  CATCH(x)    ; break; case x:
#define  ENDTRY      } SET_RUNTIMEXCEPTION_POINT; }

#define  _THROW(x)    longjmp(__except,(uint32_t)x);
#define  THROW(c,s,d) { Exception_t __local_except = {c,s,d}; _THROW(&__local_except); }

#endif

#endif
