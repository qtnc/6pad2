#include "global.h"
#include "python34.h"

extern "C" FILE* msvcfopen (const char* name, const char* ax) ;
extern "C" void msvcfclose (FILE*);

extern vector<tstring> argv;
extern tstring appName;

void PyStart (void) {
Py_SetProgramName(const_cast<wchar_t*>(toWString(argv[0]).c_str()));
Py_Initialize();
GIL_PROTECT
string pyfn = toString(appName + TEXT(".py"));
FILE* fp = msvcfopen(pyfn.c_str(), "r");
if (fp) {
PyRun_SimpleFile(fp, pyfn.c_str() );
msvcfclose(fp);
}
// Ohter initialization stuff
}
