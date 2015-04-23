#ifndef ___INIFILE_H9
#define ___INIFILE_H9
#include<string>
#include<map>
#include "strings.hpp"

struct IniFile: private std::multimap<string,string>  {
typedef std::multimap<string,string> super;
using super::find;
using super::erase;
using super::count;
using super::begin;
using super::end;
using super::size;
bool load (const tstring& fn) ;
bool save (const tstring& fn) ;
inline bool contains (const std::string& key)  { return find(key)!=end(); }
inline const std::string& get3 (const string& key, const string& def = "")  { 
auto it = find(key);
if (it==end()) return def;
else return it->second;
}
inline void set3 (const std::string& key, const std::string& value, bool allowMulti=false) { 
if (!allowMulti) erase(key);
insert(pair<std::string,std::string>(key,value)); 
}
template<class T> inline T get (const std::string& key, T def) { return fromString<T>(get3(key, toString(def))); }
template<class T> inline T get (const char*  key, T def) { return get<T>(string(key), def); }
template<class T> inline void set (const std::string& key, const T& value, bool allowMulti=false) { set3(key, toString(value), allowMulti); }
template<class T> inline void set (const char* key, const T& value, bool allowMulti=false) { set3(string(key), toString(value), allowMulti); }
};

#endif