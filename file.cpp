#include "File.h"
#include<shlwapi.h>
using namespace std;

struct StdFile: IO {
HANDLE fd;

StdFile (HANDLE h=0): fd(h) {}

int Read (void* buf, int len) {
if (!fd) return -1;
DWORD nRead=0;
if (ReadFile(fd, buf, len, &nRead, NULL)) return nRead;
else { Close(); return -1; }
}

int Write (const void* buf, int len) {
DWORD nWritten=0;
if (!fd) return -1;
if (len<0) len = strlen((const char*)buf);
if (WriteFile(fd, buf, len, &nWritten, NULL)) return nWritten;
else { Close(); return -1; }
}

void Flush () { if (fd) FlushFileBuffers(fd); }
void Close () { if (fd) CloseHandle(fd); fd=INVALID_HANDLE_VALUE; }
bool IsClosed () { return !fd || fd==INVALID_HANDLE_VALUE; }

static StdFile* Open (const tstring& path, bool write, bool append) {
HANDLE fd = NULL;
if (write) fd = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, append? OPEN_ALWAYS : CREATE_ALWAYS, 0, NULL);
else fd = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
if (fd) return new StdFile(fd);
else return NULL;
}

};//StdFile

IO* FileURIProtocolHandler (const tstring& uri, bool write) {
if (!UrlIsFileUrl(uri.c_str())) return NULL;
DWORD pathlen = 300;
TCHAR path[300] = {0};
if (S_OK!=PathCreateFromUrl(uri.c_str(), path, &pathlen, NULL)) return NULL;
path[pathlen]=0;
return StdFile::Open(path, write, false);
}

void File::close () { 
if (io){
io->Close();
delete io;
io = NULL;
}}

void File::flush () {
if (io) io->Flush();
}

bool File::open (const tstring& path, bool write, bool append) {
//if (path==TEXT("STDIN")) { fd = GetStdHandle(STD_INPUT_HANDLE); noclose=true; }
//else if (path==TEXT("STDOUT")) fd = GetStdHandle(STD_OUTPUT_HANDLE);
//else if (path==TEXT("STDERR")) fd = GetStdHandle(STD_ERROR_HANDLE);
int dot = path.find(':');
if (dot>1 && dot<=5) { // Handling custom protocols
for (auto handler: protocolHandlers) {
if (io = handler(path, write)) break;
}}
if (!io) io = StdFile::Open(path, write, append);
return !!io;
}

int File::read (void* buf, int len) {
if (io) return io->Read(buf,len);
else return -1;
}

int File::write (const void* buf, int len) {
if (io) return io->Write(buf, len);
else return -1;
}

int File::write (const string& s) {
return write(s.c_str(), s.size());
}

string File::readFully () {
if (!io) return "";
int cap = 4096, pos=0, nRead=0;
string str;
str.resize(cap);
while ((nRead = read((char*)(str.data()+pos), cap-pos))>0) {
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
int pos=0, nWritten=0;
if (!io) return false;
while (pos<len && (nWritten=write(buf+pos, len-pos))>0) pos+=nWritten;
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

vector<function<IO*(const tstring&,bool)>> File::protocolHandlers = {
FileURIProtocolHandler
};

