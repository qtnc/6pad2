#include "IniFile.h"
#include "file.h"

bool IniFile::save (const tstring& fn) {
File f(fn, true);
for (auto it: *this) {
f.write(it.first);
f.write("=");
f.write(it.second);
f.write("\r\n");
}
for (auto sect: sections) {
f.write("\r\n[");
f.write(sect.first);
f.write("]\r\n");
for (auto it: *sect.second) {
f.write(it.first);
f.write("=");
f.write(it.second);
f.write("\r\n");
}}
return true;
}


bool IniFile::load (const tstring& fn) {
File f(fn);
if (!f) return false;
string section = "";
for (string line: split(f.readFully(), "\n")) {
trim(line);
if (line.size()<1 || line[0]=='#' || line[0]==';') continue;
if (line[0]=='[') {
int pos = line.rfind(']');
if (pos<1 || pos>=line.size()) continue;
section = line.substr(1, pos -1);
trim(section);
continue;
}
int pos = line.find('=');
if (pos<0 || pos>=line.size()) continue;
string name = (line.substr(0, pos));
string value = (line.substr(pos+1));
trim(name);
trim(value);
if (section.size()>0) set3(section, name, value, true);
else set3(name, value, true);
}
return true;
}

void IniFile::fusion (IniFile& in, bool overwrite) {
for (auto it: in) {
if (overwrite || !contains(it.first)) set3(it.first, it.second);
}
for (auto inSect: in.sectionsByName) {
string section = inSect.first;
auto sect = sectionsByName[section];
if (!sect) {
sect = shared_ptr<IniFile>(new IniFile());
sectionsByName[section] = sect;
sections.push_back(pair<string,shared_ptr<IniFile>>(section,sect));
}
sect->fusion(*inSect.second);
}}

