#include "variant.h"

struct GetType: boost::static_visitor<int> {
int operator() (nullptr_t& _) { return T_NULL; }
int operator() (bool& _) { return T_BOOL; }
int operator() (int& _) { return T_INT; }
int operator() (tstring& _) { return T_STR; }
};

struct ToTString: boost::static_visitor<tstring> {
tstring operator() (nullptr_t& x) { return TEXT(""); }
tstring operator() (bool& b) { return b?TEXT("true"):TEXT("false"); }
tstring operator() (int& x) { return toTString(x); }
tstring operator() (tstring& s) { return s; }
};

struct ToInt: boost::static_visitor<int> {
int operator() (nullptr_t& _) { return 0; }
int operator() (bool& x) { return x?1:0; }
int operator() (int& x) { return x; }
int operator() (tstring& x) { return toInt(x); }
};

struct ToBool: boost::static_visitor<bool> {
bool operator() (nullptr_t& _) { return false; }
bool operator() (bool& b) { return b; }
bool operator() (int& x) { return x!=0; }
bool operator() (tstring& x) { return x.size()>0; }
};

int var::getType () {
static GetType gt;
return boost::apply_visitor(gt, *this);
}

int var::toInt () {
static ToInt x;
return boost::apply_visitor(x,*this);
}

bool var::toBool () {
static ToBool x;
return boost::apply_visitor(x,*this);
}

tstring var::toTString () {
static ToTString x;
return boost::apply_visitor(x,*this);
}
