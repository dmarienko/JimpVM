/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Interpretator and main() function
*/

#include "vm.h"
#include "vmmemory.h"
#include "opcodes.h"
#include "thread.h"

int        JIMP_vm_initialize = 0;

JIMP_class *__STRINGCLASS;
JIMP_class *__OBJECTCLASS;

int JIMP_vm_init( uint32 sz_class_mem, uint32 sz_heap_mem, uint32 sz_stack )
{
	if( JIMP_vm_initialize ) return 1;

   /* Allocate memory for class area 
    *  (* OS depended *) 
    */
   	__class_memory = ( uchar* ) malloc( sz_class_mem );
   	if( !__class_memory ) { 
   	 printf("vm_init> not enought memory for class area\n");
   	 return 0;
   	}
   	__used_class_memory = 0;
    __sz_class_memory = sz_class_mem;

   /* Allocate vm heap */
	if( !alloc_heap_memory( sz_heap_mem ) ) return 0;

   /* Allocate vm stack */
	if( !alloc_vm_stack( sz_stack ) ) return 0;

    JIMP_vm_initialize = 1;
	return 1;
}

void JIMP_vm_free()
{
	if( __class_memory ) free( __class_memory );
	if( heap ) free( heap );
    if( VM_stack ) free( VM_stack );
    if( EX_stack ) free( EX_stack );
}

/* - - - - - - - - - - - -
 * Execute class method
 */
