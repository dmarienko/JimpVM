/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Miscellaneous help functions
*/

#include "jimptypes.h"
#include "jimpclass.h"
#include "vmmemory.h"

void hex_dump( uchar *b, uint16 n )
{
    int i;
	for(i=0;i<n;i++) printf( "0x%02X ", b[i] );
}

uint32 get_uint32( uchar *b ) {
	return (uint32)( (uint32)((b)[0])<<24 |
			(uint32)((b)[1])<<16 | (uint32)((b)[2])<<8 | (uint32)((b)[3]) );
}

uint16 get_uint16( uchar *b ) {
	return (uint16)(((b)[0]<<8)|(b)[1]);
}

int32 get_int32( uchar *b ) {
	return (int32)( (uint32)((b)[0])<<24 |
			(uint32)((b)[1])<<16 | (uint32)((b)[2])<<8 | (uint32)((b)[3]) );
}

int16 get_int16( uchar *b ) {
	return (int16)(((b)[0]<<8)|(b)[1]);
}

uchar get_uint8( uchar *b ) {
	return (uchar)(b)[0];
}

JIMP_utf_string createUTF( char *name ) {

	JIMP_utf_string rs;

	rs.str = name;
    rs.len = strlen( name );
   	return rs;
}

float get_float32( uchar *buf ) {

    uint32 i;
    float  f;

    // we need to make sure we're aligned before casting
    i = ((uint32)buf[0] << 24) |
	((uint32)buf[1] << 16) |
	((uint32)buf[2] << 8 ) |
	(uint32)buf[3];
    f = *((float *)&i);

    return f;
}

double get_float64( uchar *buf ) {

    uint64 i;
    double f;

    // we need to make sure we're aligned before casting
    i = ((uint64)buf[0] << 56 ) |
		((uint64)buf[1] << 48 ) |
		((uint64)buf[2] << 40 ) |
		((uint64)buf[3] << 32 ) |
    	((uint64)buf[4] << 24 ) |
    	((uint64)buf[5] << 16 ) |
    	((uint64)buf[6] << 8  ) |
    	((uint64)buf[7] << 0  );
    f = *((double *)&i);

    return f;
}

uint32 gen_hash_code( JIMP_utf_string name ) {

  uint32 value, i;

  value = 0;
  for (i = 0; i < name.len; i++) value += name.str[i];
  value = (value << 6) + name.len;
  return value;
}

int32 count_params( JIMP_utf_string desc ) {

  uint32 n;
  char *c;

  c = desc.str;
  if (*c++ != '(') return -1;
  n = 0;
  while (1){
    switch (*c) {
	  case 'B':
	  case 'C':
	  case 'F':
	  case 'I':
	  case 'S':
	  case 'Z':
	    n++;
	    c++;
	    break;

	  case 'D': /* longs/doubles not supported */
	  case 'J':
	    return -1;

	  case 'L':
	    c++;
	    while (*c++ != ';');
	    n++;
	    break;

	  case '[':
	    c++;
	    break;

	  case ')':
	    return n;

	  default:
	    return -1;
	 }
  }
}

JIMP_var get_constant_value( JIMP_class *jc, uint16 idx ) {

	JIMP_var v;
  	uchar *p = jc->consts[idx] + jc->classmem;
    int string_idx, string_len;

	switch ( GET_TAG(p) ) {
     case CONSTANT_Integer:
      v.int_value = get_uint16( p+1 );
      break;

     case CONSTANT_Float:
      v.float_value = get_float32( p+1 );
      break;

     case CONSTANT_String:
      string_idx = get_uint16( p+1 );
      v.obj = create_UTF_from_string( get_UTF_string( jc, string_idx ) );

/* Example accessing to the string 
JIMP_obj k;
char   *c;
 k = JIMP_STRING_ARRAY_OBJ( v.obj );
 c = (char*)JIMP_ARRAY_START(k);
 printf("~~~~~~~~~~~~~");
 for(string_idx=0;string_idx<20;string_idx+=2) printf("%c", *(c+string_idx) );
 */
      break;

     case CONSTANT_Long:
     case CONSTANT_Double:
     default:
      v.obj = 0; /* bad constant */
      break;
    }

	return v;
}

JIMP_utf_string get_UTF_string( JIMP_class *jc, uint16 idx ) {

	JIMP_utf_string utf;

    utf.str = (char*) (jc->classmem + jc->consts[ idx ] + 1);
    utf.len = get_uint16( utf.str );
    utf.str += 2;

    return utf;
}

JIMP_utf_string get_class_name( JIMP_class *jc, uint16 idx ) {

	uchar *p = (char*) (jc->classmem + jc->consts[ idx ] + 1);
    return get_UTF_string( jc, get_uint16( p ) );
}

uint32 __filelength( int cf ) {

	uint32 l, old;
	if( cf < 0 ) return 0;

	old = tell( cf );
	lseek( cf, 0, SEEK_END );
	l = tell( cf );
	lseek( cf, old, SEEK_SET );

	return l;
}

/* - - - - - - - - - - -
 * Native class loader - for UNIX/Win32 only
 * - - - - - - - - - - - */
