#ifndef __JIMPCLASS_H
#define __JIMPCLASS_H

#include "jimptypes.h"

#define MAX_CLASS_HASH	255     /* ??? */

typedef	uint32_t	const_offset;
typedef	uint32_t	JIMP_obj;

typedef union
{
	int32    int_value;
	float    float_value;
	int32    var_offs;
	void     *class_ref;
	uchar    *pc;
	void     *ref_value;
	JIMP_obj obj;
} JIMP_var;

typedef JIMP_var (*__F_NATIVE)(JIMP_var v[]);

typedef struct __JIMP_utf_string_struct
{
	char  	*str;
	uint32	len;
} JIMP_utf_string;

typedef struct __JIMP_methods_attr
{
    uint16	attribute_name_idx;
    uint32  length;
} JIMP_methods_attr;

/* Methods structure */
typedef struct __JIMP_class_methods
{
    uint16	access_flags;
    uint16	name_idx;
    uint16	descr_idx;

    uint16	params_count;
    uchar	init_method;
    uchar	is_ret_value;

    uint16	attributes_count;
    struct __JIMP_methods_attr *methods_attr;

    uint16  max_stack;
    uint16  max_locals;

    // Need to add Code & Exceptions table
    uint32	code_length;
    uchar	*code;
    uint16  exc_table_length;
    uchar	*exc_table;

    /* если метод native */
    __F_NATIVE fnative;

    /* Code attributes и line_number_table - пока пропускаем - это для дебуггеров, 
        вывода на консоль в какой строке произошло exception и т.д.
     */

} JIMP_class_methods;

/* Fields structure */
typedef struct __JIMP_class_fields
{
    uint16	access_flags;
    uint16	name_idx;
    uint16	descr_idx;
	
    uint8	is_final;

	JIMP_var	var;

/* attributes
 Поле может иметь любое число необязательных атрибутов, связанных с ним.
 В настоящее время, единственно предусмотренный атрибут поля - "ConstantValues".
 Это атрибут, указывающий, что это поле - числовая константа и задает числовое
 значение этого поля.
*/

} JIMP_class_fields;

/*** *         **
   *    * * *  * *  JIMP class file structure 
 * * *  ** * * **   Only constant pool 
  *  ** *  * * */

typedef struct __JIMP_class_struct
{ 
   /* array of this classes superclasses */
	struct __JIMP_class_struct *super_class;
	uint16 number_super_classes;

    JIMP_obj obj;

	uint16 this_class_idx;
	uint16 super_class_idx;

	uchar  *classmem;       // ссылка на class файл в памяти
	
    const_offset *consts;   // ссылка на константный пул в памяти
	uint16	num_consts;

	uint16	*access_flags; 	// pointer access_flags area

	uint16	interfaces_count;
	uint16	*interfaces;

	uint16	num_vars;		// total variables number
	uint16	fields_count;
    struct  __JIMP_class_fields *fields;


	uint16	methods_count;
    struct  __JIMP_class_methods *methods;

	struct	__JIMP_class_struct *next_class;

} JIMP_class;

JIMP_class *Class_Hash[ MAX_CLASS_HASH ];

/* Utilities */
float  get_float32( uchar *b );  // Ввести типы float32 float64
double get_float64( uchar *b );
uint32 get_uint32( uchar *b );
uint16 get_uint16( uchar *b );
int32  get_int32( uchar *b );
int16  get_int16( uchar *b );
uchar  get_uint8( uchar *b );

void hex_dump( uchar *b, uint16 n );
void view_const( JIMP_class *jc, uint16 idx );

JIMP_class_fields *JIMP_get_field( JIMP_class *jc, JIMP_utf_string name, JIMP_utf_string desc );
JIMP_class_fields *JIMP_get_field_by_idx( JIMP_class *jc, uint16 idx );

uchar *JIMP_native_load_class( JIMP_utf_string class_name, uint32 *csize );

JIMP_utf_string createUTF( char *name );
uint32 gen_hash_code( JIMP_utf_string name );
int32 count_params( JIMP_utf_string desc );

JIMP_var get_constant_value( JIMP_class *jc, uint16 idx );
JIMP_utf_string get_UTF_string( JIMP_class *jc, uint16 idx );
JIMP_class *JIMP_get_class_by_index( JIMP_class *jc, uint16 idx);
JIMP_class_methods *JIMP_get_method( JIMP_class *jc, JIMP_utf_string name, JIMP_utf_string desc, JIMP_class *vc );
JIMP_utf_string get_class_name( JIMP_class *jc, uint16 idx );


uint16   array_type( char c );
int32    array_type_size( uint32 type );
uint32   array_size( uint32 type, uint32 len );
JIMP_obj create_array( uint32 type, uint32 len );
JIMP_obj create_multi_array( uint32 ndim, char *desc, JIMP_var *sizes );
JIMP_obj create_UTF_from_string( JIMP_utf_string s );
JIMP_obj create_string( char *buf );

int		 compatible( JIMP_class *source, JIMP_class *target );
int		 compatible_array( JIMP_obj obj, JIMP_utf_string array_name );
JIMP_var copy_array( JIMP_var *stack );

/* JIMP macroses */

#define GET_TAG(x) (get_uint8(x))
#define GET_UTF_LEN(x) (get_uint16(x))

#define JIMP_OBJ_CLASS( x )    heap[x].__ptr[0].class_ref
#define JIMP_ARRAY_TYPE( x )   heap[x].__ptr[1].int_value
#define JIMP_ARRAY_LEN( x )    heap[x].__ptr[2].int_value
#define JIMP_ARRAY_START( x )  &(heap[x].__ptr[3])
#define JIMP_STRING_ARRAY_OBJ(x)  heap[x].__ptr[1].obj

#include "dbglevel.h"

#endif
