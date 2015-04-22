#include "global.h"
#include "strings.hpp"
#include<string>
#include<cwchar>
#include<cstdarg>
#include<boost/regex.hpp>
#include<boost/algorithm/string.hpp>
using namespace std;

#define DETECTION_MAX_LOOKUP 16384

vector<string> split (const string& str, const string& delims) {
using namespace boost;
vector<string> v;
split(v, str, is_any_of(delims));
return v;
}

vector<wstring> split (const wstring& str, const wstring& delims) {
using namespace boost;
vector<wstring> v;
split(v, str, is_any_of(delims));
return v;
}

void toLowerCase (tstring& s) {
boost::to_lower(s);
}

void toUpperCase (tstring& s) {
boost::to_upper(s);
}

bool startsWith (const tstring& a, const tstring& b) {
return boost::starts_with(a,b);
}

bool endsWith (const tstring& a, const tstring& b) {
return boost::ends_with(a,b);
}

string snprintf (int max, const string& fmt, ...) {
string out(max+1, '\0');
va_list ap;
va_start(ap,fmt);
stringSize(out) = vsnprintf((char*)out.data(), max, fmt.c_str(), ap);
va_end(ap);
return out;
}

wstring snwprintf (int max, const wstring& fmt, ...) {
wstring out(max+1, L'\0');
out.reserve(max);
va_list ap;
va_start(ap,fmt);
stringSize(out) = vsnwprintf((wchar_t*)out.data(), max, fmt.c_str(), ap);
va_end(ap);
return out;
}

static wstring& utf16beSwitchEndianess (wstring& s) {
union cvt { wchar_t w; char c[2]; } buf;
char c;
for (int i=0; i<s.size(); i++) {
buf.w = s[i];
c = buf.c[0];
buf.c[0] = buf.c[1];
buf.c[1] = c;
s[i] = buf.w;
}
return s;
}

static wstring& utf16beSwitchEndianess (wstring&& s) {
union cvt { wchar_t w; char c[2]; } buf;
char c;
for (int i=0; i<s.size(); i++) {
buf.w = s[i];
c = buf.c[0];
buf.c[0] = buf.c[1];
buf.c[1] = c;
s[i] = buf.w;
}
return s;
}

tstring ConvertFromEncoding (const string& str, int encoding) {
switch(encoding){
case CP_UTF8_BOM: return toTString( toWString(str, CP_UTF8, 3, 0));
case CP_UTF16_LE: return toTString(wstring( (const wchar_t*)(str.data()), str.size()/2));
case CP_UTF16_LE_BOM: return toTString(wstring( (const wchar_t*)(str.data()+2), str.size()/2 -1));
case CP_UTF16_BE: return toTString(utf16beSwitchEndianess(wstring( (const wchar_t*)(str.data()), str.size()/2)));
case CP_UTF16_BE_BOM: return toTString(utf16beSwitchEndianess(wstring( (const wchar_t*)(str.data()+2), str.size()/2 -1)));
default: return toTString(toWString(str, encoding));
}}

string ConvertToEncoding (const tstring& str, int encoding) {
switch(encoding){
case CP_UTF8_BOM:  {
string out = toString(str, CP_UTF8, 0, 3);
memcpy((char*)out.data(), "\xEF\xBB\xBF", 3);
return out;
}break;
case CP_UTF16_BE :
case CP_UTF16_LE_BOM:
case CP_UTF16_BE_BOM: 
{
int offset = (encoding!=CP_UTF16_BE? 2 : 0);
wstring ws = toWString(str);
if (encoding!=CP_UTF16_LE_BOM) utf16beSwitchEndianess(ws);
string out(str.size()*2 +offset, '\0');
memcpy((char*)out.data() +offset, (char*)ws.data(), ws.size()*2);
if (encoding==CP_UTF16_LE_BOM) memcpy((char*)out.data(), "\xFF\xFE", 2);
else if (encoding==CP_UTF16_BE_BOM) memcpy((char*)out.data(), "\xFE\xFF", 2);
return out;
}break;
case CP_UTF16_LE: return string((const char*)str.data(), str.size()*2);
default: return toString(str, encoding);
}}

