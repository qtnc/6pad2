#ifndef ___INIFILE_H9
#define ___INIFILE_H9
#include<string>
#include<map>
#include<unordered_map>
#include "strings.hpp"
using std::map;
using std::multimap;

template<class T> inline int eltm (const T& str, int def, const std::unordered_map<T,int>& map) {
auto it = map.find(str);
return it==map.end()? def : it->second;
}

struct export IniFile: private std::multimap<string,string>  {
std::vector<std::pair<string, shared_ptr<IniFile>>> sections;
std::unordered_map<string, shared_ptr<IniFile>> sectionsByName;

typedef std::multimap<string,string> super;
using super::find;
using super::erase;
using super::count;
using super::begin;
using super::end;
using super::size;
using super::clear;
using super::equal_range;

bool load (const tstring& fn) ;
bool save (const tstring& fn) ;
void fusion (IniFile& ini, bool overwrite=true);
inline bool contains (const std::string& key)  { return find(key)!=end(); }
inline bool contains (const std::string& section, const std::string& key) {
auto it = sectionsByName.find(section);
if (it==sectionsByName.end()) return false;
return it->second->contains(key);
}
inline const std::string& get3 (const string& key, const string& def = "")  { 
auto it = find(key);
if (it==end()) return def;
else return it->second;
}
inline const std::string& get4 (const string& section, const string& key, const string& def = "")  {
auto it = sectionsByName.find(section);
if (it==sectionsByName.end()) return def;
return it->second->get3(key, def);
}
inline void set3 (const std::string& key, const std::string& value, bool allowMulti=false) { 
if (!allowMulti) erase(key);
insert(pair<std::string,std::string>(key,value)); 
}
inline void set3 (const std::string& section, const std::string& key, const std::string& value, bool allowMulti=false) { 
auto sect = sectionsByName[section];
if (!sect) {
sect = shared_ptr<IniFile>( new IniFile() );
sectionsByName[section] = sect;
sections.push_back(std::pair<string,shared_ptr<IniFile>>(section,sect));
}
sect->set3(key, value, allowMulti);
}
template<class T> inline T get (const std::string& key, T def) { return fromString<T>(get3(key, toString(def))); }
template<class T> inline T get (const std::string& section, const std::string& key, T def) { return fromString<T>(get4(section, key, toString(def))); }
template<class T> inline T get (const char*  key, T def) { return get<T>(string(key), def); }
template<class T> inline T get (const char* section, const char*  key, T def) { return get<T>(string(section), string(key), def); }
template<class T> inline T get (const string& section, const char*  key, T def) { return get<T>(section, string(key), def); }
template<class T> inline void set (const std::string& key, const T& value, bool allowMulti=false) { set3(key, toString(value), allowMulti); }
template<class T> inline void set2 (const std::string& key, const T& value, bool allowMulti=false) { set3(key, toString(value), allowMulti); }
template<class T> inline void set (const std::string& section, const std::string& key, const T& value, bool allowMulti=false) { set3(section, key, toString(value), allowMulti); }
template<class T> inline void set (const char* key, const T& value, bool allowMulti=false) { set3(string(key), toString(value), allowMulti); }
template<class T> inline void set (const std::string& section, const char* key, const T& value, bool allowMulti=false) { set3(section, string(key), toString(value), allowMulti); }
template<class T> inline void set (const char* section, const char* key, const T& value, bool allowMulti=false) { set3(string(section), string(key), toString(value), allowMulti); }
};

#endif