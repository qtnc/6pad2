#ifndef _____ANY_H_____1
#define _____ANY_H_____1
#include<boost/optional.hpp>
#include<boost/any.hpp>
#include<typeinfo>

using boost::any;
using boost::any_cast;
using boost::optional;
using boost::none;

#define isoftype(a,...) a.type()==typeid(__VA_ARGS__)

#endif