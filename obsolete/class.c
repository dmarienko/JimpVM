/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Parsing class file 
*/

#include "jimptypes.h"
#include "jimpclass.h"
#include "thread.h"
#include "vmmemory.h"

/* - - - - - - - - - - - - -
   load constant from pool
 */
uchar *load_class_const(JIMP_class *jc, uint16 idx, uchar *p, uint16 *tag){
  switch( (*tag=GET_TAG(p)) ) {
  case CONSTANT_Utf8:
	DBG(1,view_const(jc,idx));
	p++;
	p += GET_UTF_LEN(p);
	p+=2;
	break;

  case CONSTANT_Integer:
  case CONSTANT_Float:
  case CONSTANT_Fieldref:
  case CONSTANT_Methodref:
  case CONSTANT_InterfaceMethodref: 
  case CONSTANT_NameAndType: 
	DBG(1,view_const( jc, idx ));
	p++;
	p += 4;
	break;

  case CONSTANT_Class: 
  case CONSTANT_String: 
	DBG(1,view_const( jc, idx ));
	p++;
	p += 2; 
	break;

  case CONSTANT_Long: 
  case CONSTANT_Double: 
	DBG(1,view_const( jc, idx ));
	p++;
	p += 8; 
	break;
	
  }
  return p;
}

JIMP_class_fields *JIMP_get_field_by_idx( JIMP_class *jc, uint16 idx )
{
	JIMP_class_fields *field;
	JIMP_class 		  *target_class;
	JIMP_utf_string   fname, fdesc;
	uint16 			  class_idx, name_type_idx, n_idx, t_idx;

	char *p = jc->consts[idx] + jc->classmem;
    class_idx = get_uint16( p + 1 );
	target_class = JIMP_get_class_by_index( jc, class_idx );
	if( target_class == NULL ) return NULL;
	name_type_idx = get_uint16( p + 3 );

    n_idx = get_uint16( jc->consts[name_type_idx] + jc->classmem + 1 ); 
    t_idx = get_uint16( jc->consts[name_type_idx] + jc->classmem + 3 );

	fname = get_UTF_string( jc, n_idx );
	fdesc = get_UTF_string( jc, t_idx );

	field = JIMP_get_field( target_class, fname, fdesc );

	return field;
}

JIMP_class_fields *JIMP_get_field( JIMP_class *jc, JIMP_utf_string name, JIMP_utf_string desc )
{
	JIMP_class_fields *field;
	JIMP_utf_string    fname, fdesc;
	uint16             i;

	for( i=0; i < jc->fields_count; i++ ) {

	  field = &jc->fields[i];
	  fname = get_UTF_string( jc, field->name_idx );
	  fdesc = get_UTF_string( jc, field->descr_idx );
	  if( name.len == fname.len && desc.len == fdesc.len &&
	      !strncmp( name.str, fname.str, name.len ) &&
	      !strncmp( desc.str, fdesc.str, desc.len ) ) return field;
	}

	printf("JIMP_get_field> cant find field %s\n", name.str );
	return NULL;
}


/* - - - - - - - - - - - - - - - - - - - - -
   Parse constant pool from the class file 
 */
uint32 JIMP_parse_constants( JIMP_class *jc, uchar *p )
{
	uint32 size, i;
    uchar  *__start = p;
    uint16 tag;

  	p += 8;
  	jc->num_consts = get_uint16(p);
DBG(1,printf("num_const=%d\n",jc->num_consts));

	p += 2;
	if( jc->num_consts != 0 ){
     size = sizeof( const_offset ) * jc->num_consts;

   /* allocate memory for constants pool */
     jc->consts = ( const_offset* ) calloc( 1, size );

     if(jc->consts==NULL) {
      printf("getClass> not enought memory for constant pool\n");
      return 0;
     }

	 for( i=1; i<jc->num_consts; i++ ) {

	  if( p - jc->classmem > 0x7fffffff ) {
	   printf("getClass> class too large\n");
	   return 0;
	  }

      jc->consts[i] = p - jc->classmem;
      p = load_class_const( jc, i, p, &tag );

      if( (tag == CONSTANT_Long) || (tag == CONSTANT_Double) ) i++;

     } /* for */
    } /* if( jc->num_consts != 0 ) */

    return ( p - __start );
}

