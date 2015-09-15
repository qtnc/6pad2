#ifndef _____SIGNALS_H9
#define _____SIGNALS_H9
#include "variant.h"
#include<functional>
#include<boost/signals2.hpp>
using boost::signals2::signal;
using boost::signals2::connection;

struct BoolSignalCombiner {
typedef bool result_type;
template<class I> bool operator() (I start, I end) {
bool re = true;
while(start!=end && (re=*start)) ++start;
return re;
}};

struct VarSignalCombiner {
typedef var result_type;
template<class I> var operator() (I start, I end) {
var re;
while(start!=end) {
re = *start++;
if (re.getType()==T_BOOL && !re) break;
}
return re;
}};

int export AddSignalConnection (const connection& con);
connection export RemoveSignalConnection (int id);

#endif