void JIMP_exec_method( JIMP_class *jc, JIMP_class_methods *method, 
						JIMP_var param[], uint32 numparams, JIMP_thread_s *jts )
{
	register JIMP_var *var;
	register JIMP_var *stack;
	register uchar 	  *pc;
	uint32 			  base_frame_ptr;
	JIMP_var 		  ret_val;
	uint16			  push_ret_val;

   /* For fast access :) */	
	JIMP_var 		  *VM_stack = jts->VM_stack;
	uint32			  VM_stack_ptr = jts->VM_stack_ptr;
	uint32			  VM_stack_size = jts->VM_stack_size;

    push_ret_val = 0;

   /* cannot exec native */
	if( method->access_flags & ACCESS_NATIVE ) return;

   /* have'nt code section */
	if( method->code_length == 0 ) return;

	base_frame_ptr = VM_stack_ptr;

   /* check stack */
	if( VM_stack_ptr+method->max_locals+method->max_stack+5 >= VM_stack_size )
	 return 0; /* Stack overflow - try inflate ??? */

	VM_stack[ VM_stack_ptr++ ].pc = 0;
	VM_stack[ VM_stack_ptr++ ].ref_value = 0;
	VM_stack[ VM_stack_ptr++ ].ref_value = 0;

   /* push params into local vars of frame */
    while( numparams > 0 )
    {
      numparams--;
      VM_stack[ VM_stack_ptr + numparams ] = param[ numparams ];
    }

 __METHODINVOKE:
 	/*
	 * push active stack frame:
	 *
	 * local var 1
	 * ...
	 * local var N
	 * local stack 1
	 * ...
	 * local stack N
	 * method pointer
	 * class pointer
 	 */
	
	var = &VM_stack[ VM_stack_ptr ];
	VM_stack_ptr += method->max_locals;
	stack = &VM_stack[ VM_stack_ptr ];
	VM_stack_ptr += method->max_stack;
	VM_stack[ VM_stack_ptr++ ].ref_value = method;
	VM_stack[ VM_stack_ptr++ ].ref_value = jc;
	pc = method->code;

 /* Interpreter */
 STEP:               
 DBG(100,{printf("<%d>\t\t%s  ",*pc, __ss[*pc]); hex_dump(pc+1,3); printf("\n");});
 	switch( *pc ) 
 	{
    	case OP_nop:
		  pc++;
	  	  break;

        case OP_aconst_null:
		  stack[0].obj = 0;
		  stack++;
          pc++;
          break;

		case OP_iconst_m1:
		case OP_iconst_0:
		case OP_iconst_1:
		case OP_iconst_2:
		case OP_iconst_3:
		case OP_iconst_4:
		case OP_iconst_5:
		  stack[0].int_value = ( *pc - OP_iconst_0 );
		  stack++;
		  pc++;
		  break;

		case OP_fconst_0:
		  stack[0].float_value = 0.0f;
		  stack++;
		  pc++;
		  break;

		case OP_fconst_1:
		  stack[0].float_value = 1.0f;
		  stack++;
		  pc++;
		  break;

		case OP_fconst_2:
		  stack[0].float_value = 2.0f;
		  stack++;
		  pc++;
		  break;

		case OP_bipush:
		  stack[0].int_value = ((signed char *)pc)[1];
		  stack++;
		  pc += 2;
		  break;

		case OP_sipush:
		  stack[0].int_value = get_int16( &pc[1] );
		  stack++;
		  pc += 3;
		  break;

		case OP_ldc:
		  *stack = get_constant_value( jc, (uint16) pc[1] );
//view_const( jc, (uint16)pc[1]);
		  stack++;
		  pc += 2;
		  break;

		case OP_ldc_w:
		  *stack = get_constant_value( jc, get_uint16( &pc[1] ) );
		  stack++;
		  pc += 3; 
		  break;

   		case OP_ldc2_w:
   		  // *stack = get_constant_value( jc, get_uint16(pc + 1) );
          // *(++stack) = get_constant_value( jc, get_uint16(pc + 1) );
   		  stack+=2;
   		  pc+= 3;
   		  break; 

		case OP_iload:
		case OP_fload:
		case OP_aload:
		  *stack = var[ pc[1] ];
		  stack++;
		  pc += 2;
		  break;

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

		case OP_iaload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;
		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;

		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  //stack[-2].int_value = ((int32*)&(obj_ptr[3]))[i];
		  obj_ptr = JIMP_ARRAY_START(obj);
		  stack[-2].int_value = obj_ptr[i].int_value;

		  stack--;
		  pc++;
		  break;
	    }

		case OP_saload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  stack[-2].int_value = ((int32*)&(obj_ptr[3]))[i];

	  	  stack--;
		  pc++;
		  break;
	    }

		case OP_faload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  stack[-2].float_value = ((float32*)&(obj_ptr[3]))[i];

	  	  stack--;
		  pc++;
		  break;
		}

		case OP_aaload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;

		 /* пока не могу понять в чем отличие но для n-мерных массивов
		    работает первое */
		  //if(JIMP_ARRAY_TYPE(obj)==2){
		    obj_ptr = JIMP_ARRAY_START(obj);
		    stack[-2].obj = obj_ptr[i].obj;
		  //}
		  //else stack[-2].obj = ((JIMP_obj*)&(obj_ptr[3]))[i];

	  	  stack--;
		  pc++;
		  break;
		}

		case OP_baload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
	 	  char *barray;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  barray = (char*) JIMP_ARRAY_START(obj);	  
		  stack[-2].int_value = (char) barray[i];
		  //stack[-2].int_value = (int32)((char*)&(obj_ptr[3]))[i];

	  	  stack--;
		  pc++;
		  break;
		}

		case OP_caload: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-2].obj;
		  i = stack[-1].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  obj_ptr = JIMP_ARRAY_START(obj);
		  stack[-2].int_value = obj_ptr[i].int_value; 
		  //stack[-2].int_value = (int32)((uint16*)&(obj_ptr[3]))[i];

	  	  stack--;
		  pc++;
		  break;
		}

		case OP_astore:
		case OP_istore:
		case OP_fstore:
		  stack--;
		  var[pc[1]] = *stack;
		  pc += 2;
		  break;

		case OP_istore_0:
		case OP_istore_1:
		case OP_istore_2:
		case OP_istore_3:
		  stack--;
		  var[*pc - OP_istore_0] = *stack;
		  pc++;
		  break;

		case OP_fstore_0:
		case OP_fstore_1:
		case OP_fstore_2:
		case OP_fstore_3:
		  stack--;
		  var[*pc - OP_fstore_0] = *stack;
		  pc++;
		  break;

		case OP_astore_0:
		case OP_astore_1:
		case OP_astore_2:		
		case OP_astore_3:
		  stack--;
		  var[*pc - OP_astore_0] = *stack;
		  pc++;
		  break;


		case OP_iastore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  //((int32 *)&(obj_ptr[3]))[i] = stack[-1].int_value;
		  obj_ptr = JIMP_ARRAY_START(obj);
		  obj_ptr[i].int_value = stack[-1].int_value;

		  stack -= 3;
		  pc++;
		  break;
		}

		case OP_sastore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );
//		  if( (i < 0) || (i >= obj_ptr[2].int_value) ) goto __INDEX_RANGE_ERROR;
		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  ((int16 *)&(obj_ptr[3]))[i] = (int16)stack[-1].int_value;

		  stack -= 3;
		  pc++;
		  break;
		}

		case OP_fastore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );
