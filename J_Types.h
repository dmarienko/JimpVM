#ifndef __JTYPES_H
#define __JTYPES_H

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * In real version it's coudn't be - OS specific includes
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
//#include "../include/types.h"
//#include "../include/types.h"

/* #include "../include/macro/debug.h" */
/* #include <macro/debug.h> */
//#include <inttypes.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

typedef uint16_t u2;
typedef uint32_t u4;

/* Access flags */
#define ACCESS_PUBLIC				0x0001
#define ACCESS_PRIVATE				0x0002
#define ACCESS_PROTECTED			0x0004
#define ACCESS_STATIC				0x0008
#define ACCESS_FINAL				0x0010
#define ACCESS_SYNCHRONIZED			0x0020
#define ACCESS_VOLATILE				0x0040
#define ACCESS_TRANSIENT			0x0080
#define ACCESS_NATIVE				0x0100
#define ACCESS_INTERFACE			0x0200
#define ACCESS_ABSTRACT				0x0400

/* Constant Pool tags */
#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref          10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType        12

typedef	uint32_t J_Object;

/**
 *  Warning and debug levels 
 */
#define DBG_LEV_0  (1<<0)
#define DBG_LEV_1  (1<<1)
#define DBG_LEV_2  (1<<2)
#define DBG_LEV_3  (1<<3)
#define DBG_LEV_4  (1<<4)
#define DBG_ALL    (uint16_t)(~0)
#define DBG_NONE   0

#define DBG_MEMORY     DBG_LEV_1
#define DBG_VM         DBG_LEV_2
#define DBG_EXCEPTIONS DBG_LEV_3
#define DBG_TEMPORARY  DBG_LEV_4
#define DBG_DISASM     (1<<5)
#define DBG_GC         (1<<6)

#define ERR_FATAL   1
#define ERR_WARNING 2

/* Return type */
typedef enum { 
  EOK,
  ENOMEM,
  ECLASSNOTFOUND,
  ECONSTSFAIL,
  EINTERFACEFAIL,
  EFIELDSFAIL,
  EMETHODSFAIL,
  EMETHODNOTFOUND,
  ECANTRUNNATIVE,
  EZEROLENGTH,
  ESTACKOVERFLOW,
  EFIELDNOTFOUND,
} J_Error_t;

#endif
