#include "Resource.h"
#include<cstring>

Resource::Resource (const TCHAR* name, int tp) : hf(0), hr(0) {
hf = FindResource(NULL, name, MAKEINTRESOURCE(tp));
if (hf) hr = LoadResource(NULL, hf);
}

Resource::~Resource () {
if (hr) FreeResource(hr);
}

size_t Resource::size () {
return SizeofResource(NULL, hf);
}

const void* Resource::data () {
return LockResource(hr);
}

void* Resource::copy () {
size_t sz = size();
char* ch = new char[sz+1];
memcpy(ch, data(), sz);
ch[sz]=0;
return ch;
}
