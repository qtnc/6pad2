#ifndef _____RESOURCE_HPP_3
#define _____RESOURCE_HPP_3
#include "global.h"
#include<boost/shared_array.hpp>

struct Resource {
HRSRC hf;
HANDLE hr;

Resource (const TCHAR* name, int type) ;
~Resource () ;
Resource (const Resource&) = delete;
Resource& operator= (const Resource&) = delete;
Resource (Resource&&) = default;
Resource& operator= (Resource&&) = default;
size_t size () ;
const void* data () ;
boost::shared_array<char> copy () ;
};

#endif

