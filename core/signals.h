#ifndef _____SIGNALS_H9
#define _____SIGNALS_H9
#include "any.h"
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

struct AnySignalCombiner {
typedef any result_type;
template<class I> any operator() (I start, I end) {
int count=0;
any re;
while(start!=end) {
re = *start++;
if (!re.empty()) {
bool* b = any_cast<bool>(&re);
if (b&&!*b) break;
}
}
return re;
}};

int export AddSignalConnection (const connection& con);
connection export RemoveSignalConnection (int id);

#endif
