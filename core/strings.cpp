#include "global.h"
#include "strings.hpp"
#include<string>
#include<cwchar>
#include<cstdarg>
#include<boost/regex.hpp>
#include<boost/algorithm/string.hpp>
#include<unordered_map>
using namespace std;

#define DETECTION_MAX_LOOKUP 16384

#ifdef UNICODE
typedef boost::wregex tregex;
typedef boost::wcregex_iterator tcregex_iterator;
typedef boost::wcmatch tcmatch;
#else
typedef boost::regex tregex;
typedef boost::cregex_iterator tcregex_iterator;
typedef boost::cmatch tcmatch;
#endif

static unordered_map<int,string> pythonEncodings = {
{ 12000, "utf_32_le" },
{ 12001, "utf_32_be" },
{ 12002, "utf_32_le" },
{ 12003, "utf_32_be" },
{ 28600, "ISO-8859-10" },
{ 28604, "ISO-8859-14" },
{ 28606, "ISO-8859-16" },
{ 10951, "big5" },
{ 10950, "big5-hkscs" },
{ 1006, "cp1006" },
{ 1125, "cp1125" },
{ 17154, "ptcp154" },
};

void EnumPythonBonusCP (vector<int>& v) {
for (auto it: pythonEncodings) v.push_back(it.first);
}

string encodeToPythonEncoding (const tstring& str, const string& prefix, const char* encoding) ;
tstring decodeFromPythonEncoding (const string& str, int offset, const char* encoding) ;

string snsprintf (int max, const string& fmt, ...) {
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
case CP_UTF8: return toTString(toWString(str, encoding));
case CP_UTF8_BOM: return toTString( toWString(str, CP_UTF8, 3, 0));
case CP_UTF16_LE: return toTString(wstring( (const wchar_t*)(str.data()), str.size()/2));
case CP_UTF16_LE_BOM: return toTString(wstring( (const wchar_t*)(str.data()+2), str.size()/2 -1));
case CP_UTF16_BE: return toTString(utf16beSwitchEndianess(wstring( (const wchar_t*)(str.data()), str.size()/2)));
case CP_UTF16_BE_BOM: return toTString(utf16beSwitchEndianess(wstring( (const wchar_t*)(str.data()+2), str.size()/2 -1)));
case CP_UTF32_LE_BOM: return decodeFromPythonEncoding(str, 4, "utf_32_le");
case CP_UTF32_BE_BOM: return decodeFromPythonEncoding(str, 4, "utf_32_be");
default: {
auto it = pythonEncodings.find(encoding);
if (it!=pythonEncodings.end()) return decodeFromPythonEncoding(str, 0, it->second.c_str() );
else return toTString(toWString(str, encoding));
}}}

string ConvertToEncoding (const tstring& str, int encoding) {
switch(encoding){
case CP_UTF8: return toString(str, encoding);
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
case CP_UTF32_LE_BOM: return encodeToPythonEncoding(str, string("\xFF\xFE\0\0",4), "utf_32_le");
case CP_UTF32_BE_BOM: return encodeToPythonEncoding(str, string("\0\0\xFE\xFF",4), "utf_32_be");
default: {
auto it = pythonEncodings.find(encoding);
if (it!=pythonEncodings.end()) return encodeToPythonEncoding(str, "", it->second.c_str() );
else return toString(str, encoding);
}}}

static inline BOOL testUtf8rule (const unsigned char** x, int n) {
int i = 0;
while (i<n && (*++(*x)&0xC0)==0x80) i++;
return i==n;
}

int guessEncoding (const unsigned char* ch, int len, int def, int acpdef, int oemdef) {
if (len>=3 && ch[0]==0xEF && ch[1]==0xBB && ch[2]==0xBF) return CP_UTF8_BOM;
//if (len>=8 && ch[0]==255 && ch[1]==254 && ch[2]==0 && ch[3]==0) return CP_UTF32_LE_BOM;
if (len>=2 && ch[0]==255 && ch[1]==254) return CP_UTF16_LE_BOM;
if (len>=2 && ch[0]==254 && ch[1]==255) return CP_UTF16_BE_BOM;
//if (len>=8 && ch[2]==254 && ch[3]==255 && ch[0]==0 && ch[1]==0) return CP_UTF32_BE_BOM;
//if (len>=8 && ch[0]!=0 && ch[1]==0 && ch[2]==0 && ch[3]==0 && ch[4]!=0 && ch[5]==0 && ch[6]==0 && ch[7]==0) return CP_UTF32_LE;
//if (len>=8 && ch[0]==0 && ch[1]==0 && ch[2]==0 && ch[3]!=0 && ch[4]==0 && ch[5]==0 && ch[6]==0 && ch[7]!=0) return CP_UTF32_BE;
if (len>=6 && ch[1]==0 && ch[3]==0 && ch[5]==0) return CP_UTF16_LE;
if (len>=6 && ch[0]==0 && ch[2]==0 && ch[4]==0) return CP_UTF16_BE;
BOOL encutf = FALSE;
int count = 0;
for (const unsigned char* x = ch; *x && count<len && count<DETECTION_MAX_LOOKUP; ++x, count++) {
if (*x<0x80) continue;
if (*x==164) return CP_ISO_8859_15;
else if (*x>=0x80 && *x<=0xA0 && *x!=146) return oemdef;
else if ((*x>=0x80 && *x<0xC0) || *x>=248) return acpdef;
else if (*x>=0xF0 && !testUtf8rule(&x, 3)) return acpdef;
else if (*x>=0xE0 && !testUtf8rule(&x, 2)) return acpdef;
else if (*x>=0xC0 && !testUtf8rule(&x, 1)) return acpdef;
encutf = TRUE;
}
return encutf? CP_UTF8 : def;
}

