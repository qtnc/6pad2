#ifndef _____ANY_H_____1
#define _____ANY_H_____1
#include<boost/optional.hpp>
//#include<boost/any.hpp>
#include<experimental/any>

//using boost::any;
//using boost::any_cast;
using std::experimental::any;
using std::experimental::any_cast;
using boost::optional;
using boost::none;

inline void dbgPrintAny (const any& a, const char* file, int line) {
printf("%s:%d: ", file, line);
#define T(t) \
try { \
t z = any_cast<t>(a); \
printf("%s(%s)\n", #t, toString(z) .c_str() ); \
return; \
} catch(...){} \

T(int) T(bool) T(std::string) T(std::wstring)
#undef T
try { 
pair<int,int> z = any_cast<pair<int,int>>(a); 
printf("%s(%d,%d)\n", "pair<int,int>", z.first, z.second);
return; 
} catch(...){} 
printf("unknown type\n");
}

#define printany(a) dbgPrintAny(a, __FILE__, __LINE__);

#endif