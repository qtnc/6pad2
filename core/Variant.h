#ifndef _____VARIANT_H8
#define _____VARIANT_H8
#include "global.h"
#include<boost/variant.hpp>

enum {
T_NULL, T_BOOL, T_INT, T_STR
};

struct export var: boost::variant<nullptr_t, bool, int, tstring> {
var(): variant(nullptr) {}
template<class T> var(const T& val): variant(val) {}
int export getType () ;
bool export toBool ();
int export toInt () ;
tstring export toTString () ;
inline operator bool () { return toBool(); }
inline operator int () { return toInt(); }
inline operator tstring () { return toTString(); }
};


#endif