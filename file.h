#ifndef ___FILE_H9
#define ___FILE_H9
#include "global.h"

struct IO {
virtual int Read (void*, int) = 0;
virtual int Write (const void*, int) = 0;
virtual void Flush () = 0;
virtual void Close () = 0;
virtual bool IsClosed () = 0;
};

struct File {
IO* io;
bool open (const tstring& path, bool write=false, bool append=false);
int read (void* buf, int len);
int write (const void* buf, int len = -1);
int write (const string& str) ;
bool writeFully (const void* buf, int len) ;
string readFully () ;
string readUntil (char c = '\n', char ign = '\r') ;
string readLine () { return readUntil(); }
void close () ;
void flush () ;
inline File () : io(0)  { }
inline File (const tstring& path, bool write=false, bool append=false): io(0) { open(path,write, append); }
inline File (const File&) = delete;
inline File (File&&) = default;
inline File& operator= (const File&) = delete;
inline File& operator= (File&&) = default;
inline ~File() { close(); }
inline operator bool () {   return io && !io->IsClosed();   }
inline File& operator<< (const string& s) { write(s); return *this; }
inline File& operator<< (const char* s) { return operator<<(string(s)); }
template<class T> inline File& operator<< (const T& x) { return operator<<(toString(x)); }

static void registerHandler (const function<IO*(const tstring&,bool)>&);
static std::vector<std::function<IO*(const tstring&, bool)>> protocolHandlers;
};

#endif