static inline BOOL testUtf8rule (const unsigned char** x, int n) {
int i = 0;
while (i<n && (*++(*x)&0xC0)==0x80) i++;
return i==n;
}

int guessEncoding (const unsigned char* ch, int def) {
if (ch[0]==0xEF && ch[1]==0xBB && ch[2]==0xBF) return CP_UTF8_BOM;
if (ch[0]==255 && ch[1]==254) return CP_UTF16_LE_BOM;
if (ch[0]==254 && ch[1]==255) return CP_UTF16_BE_BOM;
if (ch[1]==0 && ch[3]==0 && ch[5]==0) return CP_UTF16_LE;
if (ch[0]==0 && ch[2]==0 && ch[4]==0) return CP_UTF16_BE;
BOOL encutf = FALSE;
for (const unsigned char* x = ch; *x; ++x) {
if (*x<0x80) continue;
if (*x>=0x81 && *x<=0x90) return CP_MSDOS; 
else if (*x==164) return CP_ISO_8859_15;
else if ((*x>=0x80 && *x<0xC0) || *x>=248) return CP_ACP;
else if (*x>=0xF0 && !testUtf8rule(&x, 3)) return CP_ACP;
else if (*x>=0xE0 && !testUtf8rule(&x, 2)) return CP_ACP;
else if (*x>=0xC0 && !testUtf8rule(&x, 1)) return CP_ACP;
encutf = TRUE;
}
return encutf? CP_UTF8 : def;
}

int guessLineEnding (const TCHAR* s, int def) {
while(*s){
if (*s=='\n') return LE_UNIX;
else if (*s=='\r') return *(++s) == '\n'? LE_DOS : LE_MAC;
else ++s;
}
return def;
}

int guessIndentationMode (const TCHAR* s, int l, int def) {
for (int i=0; i<l && i<DETECTION_MAX_LOOKUP; i++) {
if (s[i]=='\n') {
if (s[i+1]=='\t') return 0;
else if (s[i+1]==' ') {
int j = i+1;
while (j<l && s[j]==' ') j++;
return j-i-1;
}}}
return def;
}

static inline bool isSpace (TCHAR c) {
return c>=0 && c<=32 && c!=10 && c!=13;
}

void normalizeLineEndings (tstring& text) {
using namespace boost;
typedef boost::wregex tregex;
const int options = regex_constants::literal;
const match_flag_type flags = match_flag_type::format_literal;
tregex reg2(TEXT("(?:\r\n|\r|\n)"), options);
text = regex_replace(text, reg2, TEXT("\r\n"), flags);
}

tstring preg_replace (tstring& str, const tstring& needle, const tstring& repl) {
using namespace boost;
typedef boost::wregex tregex;
const int options = regex_constants::perl | regex_constants::mod_s | regex_constants::collate;
const match_flag_type flags = match_flag_type::match_default | match_flag_type::format_perl;
tregex reg(needle, options);
return regex_replace(str, reg, repl, flags);
}

tstring str_replace (tstring& str, const tstring& needle, const tstring& repl) {
using namespace boost;
typedef boost::wregex tregex;
const int options = regex_constants::literal;
const match_flag_type flags = match_flag_type::format_literal;
tregex reg(needle, options);
return regex_replace(str, reg, repl, flags);
}

void PrepareSmartPaste (tstring& text, const tstring& indent) {
using namespace boost;
typedef boost::wregex tregex;
const int options = regex_constants::perl | regex_constants::mod_s | regex_constants::collate;
const match_flag_type flags = match_flag_type::match_default | match_flag_type::format_perl;
int commonIndent = 1<<30;
for (int i=0, j=0, n=text.size(); i<n; i++) {
for (j=i; j-i<commonIndent && isSpace(text[j]); j++);
if (j-i<commonIndent) commonIndent=j-i;
while(i<n && text[i]!='\n') i++;
}
tregex reg(TEXT("^") + text.substr(0, commonIndent), options);
text = regex_replace(text, reg, indent, flags);
normalizeLineEndings(text);
int pos = text.find_first_not_of(TEXT(" \t"));
if (pos<text.size()) text.erase(text.begin(), text.begin()+pos);
}

