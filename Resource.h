#ifndef _____RESOURCE_HPP_3
#define _____RESOURCE_HPP_3
#include "global.h"

struct Resource {
HRSRC hf;
HANDLE hr;

Resource (const TCHAR* name, int type) ;
~Resource () ;
Resource (const Resource&) = delete;
Resource& operator= (const Resource&) = delete;
size_t size () ;
const void* data () ;
void* copy () ;
};

#endif

