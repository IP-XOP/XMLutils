#ifndef HAVE_MEMUTILS
#include "memutils.h"
#endif

/*
 these functions convert between UTF8 and default system encoding.
 conversion is done in-place.
 if mem is NULL to start with then NULL is returned.
 otherwise the conversion is done.
 NOTE: all output is NULL terminated.  Thus if mem.getMemSize() == 0 is used as input, then
 the output will be "\0".
 */
int UTF8toSystemEncoding(MemoryStruct *mem);
int SystemEncodingToUTF8(MemoryStruct *mem);