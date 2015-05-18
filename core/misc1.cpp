#include "global.h"
#include "strings.hpp"

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
