#include "file.h"
using namespace std;

File::File () : fd(0), noclose(false)  { }
File::File (const tstring& path, bool write): fd(0), noclose(false) { open(path,write); }
File::~File () { close(); }
File::operator bool () { return fd && fd!=INVALID_HANDLE_VALUE; }

void File::close () { 
if (fd&&!noclose) CloseHandle(fd);
fd=0;
}

bool File::open (const tstring& path, bool write) {
if (path==TEXT("STDIN")) { fd = GetStdHandle(STD_INPUT_HANDLE); noclose=true; }
else if (path==TEXT("STDOUT")) fd = GetStdHandle(STD_OUTPUT_HANDLE);
else if (path==TEXT("STDERR")) fd = GetStdHandle(STD_ERROR_HANDLE);
else if (write) fd = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
else fd = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
return fd!=INVALID_HANDLE_VALUE;
}

int File::read (void* buf, int len) {
DWORD nRead=0;
if (ReadFile(fd, buf, len, &nRead, NULL)) return nRead;
else { close(); return -1; }
}

int File::write (const void* buf, int len) {
DWORD nWritten=0;
if (len<0) len = strlen((const char*)buf);
if (WriteFile(fd, buf, len, &nWritten, NULL)) return nWritten;
else { close(); return -1; }
}

int File::write (const string& s) {
return write(s.c_str(), s.size());
}

string File::readFully () {
DWORD cap = 4096, pos=0, nRead=0;
string str;
str.resize(cap);
while (ReadFile(fd, (char*)(str.data()+pos), cap-pos, &nRead, NULL)) {
if (nRead<=0) break;
pos += nRead;
if (pos>=cap-32) str.resize(cap = cap*3/2+1);
}
char* z = (char*)(str.data() +pos); *z=0;
str.resize(pos);
stringSize(str) = pos;
close();
return str;
}

bool File::writeFully (const void* buf, int len) {
DWORD pos=0, nWritten=0;
while (pos<len && WriteFile(fd, buf+pos, len-pos, &nWritten, NULL)) pos+=nWritten;
return pos>=len;
}


string File::readUntil (char lim, char ign) {
int n;
char c;
string s = "";
while((n=read(&c,1))>0) {
if (c==ign) continue;
else if (c==lim) break;
s+=c;
}
if (n<=0) close();
return s;
}