//		  if( (i < 0) || (i >= obj_ptr[2].int_value) ) goto __INDEX_RANGE_ERROR;
		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  ((float32 *)&(obj_ptr[3]))[i] = (float32)stack[-1].float_value;

		  stack -= 3;
		  pc++;
		  break;
		}

		case OP_aastore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  //((JIMP_obj *)&(obj_ptr[3]))[i] = (JIMP_obj)stack[-1].obj;
		  obj_ptr = JIMP_ARRAY_START(obj);
		  obj_ptr[i].obj = stack[-1].obj;

		  stack -= 3;
		  pc++;
		  break;
		}

		case OP_bastore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  char *barray;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;
		  barray = (char*)JIMP_ARRAY_START(obj);
		  barray[i] = (char) stack[-1].int_value;
		 // ((char *)&(obj_ptr[3]))[i] = (char)stack[-1].int_value;

		  stack -= 3;
		  pc++;
		  break;
		}

		case OP_castore: {

		  JIMP_obj obj;
		  JIMP_var *obj_ptr;
		  int32    i;

		  obj = stack[-3].obj;
		  i = stack[-2].int_value;

		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  obj_ptr = JIMP_OBJ_CLASS( obj );

		  if( (i < 0) || (i >= JIMP_ARRAY_LEN(obj)) ) goto __INDEX_RANGE_ERROR;

		  obj_ptr = JIMP_ARRAY_START(obj);
		  obj_ptr[i].int_value = stack[-1].int_value;

		  //((uint16 *)&(obj_ptr[3]))[i] = (uint16)stack[-1].int_value;

		  stack -= 3;
		  pc++;
		  break;
		}
		  

		case OP_pop:
		  stack--;
		  pc++;
		  break;

		case OP_pop2:
		  stack -= 2;
		  pc++;
		  break;

		case OP_dup:
		  stack[0] = stack[-1];
		  stack++;
		  pc++;
		  break;

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

		case OP_dup2_x1:
		  stack[1] = stack[-1];
		  stack[0] = stack[-2];
		  stack[-1] = stack[-3];
		  stack[-2] = stack[1];
		  stack[-3] = stack[0];
		  stack += 2;
		  pc++;
		  break;

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

		  JIMP_var v;
		  v = stack[-2];
		  stack[-2] = stack[-1];
		  stack[-1] = v;
		  pc++;
		  break;
		}

		case OP_iadd:
		  stack[-2].int_value += stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_fadd:
		  stack[-2].float_value += stack[-1].float_value;
		  stack--;
		  pc++;
		  break;

		case OP_isub:
		  stack[-2].int_value -= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_fsub:
		  stack[-2].float_value -= stack[-1].float_value;
		  stack--;
		  pc++;
		  break;

		case OP_imul:
		  stack[-2].int_value *= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_fmul:
		  stack[-2].float_value *= stack[-1].float_value;
		  stack--;
		  pc++;
		  break;

		case OP_idiv:
		  if (stack[-1].int_value == 0) goto __DIV_BY_ZERO_ERROR;
		  stack[-2].int_value /= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_fdiv:
		  if (stack[-1].float_value == 0.0f) goto __DIV_BY_ZERO_ERROR;
		  stack[-2].float_value /= stack[-1].float_value;
		  stack--;
		  pc++;
		  break;

		case OP_irem:
		  if (stack[-1].int_value == 0) goto __DIV_BY_ZERO_ERROR;
		  stack[-2].int_value = stack[-2].int_value % stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_frem: { /* Может остаток как-то попроще ??? */
		  float32 f;

		  if (stack[-1].float_value == 0.0f) goto __DIV_BY_ZERO_ERROR;
		  f = stack[-2].float_value / stack[-1].float_value;
		  f = (float32)((int32)f);
		  f *= stack[-1].float_value;
		  stack[-2].float_value = stack[-2].float_value - f;
		  stack--;
		  pc++;
		  break;
		}

		case OP_ineg:
		  stack[-1].int_value = - stack[-1].int_value;
		  pc++;
		  break;

		case OP_fneg:
		  stack[-1].float_value = - stack[-1].float_value;
		  pc++;
		  break;

		case OP_ishl:
		  stack[-2].int_value = stack[-2].int_value << stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_ishr:
		  stack[-2].int_value = stack[-2].int_value >> stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_iushr: {

		  int32 i;

		  i = stack[-1].int_value;
		  if( stack[-2].int_value >= 0 ) 
		    stack[-2].int_value = stack[-2].int_value >> i;
			else {
			  stack[-2].int_value = stack[-2].int_value >> i;
		      if( i >= 0 ) stack[-2].int_value += (int32)2 << (31 - i);
				else
			      stack[-2].int_value += (int32)2 << ((- i) + 1);
		    }
		   stack--;
		  pc++;
		  break;
		}

		case OP_iand:
		  stack[-2].int_value &= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_ior:
		  stack[-2].int_value |= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_ixor:
		  stack[-2].int_value ^= stack[-1].int_value;
		  stack--;
		  pc++;
		  break;

		case OP_iinc:
		  var[pc[1]].int_value += (signed char)pc[2];
		  pc += 3;
		  break;

		case OP_i2f:
		  stack[-1].float_value = (float32)stack[-1].int_value;
		  pc++;
		  break;

		case OP_f2i: {

		  float32 f;
		  f = stack[-1].float_value;
		  if (f > 2147483647.0) stack[-1].int_value = 0x7FFFFFFF;
			else if (f < -2147483648.0) stack[-1].int_value = 0x80000000;
				  else stack[-1].int_value = (int32)f;
		  pc++;
		  break;
		}

		case OP_i2b:
		  stack[-1].int_value = (int32)((signed char)(stack[-1].int_value & 0xFF));
		  pc++;
		  break;

		case OP_i2c:
		  stack[-1].int_value = (int32)((uint16)(stack[-1].int_value & 0xFFFF));
		  pc++;
		  break;

		case OP_i2s:
		  stack[-1].int_value = (int32)((int16)(stack[-1].int_value & 0xFFFF));
		  pc++;
		  break;

		case OP_fcmpl:
		case OP_fcmpg: {

		  float32 f;

		// NOTE: NaN values are currently not supported - NaN in either
		// value should return 1 or 0 depending on the opcode
		  f = stack[-2].float_value - stack[-1].float_value;
		  if (f > 0.0f) stack[-2].int_value = 1;
			else if (f < 0.0f) stack[-2].int_value = -1;
					else stack[-2].int_value = 0;
		  stack--;
		  pc++;
		  break;
		}


		case OP_ifeq:
		  if( stack[-1].int_value == 0 ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack--;
		  break;

		case OP_ifne:
		  if( stack[-1].int_value != 0 ) pc += get_int16( &pc[1] );
		 	else pc += 3;
		  stack--;
		  break;

		case OP_iflt:
		  if( stack[-1].int_value < 0 ) pc += get_int16( &pc[1] );
			else pc += 3;
		  stack--;
		  break;

		case OP_ifge:
		  if( stack[-1].int_value >= 0 ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack--;
		  break;

		case OP_ifgt:
		  if( stack[-1].int_value > 0 ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack--;
		  break;

		case OP_ifle:
		  if( stack[-1].int_value <= 0 ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack--;
		  break;


		case OP_if_icmpeq:
		  if( stack[-2].int_value == stack[-1].int_value ) pc += get_int16( &pc[1] );
 		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_icmpne:
		  if( stack[-2].int_value != stack[-1].int_value ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_icmplt:
		  if( stack[-2].int_value < stack[-1].int_value ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_icmpge:
		  if( stack[-2].int_value >= stack[-1].int_value ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_icmpgt:
		  if( stack[-2].int_value > stack[-1].int_value ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_icmple:
		  if( stack[-2].int_value <= stack[-1].int_value ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_acmpeq:
		  if( stack[-2].obj == stack[-1].obj ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_if_acmpne:
		  if( stack[-2].obj != stack[-1].obj ) pc += get_int16( &pc[1] );
		    else pc += 3;
		  stack -= 2;
		  break;

		case OP_goto:
		  pc += get_int16( &pc[1] );
		  break;

		case OP_jsr:
		  stack[0].pc = pc + 3;
		  stack++;
		  pc += get_int16( &pc[1] );
		  break;

		case OP_ret:
		  pc = var[ pc[1] ].pc;
		  break;

		case OP_tableswitch: {
		  int32 key, low, high, default_off;
		  uchar *npc;

		  key = stack[-1].int_value;
		  npc = pc + 1;
		  npc += (4 - ((npc - method->code) % 4)) % 4;
		  default_off = get_int32( npc );
		  npc += 4;
		  low = get_int32( npc );
		  npc += 4;
		  high = get_int32( npc );
		  npc += 4;
		  if( (key < low) || (key > high) ) pc += default_off;
			else pc += get_int32( &npc[(key - low) * 4] );
		  stack--;
		  break;
		}


		case OP_lookupswitch: {
		  int32 i, key, low, mid, high, npairs, default_off;
		  uchar *npc;

		  key = stack[-1].int_value;
		  npc = pc + 1;
		  npc += (4 - ((npc - method->code) % 4)) % 4;
		  default_off = get_int32(npc);
		  npc += 4;
		  npairs = get_int32(npc);
		  npc += 4;

		/* binary search */
		  if (npairs > 0) {
		    low = 0;
		    high = npairs;
		    while (1) {
			  mid = (high + low) / 2;
			  i = get_int32(npc + (mid * 8));
			  if (key == i) {
			    pc += get_int32(npc + (mid * 8) + 4); // found
			    break;	
			  }
			  if (mid == low) {
			    pc += default_off; // not found
			    break;
			  }
			  if (key < i) high = mid;
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
		  if( *pc != OP_return ) {
		    ret_val = stack[-1];
		    push_ret_val = 1;
		  }
		  else
			push_ret_val = 0;
		  goto __METHODRETURN;

	  /* field[idx] -> stack */
		case OP_getfield: {

		  JIMP_class_fields *field;
		  JIMP_obj obj;

		  field = JIMP_get_field_by_idx( jc, get_uint16( &pc[1] ) );
		  if( !field )  goto __ERROR;

/* В конструкторе класса заполнение final переменных (констант)
   происходит с использованием оператора putfield хотя они уже загружены в
   class loader'e
   Это для jdk 1.1.2

   Ставим принудительную проверку
 */
		if( field->is_final ) {
		 stack[-1] =  field->var;
		goto _e1;
		}
		  obj = stack[-1].obj;
		  if( obj == 0 ) goto __NULL_OBJ_ERROR;
		   stack[-1] = heap[obj].__ptr[ field->var.var_offs + 1]; 
		  //{ 
		  //	JIMP_class *jjc = (JIMP_class*)(heap[obj].__ptr[0].class_ref);
		//	stack[-1] = jjc->fields[ field->var.var_offs +1 ].var;
		 // }
_e1:
		  pc += 3;
		  break;
		}
		  

	  /* stack -> field[idx] */
		case OP_putfield: {
		  JIMP_class_fields *field;
		  JIMP_obj obj;

		  field = JIMP_get_field_by_idx( jc, get_uint16( &pc[1] ) );
		  if( !field )  goto __ERROR;

/* В конструкторе класса заполнение final переменных (констант)
   происходит с использованием оператора putfield хотя они уже загружены в
   class loader'e
   Это для jdk 1.1.2

   Ставим принудительную проверку
 */

		if( field->is_final ) {
		 // По идее такая константа уже загружена на этапе загрузки класса
		goto _e2;
		}

		  obj = stack[-2].obj;
		  if( obj == 0 ) goto __NULL_OBJ_ERROR;
		   heap[obj].__ptr[ field->var.var_offs + 1] = stack[-1];
		  //{ 
		   //	JIMP_class *jjc = (JIMP_class*)(heap[obj].__ptr[0].class_ref);
			//DBG(101,printf("> putfield> offs=%d, jjc->fields_count=%d, jjc->num_vars=%d\n",
			//	field->var.var_offs, jjc->fields_count, jjc->num_vars ););
		//	jjc->fields[ field->var.var_offs + 1 ].var = stack[-1];
		 // }
		  
_e2:
		  stack -= 2;
		  pc += 3;
		  break;
		}


		case OP_getstatic: {

		  JIMP_class_fields *field;

		  field = JIMP_get_field_by_idx( jc, get_uint16( &pc[1] ) );
		  if( !field )  goto __ERROR;

		  stack[0] = field->var;
		  stack++;
		  pc += 3;
		  break;
		}

		case OP_putstatic: {

		  JIMP_class_fields *field;

		  field = JIMP_get_field_by_idx( jc, get_uint16( &pc[1] ) );
		  if( !field )  goto __ERROR;

		  field->var = stack[-1];
		  stack--;
		  pc += 3;
		  break;
		}
/*O+O*/
		case OP_invokeinterface:
		case OP_invokestatic:
		case OP_invokevirtual:
		case OP_invokespecial: {

		  JIMP_class *ic;
		  JIMP_class_methods *im;
		  JIMP_obj   obj;
          char *p;
		  uint16  i, nparam, class_idx, method_idx, name_type_idx, n_idx, t_idx;
		  JIMP_utf_string met_name, met_desc;

		/* Стандартный ритуал получения class_idx и т.д. 
		   для некоей константы - это надо вынести и оформить отдельно */
		  method_idx = get_uint16( &pc[1] );
/* d e b u g */
	DBG(2,printf(" method_idx = %d\n",  method_idx ));
		  p = jc->consts[method_idx] + jc->classmem;
    	  class_idx = get_uint16( p + 1 );
/* d e b u g */
	DBG(2,printf(" class_idx = %d\n", class_idx ));
		  ic = JIMP_get_class_by_index( jc, class_idx );
		  if( ic == NULL ) goto __METHODERROR;

		  name_type_idx = get_uint16( p + 3 );
    	  n_idx = get_uint16( jc->consts[name_type_idx] + jc->classmem + 1 ); 
    	  t_idx = get_uint16( jc->consts[name_type_idx] + jc->classmem + 3 );
		  met_name = get_UTF_string( jc, n_idx );
		  met_desc = get_UTF_string( jc, t_idx );
/* d e b u g */
	DBG(100,{ char __cc[128]; memset(__cc,0,128); strncpy(__cc,met_name.str,met_name.len);
		printf(" method -> = %s", __cc ); memset(__cc,0,128);
		strncpy(__cc,met_desc.str,met_desc.len);
		printf("%s\n", __cc );}; 
		);

		  im = JIMP_get_method( ic, met_name, met_desc, NULL );
		  if( im == NULL ) goto __METHODERROR;

/* d e b u g */
	DBG(2,printf(" method %s was finded\n", met_name.str ));

		  nparam = im->params_count;
		  obj = 0;
	DBG(2,printf("-> nparam=%d\n", nparam ));

		  /* Смотрим - если метод не static - на стеке должен лежать this */
		  if( *pc != OP_invokestatic ) {
            nparam++; /* this */
		    obj = stack[- (uint32)nparam].obj;
	DBG(2,printf("-> obj=%d\n", obj ));
	   	    if( obj == 0 ) goto __NULL_OBJ_ERROR;
		  }

		 /* пропускаем Object <init> метод (and pop off object reference) */ 
		  if( (im->init_method) && (!ic->number_super_classes) ) {
		    stack -= nparam;
		    pc += 3;
	DBG(100,printf("-> It's <init> method\n"));
		    break;
		  }

		 /* Если метод виртуальный или интерфейс */ 
		  if( (*pc==OP_invokevirtual) || (*pc==OP_invokeinterface) ) {
		    ic = (JIMP_class*) heap[obj].__ptr[0].class_ref;
		    im = JIMP_get_method( ic, met_name, met_desc, &ic );
		    if( !im ) goto __METHODERROR;
		  }

		 /* inc pc */ 
		  if( *pc == OP_invokeinterface ) pc += 5;
			else pc += 3;

		 /* Check stack overflow */ 
		  if(((im->access_flags) & ACCESS_NATIVE)) {
		    if( im->fnative == NULL ) goto __METHODERROR;
	  	    if( (VM_stack_ptr+3+nparam+3) >= VM_stack_size ) goto __STACK_OVERFLOW_ERROR;
		  }
		   else {
		     if( !im->attributes_count )  goto __METHODERROR;
	  	     if( (VM_stack_ptr+3+im->max_locals+im->max_stack+2) >= VM_stack_size ) goto __STACK_OVERFLOW_ERROR;
		   }

		 /* push params on to stack */
		  VM_stack[VM_stack_ptr++].pc = pc;
		  VM_stack[VM_stack_ptr++].ref_value = var;
		  VM_stack[VM_stack_ptr++].ref_value = stack - nparam;

		  for (i = 0; i < nparam; i++) {
			VM_stack[VM_stack_ptr + nparam - i - 1] = stack[-1];
			stack--;
		  }

		  jc = ic;
		  method = im;

		 /* if method native exec it ... */
		  if(((method->access_flags) & ACCESS_NATIVE)) {
		  /* the active frame for a native method is:
		  
		     param 1
		     ...
		     param N
		     num params
		     method pointer
		     class pointer 		   */ 
		    VM_stack_ptr += nparam;
		    VM_stack[VM_stack_ptr++].int_value = nparam;
		    VM_stack[VM_stack_ptr++].ref_value = method;
		    VM_stack[VM_stack_ptr++].ref_value = jc;

		   /* exec ... */
		    ret_val = method->fnative( stack );

			if( method->is_ret_value ) push_ret_val = 1;
		     else push_ret_val = 0;

		   /* return from the method */
			goto __METHODRETURN;
		  }

		  goto __METHODINVOKE;

__METHODERROR:
		  printf("i> Can not find method ");
		  if( met_name.len > 0 ) printf("%s%s ", met_name.str, met_desc.str );
		  goto __ERROR;
		}
		  
/* ------------------------------------------------------------------------ */
		  
		case OP_new: { 

		  uint16 cls_idx = get_uint16( &pc[1] );

		  stack[0].obj = alloc_object( JIMP_get_class_by_index( jc, cls_idx ), 1 );
		  stack++;
		  pc += 3;
		  break;  	
		}

		case OP_newarray: { 

		  stack[-1].obj = create_array( (int32)pc[1], stack[-1].int_value );
		  pc += 2;
		  break;
		}

		case OP_anewarray: {

		  stack[-1].obj = create_array( 1, stack[-1].int_value );
		  pc += 3;
		  break;
		}

		case OP_arraylength: { 

		  JIMP_obj obj;
		  obj = stack[-1].obj;
DBG(100,printf("OBJ=%d\n",obj));
		  if( obj == 0 ) goto __NULL_ARRAY_ERROR;
		  stack[-1].int_value = JIMP_ARRAY_LEN( obj );
		  pc++;
		  break;
		}

		case OP_instanceof:
		case OP_checkcast: { 

		  JIMP_obj   obj;
		  JIMP_class *src, *trg;
		  JIMP_utf_string nm;
		  int        result;

		  obj = stack[-1].obj;
		  if( obj == 0 ) {
		    if( *pc == OP_instanceof ) stack[-1].int_value = 0;
		    pc += 3;
		    break;
		  }
		  src = JIMP_OBJ_CLASS( obj );
          trg = JIMP_get_class_by_index( src, get_uint16( pc + 1 ) );

          if( trg ) { /* Not array */
           result = compatible( src, trg );
          }
           else {
             nm = get_class_name( jc, jc->this_class_idx );
             if( (nm.len > 1) && (nm.str[0] == '[' ) ) result = compatible_array( obj, nm );
              else {
                printf("i> TARGET CLASS NOT FOUND ");
                goto __ERROR;
              }
           }

		if( *pc == OP_checkcast ) {
 	     if( !result ) {
		   printf("i> CLASS CAST EXCEPTION "); 
	       goto __ERROR;
         }
        }
		 else stack[-1].int_value = result;

          pc += 3;
		  break;
		}

		case OP_wide: {

		  pc++;
		  switch( *pc ){

		    case OP_iload:
		    case OP_fload:
		    case OP_aload:
			  stack[0] = var[ get_uint16( &pc[1] ) ];
			  stack++;
			  pc += 3;
		      break;

		    case OP_astore:
			case OP_istore:
			case OP_fstore:
			  var[ get_uint16( &pc[1] ) ] = stack[-1];
			  stack--;
			  pc += 3;
			  break;

			case OP_iinc:
			  var[ get_uint16( &pc[1] ) ].int_value += get_int16( &pc[3] );
			  pc += 5;
			  break;

			case OP_ret:
			  pc = var[ get_uint16( &pc[1] ) ].pc;
			  break;
		  }
		  break;
		}

		case OP_multianewarray: {

		  uint16 cls_idx;
		  JIMP_utf_string cls_name;
		  int32  ndim;
		  char   *p;
		  char   *cstr;

		/* Получаем описание массива */
		  p = jc->classmem + jc->consts[get_uint16( &pc[1] )] + 1;
		  cls_idx = get_uint16(p);
		  cls_name = get_UTF_string( jc, cls_idx );

		  ndim = (int32)pc[3];
		  cstr = &cls_name.str[1]; /* ??? а может [0] ???  */
		  stack -= ndim;
		  stack[0].obj = create_multi_array( ndim, cstr, stack );
		  stack++;
		  pc += 4;
		  break;
		}

		case OP_ifnull: { 

      	  if( stack[-1].obj == 0 ) pc += get_int16( &pc[1] );
            else pc += 3;
          stack--;
          break;
		}

		case OP_ifnonnull: { 

      	  if( stack[-1].obj != 0 ) pc += get_int16( &pc[1] );
            else pc += 3;
          stack--;
          break;
		}

        case OP_goto_w: { 

		  pc += get_int32( &pc[1] );
		  break;
        }

        case OP_jsr_w: { 

		  stack[0].pc = pc + 5;
	 	  pc += get_int32( &pc[1] );
		  stack++;
		  break;
        }

 /**  Thread implementation */
        case OP_monitorenter: {
		  JIMP_obj obj = stack[-1].obj;
		  if( !obj ) goto __NULL_OBJ_ERROR;

		  __thread_lock( obj, jts );

		  stack--;
		  pc++;
		  break;
		}
	
        case OP_monitorexit: {
		  JIMP_obj obj = stack[-1].obj;
		  if( !obj ) goto __NULL_OBJ_ERROR;
			
		  __thread_unlock( obj, jts );

		  stack--;
		  pc++;	 
		 break;
		}
  

  /* Errors opcodes :) */ 
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
		case OP_dreturn:
		  printf("i> STILL CANNT EXEC LONG OR DOUBLE OPERATIONS\n");
		  goto __ERROR;
		case OP_athrow:
		default:
		  printf("i> BAD OPCODE");
		  goto __ERROR;

 	} /* switch( *pc ) */

DBG(2,printf("<+>\n"));

	goto STEP;

__NULL_OBJ_ERROR:
  printf("i> NULL OBJECT");
  goto __ERROR;

__INDEX_RANGE_ERROR: 	         
  printf("i>  INDEX RANGE");
  goto __ERROR;

__NULL_ARRAY_ERROR:
  printf("i>  NULL POINTER ARRAY");
  goto __ERROR;

__DIV_BY_ZERO_ERROR:
  printf("i>  DIVISION BY ZERO");
  goto __ERROR;

__STACK_OVERFLOW_ERROR:
  printf("i>  STACK OVERFLOW");
  goto __ERROR;

__ERROR:
  printf(" ERROR \n");
  VM_stack_ptr = base_frame_ptr;
  return;

__METHODRETURN:
DBG(100, printf("METHODRETURN\n"));

  /* pop frame and restore state */
	if( (method->access_flags & ACCESS_NATIVE) > 0) {
      VM_stack_ptr -= 2;
      VM_stack_ptr -= VM_stack[--VM_stack_ptr].int_value;
    }
     else VM_stack_ptr -= method->max_locals + method->max_stack + 2;

   /* fully completed execution */
    if( VM_stack_ptr == base_frame_ptr + 3) {
	  VM_stack_ptr = base_frame_ptr;
      return;
    }

	stack = (JIMP_var *)VM_stack[--VM_stack_ptr].ref_value;
	if( push_ret_val ) {
      stack[0] = ret_val;
      stack++;
	}
	var = (JIMP_var *)VM_stack[--VM_stack_ptr].ref_value;
	pc = VM_stack[--VM_stack_ptr].pc;
	jc = (JIMP_class *)VM_stack[VM_stack_ptr - 1].ref_value;
	method = (JIMP_class_methods *)VM_stack[VM_stack_ptr - 2].ref_value;
	goto STEP;
  
	return;
}

int load_predefined()
{
  /* Try to get Object class */
    if( !(__OBJECTCLASS = JIMP_get_class( __OBJECTCLASS, createUTF( "java/lang/Object" ) )) ) {
      printf("ERROR> JIMP_get_class Object failed\n");
      return 0;
    }
     
  /* Try to get String class */
    if( !(__STRINGCLASS = JIMP_get_class( __STRINGCLASS, createUTF( "java/lang/String" ) )) ){ 
      printf("ERROR> JIMP_get_class String failed\n"); 
      return 0; 
    }  

    return 1;
}

/* - - - - - - - - - -
 * Temporary main :)
 */
void main( int argc, char *argv[] )
{
    JIMP_class  *jclass;
    JIMP_class  *jt;
    JIMP_obj    __argv, *__args;
    JIMP_class_methods *method;
    JIMP_var    param[10], p;
    int         i;
	JIMP_thread_s *jts;

  /* Init VM */
	if( !JIMP_vm_init( 1024, 1024, 1024 ) ) return;

  /* Try to loading predefined classes */	
    if( !load_predefined() ) return;
 
  /* Try to get class */
    if( !(jclass=JIMP_get_class( jclass, createUTF( argv[1] ) )) ) { 
      printf("ERROR> JIMP_get_class failed\n"); 
      return; 
    }

 /* push it's class object on the stack */
	ex_push_obj( jclass->obj ); 

  /* Main thread :) */
	jts = (JIMP_thread_s *) malloc( sizeof( JIMP_thread_s ) );
	if( jts == NULL ) { 
	  printf( "main> can't allocate memory for the main thread\n");
	  return; 
	}
	jts->VM_stack = VM_stack;
	jts->VM_stack_ptr = VM_stack_ptr;
	jts->VM_stack_size = VM_stack_size;
	jts->obj = jclass->obj;
	
	__init_thread_mutexes( jts );
	heap[jclass->obj].vm_t = jts;

	param[0].obj = jclass->obj;
	method = JIMP_get_method( jclass, createUTF("<init>"), createUTF("()V"), NULL );
    if( method != NULL ){
	 JIMP_exec_method( jclass, method, param, 1, jts );
    }

   /* - Main method - */
    __argv = create_array( 2, argc );
    __args = JIMP_ARRAY_START( __argv );
    for( i=1; i<argc; i++ ) {
      __args[i-1] = create_string( argv[i] ); 
    }
    __args[ argc-1 ] = 0;
    p.obj = __argv;
	method = JIMP_get_method( jclass, createUTF("main"), createUTF("([Ljava/lang/String;)V"), NULL );
    if( method != NULL ){
	
	 JIMP_exec_method( jclass, method, &p, argc-1, jts );
    } else printf( "jimp> method main(String[]) was not found\n" );

	JIMP_vm_free();
    free( jclass );
	free( jts );
}
