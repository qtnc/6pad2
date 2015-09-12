#include "File.h"
#include<shlwapi.h>
using namespace std;

struct StdFile: IO {
HANDLE fd;

StdFile (HANDLE h=0): fd(h) {}

int size () {
DWORD size = GetFileSize(fd, NULL);
if (size==INVALID_FILE_SIZE) return -1;
else return (int)size;
}

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
if (write) fd = CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, append? OPEN_ALWAYS : CREATE_ALWAYS, 0, NULL);
else fd = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
if (fd) return new StdFile(fd);
else return NULL;
}

};//StdFile

struct StdFilePipe: StdFile {
using StdFile::StdFile;
void Close () {}
int size () { return -1; }
};

IO* FileURIProtocolHandler (const tstring& uri, bool write, bool append) {
if (!UrlIsFileUrl(uri.c_str())) return NULL;
DWORD pathlen = 300;
TCHAR path[300] = {0};
if (S_OK!=PathCreateFromUrl(uri.c_str(), path, &pathlen, NULL)) return NULL;
path[pathlen]=0;
return StdFile::Open(path, write, append);
}

IO* StdstreamsProtocolHandler (const tstring& uri, bool write, bool append) {
if (starts_with(uri, TEXT("&in:")) && !write) return new StdFilePipe(GetStdHandle(STD_INPUT_HANDLE));
else if (starts_with(uri, TEXT("&out:")) && write) return new StdFilePipe(GetStdHandle(STD_OUTPUT_HANDLE));
else if (starts_with(uri, TEXT("&err:")) && write) return new StdFilePipe(GetStdHandle(STD_ERROR_HANDLE));
else return NULL;
}

void export File::close () { 
if (io){
io->Close();
delete io;
io = NULL;
}}

void export File::flush () {
if (io) io->Flush();
}

bool export File::open (const tstring& path, bool write, bool append) {
int dot = path.find(':');
if (dot>1 && dot<=5) { // Handling custom protocols
for (auto handler: protocolHandlers) {
if (io = handler(path, write, append)) break;
}}
if (!io) io = StdFile::Open(path, write, append);
return !!io;
}

int export File::read (void* buf, int len) {
if (io) return io->Read(buf,len);
else return -1;
}

int export File::write (const void* buf, int len) {
if (io) return io->Write(buf, len);
else return -1;
}

int export File::write (const string& s) {
return write(s.c_str(), s.size());
}

string export File::readFully () {
if (!io) return "";
int size = io->size();
string str;
int pos=0, nRead=0, cap = (size<0? 32768 : size);
str.resize(cap);
while ((nRead = read((char*)(str.data()+pos), cap-pos))>0) {
pos += nRead;
if (size<0 && pos>=cap-32) str.resize(cap = cap*3/2+1);
}
char* z = (char*)(str.data() +pos); *z=0;
str.resize(pos);
stringSize(str) = pos;
close();
return str;
}

bool export File::writeFully (const void* buf, int len) {
int pos=0, nWritten=0;
if (!io) return false;
while (pos<len && (nWritten=write(buf+pos, len-pos))>0) pos+=nWritten;
return pos>=len;
}

string export File::readUntil (char lim, char ign) {
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

void export File::normalizePath (tstring& file) {
int i = file.find(':');
if (i>=0 && i<=5 && i!=tstring::npos) return; // very probably wheither already an absolute path e.g. C:\xxx\yyy.zzz, or an URL path e.g. http://example.com/file.html
else if (file.find(TEXT("\\\\"))==0) return; // Very probably a network name e.g. \\machineX\SharedFolder\xyz.txt
else if (file.size()<1 || file[0]=='&') return; // special names, e.g. &out
TCHAR buf[512] = {0};
if (GetFullPathName(file.c_str(), 511, buf, NULL)) {
tstring absPath = buf;
if (absPath.size()>0) file = absPath;
}}

void export File::registerHandler (const function<IO*(const tstring&,bool,bool)>& f) {
File::protocolHandlers.push_back(f);
}

vector<function<IO*(const tstring&,bool,bool)>> File::protocolHandlers = {
FileURIProtocolHandler,
StdstreamsProtocolHandler
};

