
#include <string>
using namespace std;
/*
 these functions convert between UTF8 and default system encoding.
 conversion is done in-place.
 if mem is NULL to start with then NULL is returned.
 otherwise the conversion is done.
 NOTE: all output is NULL terminated.  Thus if mem.getMemSize() == 0 is used as input, then
 the output will be "\0".
 
 return values:
 0 = SUCCESS
 1 = NO MEMORY
 */
int UTF8toSystemEncoding(string mem);
int SystemEncodingToUTF8(string mem);