/* - - - - - - - - - - - - - - - - - - - 
   Parse interfaces from the class file 
 */
uint32 JIMP_parse_interfaces( JIMP_class *jc, uchar *p )
{
	uint16 i;
    uchar  *__start = p;

    if( jc->interfaces_count > 0 ) {
     jc->interfaces = (uint16*) calloc( jc->interfaces_count, sizeof( uint16 ) );
     if( jc->interfaces == NULL ) {
      printf("parse_interfaces> not enought memory for interfaces\n");
      return 0;
     }

     for( i=0; i<jc->interfaces_count; i++ ){
      jc->interfaces[i] = get_uint16( p );
      p += 2;
     }

    }

    return ( p - __start );
}

/* - - - - - - - - - - - - - - - - - 
   Parse fields from the class file 
 */
uint32 JIMP_parse_fields( JIMP_class *jc, uchar *p )
{
    uchar  *__start = p;
    uint16 i, j, attr_count, name_idx, access_flags;
    uint32 bytes_count;
    JIMP_utf_string us;

    if( jc->fields_count > 0 ) {
     jc->fields = (uint16*) calloc( jc->fields_count, sizeof( JIMP_class_fields ) );
     if( jc->fields == NULL ) {
      printf("parse_fields> not enought memory for fields\n");
      return 0;
     }

     jc->num_vars = 0;

     for( i=0; i < jc->fields_count; i++ ){
     
    /* get access flags */
      access_flags = get_uint16( p );
      p += 2;
      jc->fields[i].access_flags = access_flags;

      if( !( access_flags & ACCESS_STATIC ) ) {
        jc->fields[i].var.var_offs = jc->num_vars++;   /* local variable */
        access_flags = 1;
      }
       else {  
         access_flags = 0;  /* static var */
         jc->fields[i].var.var_offs = 0;
        }

     /* get name idx and description idx */
      jc->fields[i].name_idx = get_uint16( p );
	  p += 2; 
	  jc->fields[i].descr_idx = get_uint16( p );
	  p += 2; 

	  attr_count = get_uint16( p );
	  p += 2;

	  for( j = 0; j < attr_count; j++ ) {
       name_idx = get_uint16( p );
       us = get_UTF_string( jc, name_idx ); 
       p += 2;

       bytes_count = get_uint32( p );
       p += 4;

      // if final ... (constant)
       if( access_flags && us.len == 13 && bytes_count == 2 &&
	       !strncmp( us.str, "ConstantValue", 13 ) ) {
	     jc->fields[i].var = get_constant_value( jc, get_uint16( p ) );
         jc->fields[i].is_final = 1;
	     }
       else jc->fields[i].is_final = 0;
       p += bytes_count;
      }  // for(j = ...
     } // for(i = ...

    } // if( jc->fields_count > 0 )

    return ( p - __start );
}

/* - - - - - - - - - - - - - - - - - -
   Parse methods from the class file 
 */
