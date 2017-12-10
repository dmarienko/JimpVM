#ifndef __NATIVE_H
#define __NATIVE_H

#include "jimptypes.h"
#include "jimpclass.h"

typedef struct _native_funct {
	int hash;
    __F_NATIVE f;
} NATIVE_FUNCT;

#endif