uchar *JIMP_native_load_class( JIMP_utf_string class_name, uint32 *csize ) {

	int res;
	uchar *p;
	char  clp[512];

  /* - - - - Test only - - - - */	
	int cf;

	if(!class_name.len) {
     printf( "error class name \n" );
     return NULL;
	}

  /* construct full class name */
  	memset( clp, 0, sizeof( clp ) );
	strncpy( clp, class_name.str, class_name.len );
	strcat( clp, ".class" );

    cf = open( clp, O_RDONLY|O_BINARY );
    if(cf<0){
     printf( "error open %s\n", clp );
     return NULL;
    }

    *csize = __filelength( cf );
    p = (uchar *) calloc( 1, *csize );

    if(p==NULL) {
     printf( "error allocating memory\n" ); 
     close( cf );
     return NULL;
    }

	res = read( cf, p, *csize );
	if( res != *csize ) {
	 printf( "error reading %s file", clp );
	 close( cf );
	 return NULL;
	}

	return p;
}

/* - - - - - - - - - - - - - - -
 * Get pointer to class method 
 * - - - - - - - - - - - - - - - */
JIMP_class_methods *JIMP_get_method( JIMP_class *jc, JIMP_utf_string name, JIMP_utf_string desc, JIMP_class *vc ){

	JIMP_class_methods *method = NULL;
	JIMP_utf_string mname, mdesc;
	uint32 i, n;

    n = jc->number_super_classes;

	while( 1 ){
	 for ( i = 0; i < jc->methods_count; i++ ) {
	  method = &jc->methods[i];
	  mname = get_UTF_string( jc, method->name_idx );
	  mdesc = get_UTF_string( jc, method->descr_idx );
	  if (name.len == mname.len && desc.len == mdesc.len &&
	      !strncmp(name.str, mname.str, name.len) &&
	      !strncmp(desc.str, mdesc.str, desc.len)) {
       if( vc )  vc = jc;
	   return method;
	  }
 	 }

	/* not a virtual lookup or no superclass */
     if( !vc )  break; 
     if (n == 0) break;
     n--;

    /* lookup in superclass */
     if( jc->super_class ) jc = jc->super_class;
    }

	return 0;
}

/*
  Процедура сравнения - слизана с waba vm
*/

int compatible( JIMP_class *source, JIMP_class *target ) {

  int target_interface;
  uint32 i, n;

  if( !source || !target ) return 0; // source or target is array

  target_interface = 0;
  if( (get_uint16(target->access_flags) & ACCESS_INTERFACE) ) target_interface = 1;
  n = source->number_super_classes;

  while (1) {
      if( target_interface ) {
	  for (i = 0; i < get_uint16(source->access_flags[6]); i++) {
	      uint16 cls_idx;
	      JIMP_class *i_class;

	      cls_idx = get_uint16( &source->access_flags[8 + (i * 2)]);
	      i_class = JIMP_get_class_by_index( source, cls_idx );
				// NOTE: Either one of the interfaces in the source class can
				// equal the target interface or one of the interfaces
				// in the target interface (class) can equal one of the
				// interfaces in the source class for the two to be compatible
	      if( i_class == target ) return 1;
	      if( compatible( i_class, target ) ) return 1;
	    }
	}
      else if( source == target ) return 1;
      if( n == 0 ) break;
      // look in superclass
      source = &source->super_class[--n];
    }
	return 0;
}

/* - - - - - - - - - 
 * Отладочная пр-ра
 * - - - - - - - - - */
void view_const( JIMP_class *jc, uint16 idx ) {

 	char  ddd[256];
 	uchar *p = jc->consts[idx] + jc->classmem;
    uint32 _meth_ref, _int_meth_ref, _name_and_type;
    uint16 _class, _string;
    uint64 _l;


    switch( GET_TAG(p) ){
	 case CONSTANT_Utf8:
						p++;
            			p+=2;
	 memset( ddd, 0, sizeof(ddd) );
	 memcpy( ddd, p, GET_UTF_LEN(p-2) );
	 printf("(%d)UTF> %s\n", idx, ddd);
	 					p += GET_UTF_LEN(p-2);
	 					break;

	 case CONSTANT_Integer:
          				   p++;
                           printf("(%d)Integer>(%d)\n",idx,get_int32(p));
                           break;
	 case CONSTANT_Float:
          				   p++;
                           printf("(%d)Float>(%f)\n",idx,get_float32( p ));
                           break;
	 case CONSTANT_Fieldref:
          				   p++;
                           printf("(%d)Fieldref> %d\n",idx, get_int16(p));
                           break;
	 case CONSTANT_Methodref:
          				   p++;
                           printf("(%d)Methodref> %d\n", idx, get_int16(p));
                           break;
	 case CONSTANT_InterfaceMethodref: 
          				   p++;
                           printf("(%d)InterfaceMethodref> %d\n", idx, get_int16(p) );
                           break;
	 case CONSTANT_NameAndType: 
						    p++;
	 					break;

     case CONSTANT_Class: 
          				   p++;
                           printf("(%d)Class> %d\n", idx, get_int16(p));
                           break;
     case CONSTANT_String: 
          				   p++;
                           printf("(%d)String> %d\n", idx, get_int16(p));
                           break;

     case CONSTANT_Long: 
          				   p++;
                           _l = (uint64*)(*p);
                           printf("(%d)String>%ld\n",idx,_l);
                           break;
     case CONSTANT_Double: 
          				   p++;
                           printf("(%d)Double>%f\n",idx, get_float64(p));
                           break;

	}
}

