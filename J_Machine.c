/*
  _           _         
   | . |   | |_|  Jimp Virtual Machine byte interpreter
  \  | | | | |    * 12-feb-2005: start new version
                  
*/

#define MAIN

#include "J_Global.h"
#include "J_Memory.h"
#include "J_Native.h"
#include "J_Opcodes.h"

#include "exceptions.h"

/**
 *  Classes for pleloading
 */
static char* preloadedClasses[] = {
  "java/lang/Object",
  "java/lang/String",
  "java/lang/Thread",
  "java/lang/StringBuffer",
  "java/lang/JVKernel",
  NULL
};


/**
 *  Initialize VM
 */
J_Error_t initVM(){
  int i;

 /* Clear hash */
  for(i=0;i<MAX_CLASS_HASH;i++)
	globalHash[i] = (J_Class*) 0;

 /* Allocate memory */
  initializeHeap(HEAP_SIZE);

 /* Init native functions interface */
  jInitializeNativeFunctions();

  return EOK;
}


/**
 * Temporary generation exception
 */
#define GENERATE_EXCEPTION(x) {err(ERR_FATAL,"%s",x);}

/**
 *  Main execute method function
 *  execute method 'method' of class 'jc' with array parameters 'param[]' in thread context 'jth'
 *  number of parameters in variable 'numparams'
 *
 *  +---------+
 *  |  .....  | <- vmStack
 *  +---------+
 *  |    0    | = vmStackPtr = baseFramePtr
 *  +---------+
 *  |    0    | 
 *  +---------+
 *  |    0    | 
 *  +---------+
 *  |    P0   | = vmStackPtr <- {var}  (1) 
 *  +---------+       |
 *   .........        |
 *  +---------+       |
 *  |    PN   |       |
 *  +---------+       |
 *  |         |      (+) maxLocals
 *  |localVars|       |
 *  |         |       |
 *  +---------+       |
 *  |         | <-----+ = vmStackPtr <- {stack} (2) 
 *  |         |       |
 *  |  stack  |       |
 *  |         |      (+) maxStack
 *  |         |       |
 *  +---------+       |
 *  |  method | <-----+ = vmStackPtr (3)
 *  +---------+      (+) 1 
 *  |   jc    |      (+) 1 
 *  +---------+       |  
 *  |         | <-----+ = vmStackPtr (4)
 *    .......
 */
