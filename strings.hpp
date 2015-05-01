#ifndef _____STRINGS_HPP_8_____
#define _____STRINGS_HPP_8_____
#include "global.h"
#include<string>
#include<cstring>
#include<cstdlib>
#include<boost/algorithm/string.hpp>
using boost::to_lower;
using boost::to_upper;
using boost::ends_with;
using boost::starts_with;
using boost::trim;
using boost::split;
using boost::is_any_of;

tstring str_replace (const tstring& str, const tstring& needle, const tstring& repl);
tstring preg_replace (const tstring& str, const tstring& needle, const tstring& repl);
tstring str_replace (const tstring& str, const std::vector<std::pair<tstring,tstring>>& pairs);
void normalizeLineEndings (tstring& text) ;

template<class T> std::vector<std::basic_string<T>> split (const std::basic_string<T>& str, const std::basic_string<T>& delims) {
std::vector<std::basic_string<T>> v;
split(v, str, boost::is_any_of(delims));
return v;
}

template<class T> std::vector<std::basic_string<T>> split (const std::basic_string<T>& str, const T* delims) {
return split(str, std::basic_string<T>(delims));
}

// Sprintf++
std::string snsprintf (int max, const std::string& fmt, ...) ;
std::wstring snwprintf (int max, const std::wstring& fmt, ...) ;

// Conversion 

inline int toInt (const std::string& str, int base=0) {
return strtol(str.c_str(), 0, base);
}

inline int toInt (const std::wstring& str, int base=0) {
return wcstol(str.c_str(), 0, base);
}

inline long long toLongLong (const std::string& str, int base=0) {
return strtoll(str.c_str(), 0, base);
}

inline long long toLongLong (const std::wstring& str, int base=0) {
return wcstoll(str.c_str(), 0, base);
}

inline unsigned long long toUnsignedLongLong (const std::string& str, int base=0) {
return strtoull(str.c_str(), 0, base);
}

inline unsigned long long toUnsignedLongLong (const std::wstring& str, int base=0) {
return wcstoull(str.c_str(), 0, base);
}

inline double toDouble (const std::string& str) {
return strtod(str.c_str(), 0);
}

inline double toDouble (const std::wstring& str) {
return wcstod(str.c_str(), 0);
}

inline float toFloat (const std::string& str) {
return strtof(str.c_str(), 0);
}

inline float toFloat (const std::wstring& str) {
return wcstof(str.c_str(), 0);
}

inline bool toBool (const std::string& str) {
return str=="1" || !stricmp(str.c_str(),"true") || !stricmp(str.c_str(),"on");
}

inline bool toBool (const std::wstring& str) {
return str==L"1" || !wcsicmp(str.c_str(),L"true") || !wcsicmp(str.c_str(),L"on");
}

template <class T> inline size_t& stringSize (std::basic_string<T>& s) {
return ((size_t*)(s.data()))[-3];
}

inline const std::string& toString (const std::string& s) { return s; }
inline const std::wstring& toWString (const std::wstring& ws) { return ws; }
inline std::string toString (const char* str) { return std::string(str?str:""); }
inline std::wstring toWString (const wchar_t* ws) { return std::wstring(ws?ws:L""); }
inline std::wstring toWString2 (const wchar_t* ws) { return toWString(ws); }

inline std::string toString (const std::wstring& ws, int cp = CP_UTF8, int inOffset=0, int outOffset = 0) {
size_t nSize = WideCharToMultiByte(cp, 0, ws.data()+inOffset, ws.size()-inOffset, NULL, 0, NULL, NULL);
std::string s(nSize+outOffset, '\0');
stringSize(s) = outOffset + WideCharToMultiByte(cp, 0, ws.data()+inOffset, ws.size()-inOffset, (char*)(s.data() + outOffset), nSize, NULL, NULL);
return s;
}

inline std::wstring toWString (const std::string& s, int cp = CP_UTF8, int inOffset=0, int outOffset = 0) {
size_t nSize = MultiByteToWideChar(cp, 0, s.data()+inOffset, s.size()-inOffset, NULL, 0);
std::wstring ws(nSize+outOffset, L'\0');
stringSize(ws) = outOffset + MultiByteToWideChar(cp, 0, s.data()+inOffset, s.size()-inOffset, (wchar_t*)(ws.data() + outOffset), nSize);
return ws;
}

inline std::string toString (const wchar_t* ws) { std::wstring z(ws?ws:L""); return toString(z); }
inline std::wstring toWString (const char* str) { std::string z(str?str:""); return toWString(z); }

template <class T> inline T fromString (const std::string& s) { throw 0; }
template<> inline std::string fromString (const std::string& s) { return s; }
template<> inline std::wstring fromString (const std::string& s) { return toWString(s); }
template<> inline int fromString (const std::string& s) { return toInt(s); }
template<> inline long long fromString (const std::string& s) { return toLongLong(s); }
template<> inline unsigned long long fromString (const std::string& s) { return toUnsignedLongLong(s); }
template<> inline double fromString (const std::string& s) { return toDouble(s); }
template<> inline float fromString (const std::string& s) { return (float)toDouble(s); }
template<> inline bool fromString (const std::string& s) { return toBool(s); }

inline std::string toString (int n, int base = 10) {
char buf[64]={0};
ltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (int n, int base = 10) {
wchar_t buf[64]={0};
_itow(n, buf, base);
return buf;
}

inline std::string toString (long long n, int base = 10) {
char buf[64]={0};
lltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (long long n, int base = 10) {
wchar_t buf[64]={0};
_i64tow(n, buf, base);
return buf;
}

inline std::string toString (DWORD n, int base = 10) {
char buf[64]={0};
lltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (DWORD n, int base = 10) {
wchar_t buf[64]={0};
_i64tow(n, buf, base);
return buf;
}

inline std::string toString (unsigned int n, int base = 10) {
char buf[64]={0};
lltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (unsigned int n, int base = 10) {
wchar_t buf[64]={0};
_i64tow(n, buf, base);
return buf;
}

inline std::string toString (unsigned long long n, int base = 10) {
char buf[64]={0};
ulltoa(n, buf, base);
return buf;
}

inline std::wstring toWString (unsigned long long n, int base = 10) {
wchar_t buf[64]={0};
_ui64tow(n, buf, base);
return buf;
}

inline std::string toString (double d) {
char buf[64]={0};
snprintf(buf, 63, "%.14g", d);
return buf;
}

inline std::wstring toWString (double d) {
wchar_t buf[64]={0};
snwprintf(buf, 63, L"%.14g", d);
return buf;
}

inline std::string toString (bool b) {
return b?"true":"false";
}

inline std::wstring toWString (bool b) {
return (b?L"true":L"false");
}

int guessEncoding (const unsigned char* str, int def);
int guessIndentationMode (const TCHAR* str, int len, int def);
int guessLineEnding (const TCHAR* str, int def);


tstring ConvertFromEncoding (const std::string& str, int encoding);
std::string ConvertToEncoding (const tstring& str, int encoding);

#endif
