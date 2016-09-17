#ifndef _____ANY_H_____1
#define _____ANY_H_____1
#include<boost/any.hpp>
#include<typeinfo>

using boost::any;
using boost::any_cast;

#define isoftype(a,t) a.type()==typeid(t)

#endif