uint32 JIMP_parse_methods( JIMP_class *jc, uchar *p )
{
	uint16 _acc;
	uint32 i, j, sz, t;
    uchar  *__start = p;
    JIMP_utf_string str;

    if( jc->methods_count > 0 ) {

  /* allocate memory for methods */
     jc->methods = ( JIMP_class_methods* ) calloc( jc->methods_count, sizeof( JIMP_class_methods ) );
     if(jc->methods==NULL) {
      printf("parse_method> not enought memory for methods\n");
      return 0;
     }

     for( i=0; i<jc->methods_count; i++ ) {
DBG(1,printf("methods[%d]: ", i ));

      _acc = jc->methods[i].access_flags = get_uint16( p );
      p += 2;

      jc->methods[i].name_idx = get_uint16( p );
DBG(1,view_const( jc, jc->methods[i].name_idx ));
      p += 2;

      jc->methods[i].descr_idx = get_uint16( p ); 
DBG(1,view_const( jc, jc->methods[i].descr_idx ));

	 /* Count method parameters */
      jc->methods[i].params_count = count_params( get_UTF_string( jc, jc->methods[i].descr_idx ) );
      if(jc->methods[i].params_count<0) jc->methods[i].params_count = 0;
      p += 2;

	 /* Is it method init ? */
      jc->methods[i].init_method = 0;
      str = get_UTF_string( jc, jc->methods[i].name_idx );
  DBG(3,printf("check method %s \n", str.str));
      if( (str.str[0] == '<') && (str.str[1] == 'i') ) {
  DBG(3,printf("method %s is <init>\n", str.str));
       jc->methods[i].init_method = 1;
      }

	 /* Is it return value ? */
      str = get_UTF_string( jc, jc->methods[i].descr_idx );
	  jc->methods[i].is_ret_value = 0;
      if( str.str[str.len-1] != 'V' ) jc->methods[i].is_ret_value = 1;

     /* how many attribures have its method */
      jc->methods[i].attributes_count = get_uint16( p );
      p += 2;

     /* allocate memory for the attributes */
      jc->methods[i].methods_attr = (JIMP_methods_attr*) calloc( jc->methods[i].attributes_count,
                                                          sizeof( JIMP_methods_attr ) );
      if(jc->methods[i].methods_attr==NULL) {
       printf("parse_method> not enought memory for methods attrib structure\n");
       return 0;
      }

     /* get method attributes */
DBG(1,printf("method attributes count = %d\n", jc->methods[i].attributes_count ));

 /*
  *  If the method is either native or abstract, its method_info 
  *     structure must not have a Code attribute. 
  */

	/* Тут надо еще загружать native функции */
if( !( _acc & ( ACCESS_NATIVE | ACCESS_ABSTRACT ) ) ) 
{

      for( j=0; j < jc->methods[i].attributes_count; j++ ){
        jc->methods[i].methods_attr[j].attribute_name_idx = get_uint16( p );
        p += 2;
DBG(1,view_const( jc, jc->methods[i].methods_attr[j].attribute_name_idx ));

        jc->methods[i].methods_attr[j].length = get_uint32( p );
        p += 4;
//      } // for( j=0...

    /* Get max stack and max locals */
      jc->methods[i].max_stack = get_uint16( p );
      p += 2;

      jc->methods[i].max_locals = get_uint16( p );
      p += 2;

    /* Get code section */
	  jc->methods[i].code_length = get_uint32( p );
      p += 4;
      if(jc->methods[i].code_length > 0) jc->methods[i].code = p;
       else { printf("No code section enabled in " ); view_const( jc, jc->methods[i].name_idx ); }
      p += jc->methods[i].code_length;

    /* Get exception table section */
      jc->methods[i].exc_table_length = get_uint16( p );
      p += 2;
      if(jc->methods[i].exc_table_length > 0) jc->methods[i].exc_table = p;
      p += 8 * jc->methods[i].exc_table_length;

    /* Skip code attributes */
      sz = get_uint16( p ); // attributes_count
      p += 2;
      for( j = 0; j < sz; j++ ){
        p += 6; // attr_name_index(2) + attr_length(4)
        t = get_uint16( p ); // line_numb_table_len
        p += 2;
        p += t*4;  // line_table
      }
      } // for( j=0...
     } /* if( !NATIVE )*/

	/* If method is native */
	 if( _acc & ACCESS_NATIVE ) JIMP_load_native_funct( jc, i );

} /* for( i=0...*/
    } /* if( jc->methods_count ... */

   /* And General Attributes section skip ! */

    return ( p - __start );
}

