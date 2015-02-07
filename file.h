#ifndef ___FILE_H9
#define ___FILE_H9
#include "global.h"


struct File {
HANDLE fd;
File();
File (const tstring& path, bool write=false);
bool open (const tstring& path, bool write=false);
int read (void* buf, int len);
int write (const void* buf, int len = -1);
int write (const string& str) ;
bool writeFully (const void* buf, int len) ;
string readFully () ;
string readUntil (char c = '\n', char ign = '\r') ;
string readLine () { return readUntil(); }
void close () ;
~File();
operator bool () ;
};

#endif