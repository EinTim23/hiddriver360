#ifndef __XKELIB_H
#define __XKELIB_H


#pragma warning(push)
// 4214 nonstandard extension used : bit field types other than int
// 4201 nonstandard extension used : nameless struct/union
// 2220 warning treated as error - no 'object' file generated
// note #pragma warning(disable:4005) is used in xamXContent.h XCONTENTTYPE_ defs
#pragma warning(disable:4200 4201 4214)

#include "exttypes.h"
#include "kernel\_kernelext.h"
#include "kernel\console_type.h" // this must come after kernel prototypes
#include "xam\_xamext.h"
#include "xapiext.h"

#pragma warning(pop)


#endif // __XKELIB_H