/* JIMP_class *JIMP_get_class( JIMP_class *jc, JIMP_utf_string name ) */
J_Class *jimp_get_class(J_Class *jc, J_UtfString name){
	JIMP_obj 		   obj;
    JIMP_class         *jtmp;
    JIMP_class_methods *method;
    JIMP_utf_string	   hname;
	uchar   		   *p;
	uint32   		   len, i, hash;

DBG(1,printf( "-> Start JIMP_get_class %s\n", name.str ));

  /* Lookup in the hash  table */
    hash = gen_hash_code( name ) % MAX_CLASS_HASH;
	jtmp = Class_Hash[ hash ];

	while( jtmp != NULL ) {
     hname = get_class_name( jtmp, jtmp->this_class_idx );
     if( hname.len == name.len && !strncmp( name.str, hname.str, name.len )) break;
     jtmp = jtmp->next_class;
    }
    if( jtmp != NULL ) return jtmp;

 /* - try to allocate memory */
    jc = ( JIMP_class* ) malloc( sizeof( JIMP_class ) );
    if( !jc ){
     printf("error allocating memory for JIMP class\n");
     return;
    }

 /* Try allocate object */
    if( !( obj = alloc_object( jc, 1 ) ) ) { 
     printf("error alloc_object(..)\n"); 
     return 0; 
    }

 /* - backward link */
    jc->obj = obj;

 /* - load class */
    p = JIMP_native_load_class( name, &len );
    if( p == NULL ) return 0; 

 /* - zeroing superclass pointer */
    jc->super_class = 0;

 /* - where is allocated into memory */
    jc->classmem = p;

 /* - parse constants */
	if( !( len = JIMP_parse_constants( jc, p ) ) ) return 0; 
	p += len;

 /* - get access_flags section */
 	jc->access_flags = p;
	p += 2;

 /* - get this_class index */
    jc->this_class_idx = get_uint16( p );
    p+=2;

 /* - add to hash table */
	jc->next_class = Class_Hash[ hash ];
	Class_Hash[ hash ] = jc;

 /* - get super_class index */
    jc->number_super_classes = 0;
    jc->super_class_idx = get_uint16( p );
    p+=2;

 /* - get interfaces count and parse */
	jc->interfaces_count = get_uint16( p ); 
    p+=2; 
    len = 0;
    if( jc->interfaces_count > 0 ) len = JIMP_parse_interfaces( jc, p );
 	if( len ) p += len;

 /* - get fields count and parse */
	jc->fields_count = get_uint16( p ); 
    p+=2; 
    len = 0;
    if( jc->fields_count > 0 ) len = JIMP_parse_fields( jc, p );
 	if( len ) p += len;

 /* - get methods count and parse */
	jc->methods_count = get_uint16( p ); 
    p+=2; 
    len = 0;
 	if( !( len = JIMP_parse_methods( jc, p ) ) ) return 0; 
 	p += len;

 /* - try to load superclasses */
	if( jc->super_class_idx > 0 ) 
	 if( !(jc->super_class = JIMP_get_class_by_index( jc, jc->super_class_idx)) ) return 0;
	  else jc->number_super_classes++;
     

 /* - find class init <clinit> method : static{...} */
	method = JIMP_get_method( jc, createUTF("<clinit>"), createUTF("()V"), NULL );

 /* - and try to execute it */
    if( method != NULL ){
	  JIMP_thread_s js;
	  js.VM_stack = VM_stack;
	  js.VM_stack_ptr = VM_stack_ptr;
	  js.VM_stack_size = VM_stack_size; 
	  JIMP_exec_method( jc, method, NULL, 0, &js ); 
	}

	return jc;
}

JIMP_class *JIMP_get_class_by_index( JIMP_class *jc, uint16 idx )
{
	JIMP_class *ret_jc;
	uchar      *p = jc->consts[idx] + jc->classmem + 1;
	uint16     cls_idx;

   /* Get class index */
    cls_idx = get_uint16( p );

   /* Try to load it class and return */
    ret_jc = JIMP_get_class( ret_jc, get_UTF_string( jc, cls_idx ) );

    return ret_jc;
}
