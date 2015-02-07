#include "inifile.h"
#include "file.h"

bool IniFile::save (const tstring& fn) {
File f(fn, true);
for (auto it: *this) {
f.write(it.first);
f.write("=");
f.write(it.second);
f.write("\r\n");
}
return true;
}


bool IniFile::load (const tstring& fn) {
File f(fn);
if (!f) return false;
while(f){
string line = f.readLine();
trim(line);
if (line.size()<1 || line[0]=='#' || line[0]==';') continue;
int pos = line.find('=');
if (pos<0 || pos>=line.size()) continue;
string name = (line.substr(0, pos));
string value = (line.substr(pos+1));
trim(name);
trim(value);
set3(name, value);
}
return true;
}

