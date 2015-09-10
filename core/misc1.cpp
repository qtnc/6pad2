#include "global.h"
#include "strings.hpp"
#include "sixpad.h"

static vector<int> enclist = { 1200, 1201, 1202, 1203, 65002 };

void EnumPythonBonusCP (vector<int>& v);

static BOOL CALLBACK EnumProc1 (LPTSTR cpstr) {
int enc = toInt(tstring(cpstr));
enclist.push_back(enc);
return true;
}

const vector<int>& getAllAvailableEncodings () {
if (enclist.size()<=5) {
EnumSystemCodePages(EnumProc1, CP_INSTALLED);
EnumPythonBonusCP(enclist);
}
return enclist;
}

tstring export GetErrorText (int errorCode) {
TCHAR buf[512]={0};
FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 511, NULL);
return buf;
}

