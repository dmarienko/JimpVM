/*  _           _   
 *   | . |   | |_|  * JIMP Virtual Machine ( JVKernel )* 
 *  \  | | | | |    * Copyright (c) 2002 M.D.E.
 *                  * - Arrays functions 
*/

#include "vm.h"
#include "vmmemory.h"

uint16 array_type( char c )
{
	switch( c ) {
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

int32 array_type_size( uint32 type )
{
	switch( type ) {
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
    	case 7:  /* double (invalid) */
    	case 11: /* long (invalid)  */
      		return 8;
    }
	return 0;
}

uint32 array_size( uint32 type, uint32 len )
{
	int32 typesize, size;

	typesize = array_type_size( type );
	size = ( 3 * sizeof( JIMP_var ) ) + ( typesize * len );

  /* align to 4 byte boundary */
	size = (size + 3) & ~3;
	return size;
}

JIMP_obj create_array( uint32 type, uint32 len )
{
	JIMP_obj   obj;
	int y;

    obj = alloc_object( NULL, array_size( type, len ) );
    if( !obj )  return 0;

    JIMP_ARRAY_TYPE( obj )  = type;
    JIMP_ARRAY_LEN( obj )   = len;

	return obj;
}


JIMP_obj create_multi_array( uint32 ndim, char *desc, JIMP_var *sizes )
{
	JIMP_obj array_obj, sub_array, *item_start;
	int32    i, len, type;

	len = sizes[0].int_value;
	type = array_type( desc[0] );
	array_obj = create_array( type, len );

	if ( !array_obj ) return 0;
	if ( (type != 2) || (ndim <= 1) ) return array_obj;

  /* prevent from gc() */	
	ex_push_obj( array_obj );

   /* create subarray */
	for (i = 0; i < len; i++) {
	 sub_array = create_multi_array( ndim - 1, desc + 1, sizes + 1 );
	 item_start = (JIMP_obj*) JIMP_ARRAY_START( array_obj );
	 item_start[i] = sub_array;
	}

	ex_pop_obj();
	return array_obj;
}

int compatible_array( JIMP_obj obj, JIMP_utf_string array_name )
{
  JIMP_class *jc;

  jc = JIMP_OBJ_CLASS( obj );
  if( jc != NULL ) return 0; // source is not array
  if( JIMP_ARRAY_TYPE( obj ) != array_type( array_name.str[1] ) ) return 0;
  return 1;
}

JIMP_obj create_UTF_from_string( JIMP_utf_string s )
{
	JIMP_obj obj, char_array_obj;
	uint16   *char_start;
	uint32   i;

   /* create and fill char array */
	char_array_obj = create_array( 5, s.len );
	if( !char_array_obj ) return 0;

	ex_push_obj( char_array_obj );

	char_start = (uint16 *)JIMP_ARRAY_START( char_array_obj );
	for (i = 0; i < s.len; i++) char_start[i] = (uint16) s.str[i];

   /* create String object and set char array */
	obj = alloc_object( __STRINGCLASS, 1 );

	if( obj != 0 ) { 
	  JIMP_STRING_ARRAY_OBJ( obj ) = char_array_obj;
	 /* experimental */ 
	  JIMP_ARRAY_LEN(char_array_obj) = s.len;
	}	

	ex_pop_obj();
	return obj;
}

JIMP_obj create_string( char *buf ) {
  return create_UTF_from_string( createUTF( buf ) );
}

JIMP_var copy_array( JIMP_var *s ) {

	JIMP_obj	src, dst;
	JIMP_var	r;
	uint32		bsrc, bdst, len, t, tsz;
	uchar		*from, *to;
int i;

    r.int_value = 0;
	src = s[0].obj;
	bsrc = s[1].int_value;
	dst = s[2].obj;
	bdst = s[3].int_value;
	len = s[4].int_value;

	if( (!src) || (!dst) ) {
	  printf("copy_array> NULL ARRAY\n");
	  return r; 
	}
	
	if( len == 0 ) return r;
	if( (JIMP_OBJ_CLASS(src)!=NULL) || (JIMP_OBJ_CLASS(dst)!=NULL))	return r;
	t = JIMP_ARRAY_TYPE( src );
	if( t != JIMP_ARRAY_TYPE( dst ) ) return r;

	if( bsrc+len > JIMP_ARRAY_LEN(src) ) return r;
	if( bdst+len > JIMP_ARRAY_LEN(dst) ) return r;
	tsz = array_type_size( t );
	from = (uchar*) JIMP_ARRAY_START( src ) + ( tsz * bsrc );
	to =   (uchar*) JIMP_ARRAY_START( dst ) + ( tsz * bdst );

	memcpy( (uchar*)to, (uchar*)from, len * tsz );
	r.int_value = 1;
	return r;
}