int guessLineEnding (const TCHAR* s, int len, int def) {
int count = 0;
while(*s && count++<DETECTION_MAX_LOOKUP && count<len){
if (*s=='\n') return LE_UNIX;
else if (*s=='\r') return *(++s) == '\n'? LE_DOS : LE_MAC;
else if (*s==0x2028 || *s==0x2029) return LE_LS;
else if (*s==30) return LE_RS;
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

/*void normalizeLineEndings (tstring& text) {
text = preg_replace(text, TEXT("\r\n|\n|\r"), TEXT("\r\n") );
}*/

bool preg_check (const tstring& regex, bool rethrow) {
using namespace boost;
try {
tregex reg(regex, regex_constants::perl | regex_constants::mod_s | regex_constants::collate | regex_constants::nosubs);
return true;
} catch (const std::exception& e) {
if (rethrow) throw;
else return false;
}}

pair<int,int> preg_search (const tstring& text, const tstring& needle, int pos, bool icase, bool literal) {
try {
using namespace boost;
int options = 0;
if (literal) options |= regex_constants::literal;
else options |= regex_constants::perl | regex_constants::mod_s | regex_constants::collate | regex_constants::nosubs;
if (icase) options |= regex_constants::icase;
tregex reg(needle, options);
tcmatch m;
match_flag_type mtype = match_flag_type::match_default;
if (pos>0) mtype |= match_flag_type::match_prev_avail;
if (regex_search(text.data()+pos, text.data()+text.size(), m, reg, mtype)) return pair<int,int>(
m[0].first - text.data(),
m[0].second - text.data() );
} catch (const exception& e) {}
return pair<int,int>(-1,-1);
}

pair<int,int> preg_rsearch (const tstring& text, const tstring& needle, int pos, bool icase, bool literal) {
try {
using namespace boost;
int lastStart=-1, lastEnd=-1, options = 0;
if (literal) options |= regex_constants::literal;
else options |= regex_constants::perl | regex_constants::mod_s | regex_constants::collate | regex_constants::nosubs;
if (icase) options |= regex_constants::icase;
tregex reg(needle, options);
match_flag_type mtype = match_flag_type::match_default;
if (pos>0) mtype |= match_flag_type::match_prev_avail;
for (tcregex_iterator _end, it(text.data(), text.data()+text.size(), reg, mtype); it!=_end; ++it) {
auto m = *it;
int start = m[0].first - text.data();
int end = m[0].second - text.data();
if (start>pos) break;
lastStart=start;
lastEnd=end;
}
if (lastStart>=0 && lastEnd>=0 && lastStart<=pos && lastEnd<=pos) return pair<int,int>(lastStart, lastEnd);
} catch (const exception& e) {}
return pair<int,int>(-1,-1);
}

tstring preg_replace (const tstring& str, const tstring& needle, const tstring& repl, bool icase, bool literal) {
try {
using namespace boost;
int options = 0;
if (literal) options |= regex_constants::literal;
else options |= regex_constants::perl | regex_constants::mod_s | regex_constants::collate;
if (icase) options |= regex_constants::icase;
match_flag_type flags = 
(literal? match_flag_type::format_literal :
(match_flag_type::match_default | match_flag_type::format_perl) );
tregex reg(needle, options);
return regex_replace(str, reg, repl, flags);
} catch (const exception& e) { return str; }
}

tstring str_replace (const tstring& str, const std::vector<std::pair<tstring,tstring>>& pairs) {
tstring re = str;
for (auto p: pairs) re = str_replace(re, p.first, p.second);
return re;
}

void export ParseLineCol (tstring& file, int& line, int& col) {
using namespace boost;
tregex r1(TEXT(":(\\d+):(\\d+)$"), regex_constants::perl | regex_constants::mod_s | regex_constants::collate);
tcmatch m;
if (regex_search(file.data(), file.data() + file.size(), m, r1, match_flag_type::match_default)) {
int start = m[0].first - file.data();
line = toInt(m[1].str());
col = toInt(m[2].str());
file = file.substr(0, start);
return;
}
tregex r2(TEXT(":(\\d+)$"), regex_constants::perl | regex_constants::mod_s | regex_constants::collate);
if (regex_search(file.data(), file.data() + file.size(), m, r2, match_flag_type::match_default)) {
int start = m[0].first - file.data();
line = toInt(m[1].str());
col = 1;
file = file.substr(0, start);
return;
}}

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
text = preg_replace(text, TEXT("\r\n|\n|\r"), TEXT("\r\n"));
int pos = text.find_first_not_of(TEXT(" \t"));
if (pos<text.size()) text.erase(text.begin(), text.begin()+pos);
}