J_Error_t jExecMethod(J_Class* jc,J_ClassMethods* method,
					  J_Var param[],uint32_t numparams,J_Thread* jth){
  register uint8_t* pc;
  register J_Var*   stack, *var, retVal;
  uint32_t baseFramePtr;
  uint16_t pushRetVal = 0;
  int tn = 0;

 /* For fast access */	
  J_Var*   vmStack     = jth->thStack;
//  uint32_t vmStackPtr  = jth->thStackPtr;

 /* cannot exec native */
  if(method->accessFlags & ACCESS_NATIVE)
	return ECANTRUNNATIVE;

   /* have'nt code section */
  if(method->codeLength==0)
	return EZEROLENGTH;

 /* save frane pointer here */
  baseFramePtr = jth->thStackPtr;

 /* check stack */
  if(jth->thStackPtr+method->maxLocals+method->maxStack+5 >= jth->thStackSize)
	return ESTACKOVERFLOW;

  vmStack[jth->thStackPtr++].pc = 0;
  vmStack[jth->thStackPtr++].refValue = 0;
  vmStack[jth->thStackPtr++].refValue = 0; /* 1 */

 /* push params into local vars of frame */
  for(tn=0;tn<numparams;tn++)
	vmStack[jth->thStackPtr+tn] = param[tn];
  
 L_METHOD_INVOKE:

  var = &vmStack[jth->thStackPtr];
  jth->thStackPtr += method->maxLocals;                      /*          STACK          */
  stack = &vmStack[jth->thStackPtr];	         /* 2 */        /* +-----------+           */
  jth->thStackPtr += method->maxStack;        /* 3 */        /* | stack[1]  | <---+     */
  vmStack[jth->thStackPtr++ ].refValue = method;             /* +-----------+ {stack++} */
  vmStack[jth->thStackPtr++ ].refValue = jc;	 /* 4 */        /* |  *stack   |-----+     */
  pc = method->code;                                    /* +-----------+ {stack--} */
                                                        /* | stack[-1] | <---+     */ 
                                                        /* +-----------+           */
  jth->currMethod = method;
  jth->currClass = jc;
 /* Start instruction parsing */              
 STEP:
  
#ifdef DISASSEMBLER  
  dbg(DBG_DISASM,"\t%s",disassembed[*pc]);
#endif
  
  switch(*pc){
  case OP_nop:
	pc++;
	break;
	
   /* Push null reference 
	* Stack: ... -> ..., null
	*/
  case OP_aconst_null:
	stack[0].obj = 0;
	stack++;
	pc++;
	break;

   /* Push value between -1, ..., 5, other values cause an exception 
	* Stack: ... -> ..., <value>
	*/
  case OP_iconst_m1:
  case OP_iconst_0:
  case OP_iconst_1:
  case OP_iconst_2:
  case OP_iconst_3:
  case OP_iconst_4:
  case OP_iconst_5:
	stack[0].vInt = (*pc-OP_iconst_0);
	stack++;
	pc++;
	break;

   /* Push value 0.0f
	* Stack: ... -> ..., <0.0f>
	*/
  case OP_fconst_0:
	stack[0].vFloat = 0.0f;
	stack++;
	pc++;
	break;

  case OP_fconst_1:
	stack[0].vFloat = 1.0f;
	stack++;
	pc++;
	break;
	
  case OP_fconst_2:
	stack[0].vFloat = 2.0f;
	stack++;
	pc++;
	break;

   /* Push byte on stack
	* Stack: ... -> ..., value
 	*/
  case OP_bipush:
	stack[0].vInt = ((int8_t*)pc)[1];
	stack++;
	pc += 2;
	break;

   /* Push short on stack
	* Stack: ... -> ..., value
	*/
  case OP_sipush:
	stack[0].vInt = getInt16(&pc[1]);
	stack++;
	pc += 3;
	break;
	
   /* Push item from constant pool.
	* Stack: ... -> ..., item
	*/
  case OP_ldc:
	*stack = getConstant(jc,(uint16_t)pc[1]);
	stack++;
	pc += 2;
	break;

   /* Push item from constant pool (wide index)
	* Stack: ... -> ..., item
	*  from pc[1] and pc[2] make 16-bits index in constant pool
	*/
  case OP_ldc_w:
	*stack = getConstant(jc,getUint16(&pc[1]));
	stack++;
	pc += 3; 
	break;

   /* Push long or double from constant pool 
	* Stack: ... -> ..., item.word1, item.word2
 	*/
  case OP_ldc2_w: // - LONG -
   // *stack = get_constant_value( jc, get_uint16(pc + 1) );
   // *(++stack) = get_constant_value( jc, get_uint16(pc + 1) );
	dbg(DBG_VM," ldc2_w instruction (constant index: %d)",getUint16(&pc[1]));
	stack+=2;
	pc+= 3;
	break;

   /* Load int/float/reference from local variable onto stack
	* Stack: ... -> ..., value
	*/
  case OP_iload:
  case OP_fload:
  case OP_aload:
	*stack = var[pc[1]];
	stack++;
	pc += 2;
	break;

   /* Load int/float/reference from local variable onto stack by index
	* Stack: ... -> ..., value
	*/
  case OP_iload_0:
  case OP_iload_1:
  case OP_iload_2:
  case OP_iload_3:
	*stack = var[*pc - OP_iload_0];
	stack++;
	pc++;
	break;

  case OP_fload_0:
  case OP_fload_1:
  case OP_fload_2:
  case OP_fload_3:
	*stack = var[*pc - OP_fload_0];
	stack++;
	pc++;
	break;

  case OP_aload_0:
  case OP_aload_1:
  case OP_aload_2:
  case OP_aload_3:
	*stack = var[*pc - OP_aload_0];
	stack++;
	pc++;
	break;

   /* Load int/short/float/char/byte from array 
	* Stack: ...,arrayref,index -> ...,value
	*/
  case OP_iaload:
  case OP_saload:
  case OP_faload:
  case OP_caload:
  case OP_baload:
  case OP_aaload: {
	J_Object arrayRef;
	J_Var*   objPtr;
	int32_t  index;

	arrayRef = stack[-2].obj;
	index = stack[-1].vInt;
	if(arrayRef==0)
	  GENERATE_EXCEPTION("NULL_POINTER_EXCEPTION");
	
	if((index<0) || (index>=J_ARRAY_LEN(arrayRef))){
	  printf("obj = %d, index = %d, array size = %d\n",arrayRef,index,J_ARRAY_LEN(arrayRef));
	  GENERATE_EXCEPTION("ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION");
	}

   /* start  array */
	objPtr = J_ARRAY_START(arrayRef);

	switch(*pc){
	case OP_saload:
	case OP_caload:{
	  int16_t* i16 = (int16_t*) objPtr;
	  stack[-2].vInt = (int32_t) i16[index];
	};break;
	case OP_faload:{
	  stack[-2].vFloat = objPtr[index].vFloat;
	};break;
	case OP_baload:{
	  int8_t* bArray = (int8_t*) objPtr;
	  stack[-2].vInt = (int32_t) bArray[index];
	};break;
	case OP_aaload:{
	  stack[-2].obj = objPtr[index].obj;
	};break;
	default:
	  stack[-2].vInt = objPtr[index].vInt;
	}
	
	stack--;
	pc++;
	break;
  }


   /* Store reference/int/float into local variable 
	* Stack ..., v -> ... 
	*/
  case OP_astore:
  case OP_istore:
  case OP_fstore:
	stack--;
	var[pc[1]] = *stack;
	pc += 2;
	break;
	
   /* Store int into local variable by index
	* Stack ..., v -> ... 
	*/
  case OP_istore_0:
  case OP_istore_1:
  case OP_istore_2:
  case OP_istore_3:
	stack--;
	var[*pc - OP_istore_0] = *stack;  
	pc++;
	break;
	
   /* Store float into local variable by index
	* Stack ..., v -> ... 
	*/
  case OP_fstore_0:
  case OP_fstore_1:
  case OP_fstore_2:
  case OP_fstore_3:
	stack--;
	var[*pc - OP_fstore_0] = *stack;
	pc++;
	break;

   /* Store reference into local variable by index
	* Stack ..., v -> ... 
	*/
  case OP_astore_0:
  case OP_astore_1:
  case OP_astore_2:		
  case OP_astore_3:
	stack--;
	var[*pc - OP_astore_0] = *stack;
	pc++;
	break;

   /* Store value into int/short/float/reference array 
	* Stack: ..., arrayref, index, value -> ...
	*/
  case OP_iastore:
  case OP_bastore:
  case OP_castore:
  case OP_sastore:
  case OP_fastore:
  case OP_aastore: {
	J_Object arrayRef;
	J_Var*   objPtr;
	int32_t  index;
	
	arrayRef = stack[-3].obj;
	index = stack[-2].vInt;

	if(arrayRef==0)
	  GENERATE_EXCEPTION("NULL_POINTER_EXCEPTION");
	
	if((index<0) || (index>=J_ARRAY_LEN(arrayRef)))
	  GENERATE_EXCEPTION("ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION");

   /* start  array */
	objPtr = J_ARRAY_START(arrayRef);

	switch(*pc){
	case OP_sastore:
	case OP_castore:{
	  int16_t* i16 = (int16_t*) objPtr;
	  i16[index] = (int16_t) stack[-1].vInt;
	};break;
	case OP_fastore:{
	  objPtr[index].vFloat = stack[-1].vFloat;
	};break;
	case OP_bastore:{
	  int8_t* bArray = (int8_t*) objPtr;
	  bArray[index] = (int8_t) stack[-1].vInt;
	};break;
	case OP_astore:{
	  objPtr[index].obj = stack[-1].obj;
	};break;
	default:
	  objPtr[index].vInt = stack[-1].vInt;
	}
	
	stack -= 3;
	pc++;
	break;
  }

   /* Pop top operand stack word 
	* Stack: ..., word -> ...
	*/
  case OP_pop:
	stack--;
	pc++;
	break;

   /* Pop two top operand stack words 
	* Stack: ..., word2, word1 -> ...
	*/
  case OP_pop2:
	stack -= 2;
	pc++;
	break;

  case OP_dup:
	stack[0] = stack[-1];
	stack++;
	pc++;
	break;

	/* Duplicate top operand stack word and put two down 
	 * Stack: ..., word2, word1 -> ..., word1, word2, word1
	 */
  case OP_dup_x1:
	stack[0] = stack[-1];
	stack[-1] = stack[-2];
	stack[-2] = stack[0];
	stack++;
	pc++;
	break;

  case OP_dup_x2:
	stack[0] = stack[-1];
	stack[-1] = stack[-2];
	stack[-2] = stack[-3];
	stack[-3] = stack[0];
	stack++;
	pc++;
	break;

  case OP_dup2:
	stack[1] = stack[-1];
	stack[0] = stack[-2];
	stack += 2;
	pc++;
	break;

   /* Duplicate two top operand stack words and put three down 
	* Stack: ..., word3, word2, word1 -> ..., word2, word1, word3, word2, word1
 	*/
  case OP_dup2_x1:
	stack[1] = stack[-1];
	stack[0] = stack[-2];
	stack[-1] = stack[-3];
	stack[-2] = stack[1];
	stack[-3] = stack[0];
	stack += 2;
	pc++;
	break;
		  
   /* Duplicate two top operand stack words and put four down 
	* Stack: ..., word4, word3, word2, word1 -> ..., word2, word1, word4, word3, word2, word1
	*/
  case OP_dup2_x2:
	stack[1] = stack[-1];
	stack[0] = stack[-2];
	stack[-1] = stack[-3];
	stack[-2] = stack[-4];
	stack[-3] = stack[1];
	stack[-4] = stack[0];
	stack += 2;
	pc++;
	break;

  case OP_swap: {
	J_Var v = stack[-2];
	stack[-2] = stack[-1];
	stack[-1] = v;
	pc++;
	break;
  }

  case OP_iadd:
	stack[-2].vInt += stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_fadd:
	stack[-2].vFloat += stack[-1].vFloat;
	stack--;
	pc++;
	break;

  case OP_isub:
	stack[-2].vInt -= stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_fsub:
	stack[-2].vFloat -= stack[-1].vFloat;
	stack--;
	pc++;
	break;
	
  case OP_imul:
	stack[-2].vInt *= stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_fmul:
	stack[-2].vFloat *= stack[-1].vFloat;
	stack--;
	pc++;
	break;

  case OP_idiv:
	if(stack[-1].vInt==0) GENERATE_EXCEPTION("ARITHMETIC_EXCEPTION");
	stack[-2].vInt /= stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_fdiv:
	if(stack[-1].vFloat==0.0f) GENERATE_EXCEPTION("ARITHMETIC_EXCEPTION");
	stack[-2].vFloat /= stack[-1].vFloat;
	stack--;
	pc++;
	break;
	
  case OP_irem:
	if(stack[-1].vInt==0) GENERATE_EXCEPTION("ARITHMETIC_EXCEPTION");
	stack[-2].vInt = stack[-2].vInt % stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_frem: {
	float f;
	if(stack[-1].vFloat==0.0f) GENERATE_EXCEPTION("ARITHMETIC_EXCEPTION");
	f = stack[-2].vFloat / stack[-1].vFloat;
	f = (float)((int32_t)f);
	f *= stack[-1].vFloat;
	stack[-2].vFloat = stack[-2].vFloat - f;
	stack--;
	pc++;
	break;
  }

  case OP_ineg:
	stack[-1].vInt = -stack[-1].vInt;
	pc++;
	break;

  case OP_fneg:
	stack[-1].vFloat = -stack[-1].vFloat;
	pc++;
	break;

  case OP_ishl:
	stack[-2].vInt = stack[-2].vInt << stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_ishr:
	stack[-2].vInt = stack[-2].vInt >> stack[-1].vInt;
	stack--;
	pc++;
	break;

   /* Logical shift right int
	* Stack: ..., value1, value2 -> ..., result
	*/
  case OP_iushr: {
	int32_t i = stack[-1].vInt;
	if(stack[-2].vInt>=0){
	  stack[-2].vInt = stack[-2].vInt >> i;
	} else {
	  stack[-2].vInt = stack[-2].vInt >> i;
	  if(i>=0) stack[-2].vInt += (int32_t) 2 << (31-i);
	  else
		stack[-2].vInt += (int32_t) 2 << ((-i)+1);
	}
	stack--;
	pc++;
	break;
  }

  case OP_iand:
	stack[-2].vInt &= stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_ior:
	stack[-2].vInt |= stack[-1].vInt;
	stack--;
	pc++;
	break;

  case OP_ixor:
	stack[-2].vInt ^= stack[-1].vInt;
	stack--;
	pc++;
	break;
	
  case OP_iinc:
	var[pc[1]].vInt += (int8_t)pc[2];
	pc += 3;
	break;
	
  case OP_i2f:
	stack[-1].vFloat = (float)stack[-1].vInt;
	pc++;
	break;

  case OP_f2i: {
	float f;
	f = stack[-1].vFloat;
	if(f>2147483647.0) stack[-1].vInt = 0x7FFFFFFF;
	else if(f<-2147483648.0) stack[-1].vInt = 0x80000000;
	else stack[-1].vInt = (int32_t)f;
	pc++;
	break;
  }

  case OP_i2b:
	stack[-1].vInt = (int32_t)((int8_t)(stack[-1].vInt & 0xFF));
	pc++;
	break;

  case OP_i2c:
	stack[-1].vInt = (int32_t)((uint16_t)(stack[-1].vInt & 0xFFFF));
	pc++;
	break;

  case OP_i2s:
	stack[-1].vInt = (int32_t)((int16_t)(stack[-1].vInt & 0xFFFF));
	pc++;
	break;

  case OP_fcmpl:
  case OP_fcmpg: {
	float f = stack[-2].vFloat - stack[-1].vFloat;
	if(f>0.0f) stack[-2].vInt = 1;
	else if(f<0.0f) stack[-2].vInt = -1;
	else stack[-2].vInt = 0;
	stack--;
	pc++;
	break;
  }

  case OP_ifeq:
	if(stack[-1].vInt==0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;
	
  case OP_ifne:
	if(stack[-1].vInt!=0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;

  case OP_iflt:
	if(stack[-1].vInt<0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;

  case OP_ifge:
	if(stack[-1].vInt>=0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;

  case OP_ifgt:
	if(stack[-1].vInt>0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;
		  
  case OP_ifle:
	if(stack[-1].vInt<=0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;


  case OP_if_icmpeq:
	if(stack[-2].vInt==stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;
	
  case OP_if_icmpne:
	if(stack[-2].vInt!=stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;
	
  case OP_if_icmplt:
	if(stack[-2].vInt<stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;

  case OP_if_icmpge:
	if(stack[-2].vInt>=stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;

  case OP_if_icmpgt:
	if(stack[-2].vInt>stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;

  case OP_if_icmple:
	if(stack[-2].vInt<=stack[-1].vInt) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;

  case OP_if_acmpeq:
	if(stack[-2].obj==stack[-1].obj) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;

  case OP_if_acmpne:
	if(stack[-2].obj!=stack[-1].obj) pc += getInt16(&pc[1]);
	else pc += 3;
	stack -= 2;
	break;
	
  case OP_goto:
	pc += getInt16(&pc[1]);
	break;
		  
  case OP_jsr:
	stack[0].pc = pc + 3;
	stack++;
	pc += getInt16(&pc[1]);
	break;

  case OP_ret:
	pc = var[pc[1]].pc;
	break;

   /* Switch within given range of values, i.e., low..high  */
  case OP_tableswitch: {
	int32_t  key, low, high, default_off;
	uint8_t* npc;
	
	key = stack[-1].vInt;
	npc = pc + 1;
	npc += (4 - ((npc - method->code) % 4)) % 4;
	default_off = getInt32(npc);
	npc += 4;
	low = getInt32(npc);
	npc += 4;
	high = getInt32(npc);
	npc += 4;
	if((key<low) || (key>high)) pc += default_off;
	else pc += getInt32(&npc[(key-low)*4]);
	stack--;
	break;
  }

   /*  Switch with unordered set of values  */
  case OP_lookupswitch: {
	int32_t i, key, low, mid, high, npairs, default_off;
	uint8_t* npc;
	
	key = stack[-1].vInt;
	npc = pc + 1;
	npc += (4 - ((npc - method->code) % 4)) % 4;
	default_off = getInt32(npc);
	npc += 4;
	npairs = getInt32(npc);
	npc += 4;

   /* binary search */
	if(npairs>0){
	  low = 0;
	  high = npairs;
	  while(1){
		mid = (high + low) / 2;
		i = getInt32(npc + (mid * 8));
		if(key==i){
		  pc += getInt32(npc + (mid * 8) + 4); // found
		  break;	
		}
		if(mid==low){
		  pc += default_off; // not found
		  break;
		}
		if(key<i) high = mid;
		else low = mid;
	  }
	} else pc += default_off; // no pairs
	stack--;
	break;
  }
	
  case OP_ireturn:
  case OP_freturn:
  case OP_areturn:
  case OP_return:
	if(*pc!=OP_return){
	  retVal = stack[-1];
	  pushRetVal = 1;
	} else pushRetVal = 0;
	goto L_METHOD_RETURN;

   /* load value from field[idx] to stack */
  case OP_getfield: {
	J_ClassFields* field;
	J_Object obj;

	if(jGetFieldByIndex(jc,&field,getUint16(&pc[1]),jth)!=EOK){
	  err(ERR_FATAL,"Can't found field (%d)",getUint16(&pc[1]));
	}
	
	if(field->isFinal){
	  stack[-1] = field->var;
	  goto _LOCAL_L1;
	}
	obj = stack[-1].obj;
	if(obj==0) GENERATE_EXCEPTION("OP_getfield: NULL POINTER EXCEPTION");
	stack[-1] = vmHeap[obj].ptr[field->var.varOffs+1]; 
   //{ 
   //	JIMP_class *jjc = (JIMP_class*)(heap[obj].__ptr[0].class_ref);
   //	stack[-1] = jjc->fields[ field->var.var_offs +1 ].var;
   // }
  _LOCAL_L1:
	pc += 3;
	break;
  }
	

   /* put value from stack to field[idx] */
  case OP_putfield: {
	J_ClassFields* field;
	J_Object obj;

	if(jGetFieldByIndex(jc,&field,getUint16(&pc[1]),jth)!=EOK){
	  err(ERR_FATAL,"Can't found field (%d)",getUint16(&pc[1]));
	}

	if(field->isFinal){
	 // It's constant also loaded when class loads
	  goto _LOCAL_L2;
	}

	obj = stack[-2].obj;
	if(obj==0) GENERATE_EXCEPTION("OP_putfield: NULL POINTER EXCEPTION");
	vmHeap[obj].ptr[field->var.varOffs+1] = stack[-1];
	
/* 	{  */
/* 	 J_Class* jjc = (J_Class*)(vmHeap[obj].ptr->classRef); */
/* 	//field->var.var_offs, jjc->fields_count, jjc->num_vars );); */
/* printf("PUT FIELD: %d\n",stack[-1]);	  */
/* //	 jjc->fields[field->var.varOffs+1].var = stack[-1];  */
/* 	} */
		  
  _LOCAL_L2:
	stack -= 2;
	pc += 3;
	break;
  }
	
  case OP_getstatic: {
	J_ClassFields* field;
	if(jGetFieldByIndex(jc,&field,getUint16(&pc[1]),jth)!=EOK){
	  err(ERR_FATAL,"Can't found field (%d)",getUint16(&pc[1]));
	}
	stack[0] = field->var;
	stack++;
	pc += 3;
	break;
  }

  case OP_putstatic: {
	J_ClassFields* field;
	if(jGetFieldByIndex(jc,&field,getUint16(&pc[1]),jth)!=EOK){
	  err(ERR_FATAL,"Can't found field (%d)",getUint16(&pc[1]));
	}
	field->var = stack[-1];
	stack--;
	pc += 3;
  }; break;

  
 /**
  *  calls an interface method
  *  Stack: argN, ...., arg1, objref -> ret_value
  */
  case OP_invokeinterface:

   /**
	*  calls a special class method
	*  Stack: argN, ...., arg1, objref -> ret_value
	*/
  case OP_invokespecial:
	
   /**
	*  calls a static method
	*  Stack: argN, ...., arg1 -> ret_value
	*/
  case OP_invokestatic:
	
   /**
	*  calls any method not called by invokestatic, invokespecial, or invokeinterface
	*  Stack: argN, ...., arg1, objref -> ret_value
	*/
  case OP_invokevirtual: {
	J_Class*        ic;
	J_ClassMethods* im;
	J_Object        obj;
	char*           p;
	uint16_t        i, nparam, classIdx, methodIdx, nameTypeIdx, nameIdx, typeIdx;
	J_UtfString     methodName, methodDesc;
	J_Var*          nativeStack;

	methodIdx = getUint16(&pc[1]);
	p = jc->classmem + jc->consts[methodIdx];
	classIdx = getUint16(p+1);

	if(jGetClassByIndex(jc,&ic,classIdx,jth)!=EOK)
	  GENERATE_EXCEPTION("jGetClassByIndex -> METHODERROR");

   /* get method name and description */
	nameTypeIdx = getUint16(p+3);
	nameIdx = getUint16(jc->consts[nameTypeIdx] + jc->classmem + 1); 
	typeIdx = getUint16(jc->consts[nameTypeIdx] + jc->classmem + 3);
	methodName = jGetUtfString(jc,nameIdx);
	methodDesc = jGetUtfString(jc,typeIdx);
	
/* printf(" ******** %s%s ********\n",methodName.str,methodDesc.str); */

   /* try to get method */
	
	if(jGetMethod(ic,methodName,methodDesc,NULL,&im)!=EOK){
/* CLASS_INFO(ic); */
/* printf("Can't get %s%s\n",methodName.str,methodDesc.str); */
	 //GENERATE_EXCEPTION("jGetMethod -> METHODERROR");
	  
	  if(jGetMethod(ic,methodName,methodDesc,&ic,&im)!=EOK)
		GENERATE_EXCEPTION("jGetMethod -> METHODERROR");
	  
	}

   /* how many parameters need to its method */
	nparam = im->paramsCount;
	obj = 0;
	
   /* if this method is not static - top stack must containe to object */
	if(*pc!=OP_invokestatic){
	  nparam++; /* this */
	  obj = stack[-(uint32_t)nparam].obj;
/* { */
/* J_Class* _tt = (J_Class*) vmHeap[stack[-(uint32_t)(nparam)].obj].ptr->classRef; */
/* printf("Stack has : "); CLASS_INFO(_tt); */
/* } */
	  if(obj==0){
		char _t[256];
		sprintf(_t,"NULL_OBJ_ERROR: %s %s",methodName.str,methodDesc.str);
		GENERATE_EXCEPTION(_t);
	  }
	}
	
   /* skip Object <init> method (and pop off object reference) */ 
	if((im->initMethod) && (!ic->numberSuperClasses)){
	  stack -= nparam;
	  pc += 3;
	  break;
	}

   /* If its virtual or interface realisation */
	if((*pc==OP_invokevirtual) || (*pc==OP_invokeinterface)){
	  ic = (J_Class*) vmHeap[obj].ptr[0].classRef;
	  if(!vmHeap[obj].used)
		printf(">>> jGetMeth: [%d]  %s%s :: obj=%d\n",vmHeap[obj].used,methodName.str,methodDesc.str,obj);
	  if(jGetMethod(ic,methodName,methodDesc,&ic,&im)!=EOK){
		char _t[256];
		sprintf(_t,"METHODERROR: obj = %d,  %s %s",obj,methodName.str,methodDesc.str);
		GENERATE_EXCEPTION(_t);
	  }
	}

   /* inc pc */ 
	pc += (*pc==OP_invokeinterface) ? 5 : 3;
	
   /* Check stack overflow */ 
	if(((im->accessFlags) & ACCESS_NATIVE)){
	  if(im->fnative==NULL) GENERATE_EXCEPTION("NOT_FOND_NATIVE_METHODERROR");
	  if(jth->thStackPtr+3+nparam+3 >= jth->thStackSize) GENERATE_EXCEPTION("STACK_OVERFLOW_ERROR");
	} else {
	  if(!im->attributesCount) GENERATE_EXCEPTION("METHODERROR");
	  if((jth->thStackPtr+3+im->maxLocals+im->maxStack+2) >= jth->thStackSize ) GENERATE_EXCEPTION("STACK_OVERFLOW_ERROR");
	}


   /* push params on to stack */
	vmStack[jth->thStackPtr++].pc = pc;
	vmStack[jth->thStackPtr++].refValue = var;
	vmStack[jth->thStackPtr++].refValue = stack - nparam;

   /* store stack pointer for native method */
	nativeStack	= stack;
	for(i=0;i<nparam;i++){
	  vmStack[jth->thStackPtr + nparam - i - 1] = stack[-1];
	  stack--;
	}

	jc = ic;
	method = im;

   /* if method native exec it ... */
	if((method->accessFlags) & ACCESS_NATIVE){
	 /* the active frame for a native method is:
		
	 param 1
	 ...
	 param N
	 num params
	 method pointer
	 class pointer 		   */
	  jth->thStackPtr += nparam;
	  vmStack[jth->thStackPtr++].vInt = nparam;
	  vmStack[jth->thStackPtr++].refValue = method;
	  vmStack[jth->thStackPtr++].refValue = jc;
	  
	 /* call native method */
	  retVal = callJNIMethod(nativeStack-nparam,obj,jc,(J_NativeFunction*) method->fnative,nparam,method->isRetValue);
	  pushRetVal = (method->isRetValue) ? 1 : 0;
		
	 /* return from the method */
	  goto L_METHOD_RETURN;
	}
	goto L_METHOD_INVOKE;

  }; break;


 /* NEW - Create new object 
  *	  Stack: ... -> ..., objectref
  */
  case OP_new: { 
	uint16_t classIdx = getUint16(&pc[1]);
	J_Class* newClass;
	J_Error_t err;
	if((err=jGetClassByIndex(jc,&newClass,classIdx,jth))!=EOK)
	  err(ERR_FATAL,"Can't find class, error [%d] \n",err);
	stack[0].obj = allocateObject(newClass,1);
	stack++;
	pc += 3;
  }; break;


 /*
  * NEWARRAY - Create new array of basic type (int, short, ...) 
  * Stack: ..., count -> ..., arrayref
  * type must be one of T_INT, T_SHORT, ...
  */
  case OP_newarray: { 
	stack[-1].obj = createArray((int32_t)pc[1],stack[-1].vInt);
	pc += 2;
  }; break;

  
 /*
  * ANEWARRAY - Create new array of references 
  * Stack: ..., count -> ..., arrayref
  */
  case OP_anewarray: {
	stack[-1].obj = createArray(1,stack[-1].vInt);
	pc += 3;
  }; break;


  case OP_arraylength: { 
	J_Object obj = stack[-1].obj;
	if(obj==0) GENERATE_EXCEPTION("NULL_ARRAY_ERROR");
	stack[-1].vInt = J_ARRAY_LEN(obj);
	pc++;
  }; break;


 /*
  * INSTANCEOF - Determine if object is of given type 
  *  Stack: ..., objectref -> ..., result
  * - - - - - - - - - - - - - - - - - - - - - - - - - 
  * CHECKCAST - Check whether object is of given type 
  *  Stack: ..., objectref -> ..., objectref
  */
  case OP_instanceof:
  case OP_checkcast: { 
	J_Object obj;
	J_Class* src;
	J_Class* trg;
	J_UtfString nm;
	int result = 0;

	obj = stack[-1].obj;
	if(obj==0){
	  if(*pc==OP_instanceof) stack[-1].vInt = 0;
	  pc += 3;
	  break;
	}

   /* get source and target class references */
	src = J_OBJ_CLASS(obj);
	if(jGetClassByIndex(src,&trg,getUint16(pc+1),jth)!=EOK)
	   err(ERR_FATAL,"[OP_instanceof] : Can't get class by index");
	   
   /* Not array */
	if(trg){ 
	  result = compatible(src,trg,jth);
	} else {
	  nm = jGetClassName(jc,jc->thisClassIdx);
	  if((nm.len>1) && (nm.str[0]=='['))
		result = compatibleArray(obj,nm);
	  else GENERATE_EXCEPTION("TARGET_CLASS_NOT_FOUND");
	}
	
	if(*pc==OP_checkcast){
	  if(!result) GENERATE_EXCEPTION("CLASS_CAST_EXCEPTION"); 
	} else stack[-1].vInt = result;
	
	pc += 3;
  }; break;


  
  case OP_wide: {
	pc++;
	switch(*pc){

	case OP_iload:
	case OP_fload:
	case OP_aload:
	  stack[0] = var[getUint16(&pc[1])];
	  stack++;
	  pc += 3;
	  break;

	case OP_astore:
	case OP_istore:
	case OP_fstore:
	  var[getUint16(&pc[1])] = stack[-1];
	  stack--;
	  pc += 3;
	  break;

	case OP_iinc:
	  var[getUint16(&pc[1])].vInt += getInt16(&pc[3]);
	  pc += 5;
	  break;

	case OP_ret:
	  pc = var[getUint16(&pc[1])].pc;
	  break;
	}
	break;
  }

	
  case OP_multianewarray: {
	uint16_t clsIdx;
	J_UtfString clsName;
	uint32_t  ndim;
	char* p, *cstr;

   /* Get array description */
	p = jc->classmem + jc->consts[getUint16(&pc[1])] + 1;
	clsIdx = getUint16(p);
	clsName = jGetUtfString(jc,clsIdx);
	
	ndim = (uint32_t) pc[3];
	cstr = &clsName.str[1];
	stack -= ndim;
	stack[0].obj = createMultiArray(ndim,cstr,stack);
	stack++;
	pc += 4;
	break;
  }

	
  case OP_ifnull: { 
	if(stack[-1].obj==0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;
  }

	
  case OP_ifnonnull: { 
	if(stack[-1].obj!=0) pc += getInt16(&pc[1]);
	else pc += 3;
	stack--;
	break;
  }

	
  case OP_goto_w: { 
	pc += getInt32(&pc[1]);
	break;
  }

	
  case OP_jsr_w: { 
	stack[0].pc = pc + 5;
	pc += getInt32(&pc[1]);
	stack++;
	break;
  }

 /* Thread implementation */
  case OP_monitorenter: {
/* 	JIMP_obj obj = stack[-1].obj; */
/* 	if( !obj ) goto __NULL_OBJ_ERROR; */
/* 	__thread_lock( obj, jts ); */
	stack--;
	pc++;
	break;
  }
	
  case OP_monitorexit: {
/* 	JIMP_obj obj = stack[-1].obj; */
/* 	if( !obj ) goto __NULL_OBJ_ERROR; */
/* 	__thread_unlock( obj, jts ); */
	stack--;
	pc++;	 
	break;
  }

  /* Not implemented opcodes */ 
  case OP_lconst_0:
  case OP_lconst_1:
  case OP_dconst_0:
  case OP_dconst_1:
  case OP_lload:
  case OP_dload:
  case OP_lload_0:
  case OP_lload_1:
  case OP_lload_2:
  case OP_lload_3:
  case OP_dload_0:
  case OP_dload_1:
  case OP_dload_2:
  case OP_dload_3:
  case OP_laload:
  case OP_daload:
  case OP_lstore:
  case OP_dstore:
  case OP_lstore_0:
  case OP_lstore_1:
  case OP_lstore_2:
  case OP_lstore_3:
  case OP_dstore_0:
  case OP_dstore_1:
  case OP_dstore_2:
  case OP_dstore_3:
  case OP_lastore:
  case OP_dastore:
  case OP_ladd:
  case OP_dadd:
  case OP_lsub:
  case OP_dsub:
  case OP_lmul:
  case OP_dmul:
  case OP_ldiv:
  case OP_ddiv:
  case OP_lrem:
  case OP_drem:
  case OP_lneg:
  case OP_dneg:
  case OP_lshl:
  case OP_lshr:
  case OP_lushr:
  case OP_land:
  case OP_lor:
  case OP_lxor:
  case OP_i2l:
  case OP_i2d:
  case OP_l2i:
  case OP_l2f:
  case OP_l2d:
  case OP_f2l:
  case OP_f2d:
  case OP_d2i:
  case OP_d2l:
  case OP_d2f:
  case OP_lcmp:
  case OP_dcmpl:
  case OP_dcmpg:
  case OP_lreturn:
  case OP_dreturn:{
	GENERATE_EXCEPTION("CANNT_EXEC_LONG_OR_DOUBLE_OPERATIONS");
	break;
  }

   /* ATHROW - Throw exception 
	* Stack: ..., objectref -> [undefined]
	*/
  case OP_athrow: {
	uint16_t ptr;
	J_Object eObj;
	J_Class* exception;
	J_UtfString exceptName;
	
	eObj = stack[-1].obj;
	exception = vmHeap[eObj].ptr->classRef;
	exceptName = jGetClassName(exception,exception->thisClassIdx);
	
  L_TRY_AGAINE:
	ptr = pc-method->code;
	
dbg(DBG_EXCEPTIONS,"Stack objref = %d, current pc = %d, classIdx = %d, name = %s\n",eObj,
		   ptr,exception->thisClassIdx,exceptName.str);
	
	{
	  int k;
	  uint8_t* p;
	  for(p = method->excTable,k=0;k<method->excTableLength;k++,p+=8*k){
		uint16_t start = getUint16(p);
		uint16_t end = getUint16(p+2);
		uint16_t pcHandler = getUint16(p+4);
		uint16_t catchType = getUint16(p+6);
		J_Class* hE;
		J_UtfString eName;
		
		if(jGetClassByIndex(jc,&hE,catchType,jth)!=EOK){
		  continue;
		}

		eName = jGetClassName(hE,hE->thisClassIdx);
dbg(DBG_EXCEPTIONS,"from %d to %d -> %d (%d --> %s [classIdx = %d])\n",start,end,pcHandler,catchType,eName.str,hE->thisClassIdx);
		
	   /* check is it our exception ? */
        if(ptr <= end && ptr >= start && exceptName.len == eName.len &&
		   !strncmp(exceptName.str,eName.str,eName.len)){
dbg(DBG_EXCEPTIONS," ===> FOUND EXCEPTION HANDLER <=== \n");
		 /* We need place to stack generated exception object */
		  stack[0].obj =  eObj;
		  stack++;
		  pc = method->code + pcHandler;
		  goto STEP;
		}
		
	  }
	  
	 /* if exceptions not found make return from the current function */
	  if((method->accessFlags & ACCESS_NATIVE)>0){
		jth->thStackPtr -= 2;
		jth->thStackPtr -= vmStack[--jth->thStackPtr].vInt;
	  }
	  else
		jth->thStackPtr -= method->maxLocals + method->maxStack + 2;

	 /* fully completed execution */
	  if(jth->thStackPtr == baseFramePtr + 3){
		jth->thStackPtr = baseFramePtr;
		GENERATE_EXCEPTION("UNHANDLED EXCEPTION OCCURED");
	  }
	  
	  stack = (J_Var*) vmStack[--jth->thStackPtr].refValue;
	  var = (J_Var*) vmStack[--jth->thStackPtr].refValue;
	  pc = vmStack[--jth->thStackPtr].pc;
	  jc = (J_Class*) vmStack[jth->thStackPtr-1].refValue;
	  method = (J_ClassMethods*) vmStack[jth->thStackPtr-2].refValue;
	  goto L_TRY_AGAINE;
  	}
	break;
  }
	
  default:
	GENERATE_EXCEPTION("BAD_OPCODE");
  }
  goto STEP;
  
 L_METHOD_RETURN:
 /* pop frame and restore state */
  if((method->accessFlags & ACCESS_NATIVE)>0){
	jth->thStackPtr -= 2;
	jth->thStackPtr -= vmStack[--jth->thStackPtr].vInt;
  } else jth->thStackPtr -= method->maxLocals + method->maxStack + 2;

 /* fully completed execution */
  if(jth->thStackPtr == baseFramePtr + 3){
	jth->thStackPtr = baseFramePtr;
	return EOK;
  }
  stack = (J_Var*) vmStack[--jth->thStackPtr].refValue;
  if(pushRetVal){
	stack[0] = retVal;
	stack++;
  }
  var = (J_Var*) vmStack[--jth->thStackPtr].refValue;
  pc = vmStack[--jth->thStackPtr].pc;
  jc = (J_Class*) vmStack[jth->thStackPtr-1].refValue;
  method = (J_ClassMethods*) vmStack[jth->thStackPtr-2].refValue;
  goto STEP;
  
  
  return EOK;
}


// Main thread make available for all (temporary debug)
J_Thread* mainThread;

/**
 *  Test main function
 */
int main(int argc,char *argv[]){
  J_Class*  c = NULL;
  J_Error_t e = EOK;
  J_Var     p;
  J_ClassMethods* method;
  J_Object  jargv, *jargs;
  int i = 0;


 /* Set initial points for runtime exceptions */
  SET_RUNTIMEXCEPTION_POINT;

  if(!(argc>=2 && argv[1]!=NULL))
	THROW(ERR_FATAL,"Too few arguments ...",0);
  /* 	err(ERR_FATAL,"Too few arguments ...\n"); */
  
 /* Init VM */
  if((e=initVM())!=EOK)
	err(ERR_FATAL,"Can't initialize virtual machine: %d\n",e);

 /* Create main thread context */
  mainThread = allocateThreadMemory(THREAD_STACK_SIZE);
  
OFF_DEBUG_FILTER(DBG_TEMPORARY);
OFF_DEBUG_FILTER(DBG_LEV_1);
OFF_DEBUG_FILTER(DBG_EXCEPTIONS);
OFF_DEBUG_FILTER(DBG_DISASM);
  
 /* Load preloaded classes */
  for(i=0;preloadedClasses[i]!=NULL;i++){
	if((e=jGetClass(&c,jUtfCreate(preloadedClasses[i]),mainThread))!=EOK)
	  err(ERR_FATAL,"Can't find class %s : %d\n",preloadedClasses[i],e);
  }

 /* Load class */
  if((e=jGetClass(&c,jUtfCreate(argv[1]),mainThread))!=EOK)
	err(ERR_FATAL,"Can't find class: %d\n",e);

 /* push it onto extended stack */
  jExtPushObj(c->obj);
  
 /*
  param[0].obj = c->obj;
  if(jGetMethod(c,jUtfCreate("<init>"),jUtfCreate("()V"),NULL,&method)==EOK)
	if(method!=NULL){
	  jExecMethod(c,method,param,1,mainThread);
	}
 */

 /* Main method: some fixes from 21-mar-2006 */
    jargv = createArray(1,argc-2);
    jargs = (J_Object*) J_ARRAY_START(jargv);
    for(i=2;i<argc;i++)
      jargs[i-2] = createString(argv[i]);
    p.obj = jargv;

OFF_DEBUG_FILTER(DBG_DISASM);
	if(jGetMethod(c,jUtfCreate("main"),jUtfCreate("([Ljava/lang/String;)V"),NULL,&method)==EOK){
	  if(method!=NULL)
		jExecMethod(c,method,&p,argc-1,mainThread);
	} else err(ERR_FATAL,"method main(String[]) not found\n" );

 /*
   TODO List:
   ----------------------------------------------------------------------------------------------------
   1). jLoadNativeFunction - OK /12-feb-2005/
   2). jGetMethod          - OK
   3). jExecMethod         - OK /21-jun-2005/ (провести еще больше тестов)
   4). Продумать закидывание уже загруженных классов в хэш - OK
   5). Обработка исключений - OK /24-jun-2005/ (провести еще тесты)
   6). Проблема с вызовом виртуальных ф-ций - OK /28-jun-2005/ (еще провести оптимизацию разбора invoke...)
   7). Сборка мусора        - OK /20-mar-2006/
   8). Нити
   9). Reflection
   10). Exception generation from jExecMethod (if some errors was occured)
   11). Реализация Long & Double 
   12). Связка с jimp 
   ----------------------------------------------------------------------------------------------------
  */

/*   SET_DEBUG_FILTER(DBG_ALL); */
  
  freeThreadMemory(mainThread);
  return 0;
}


