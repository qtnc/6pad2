@echo off
if not exist obj md obj
isnewer res.rc obj\rc.o && windres res.rc -o obj\rc.o -DDEBUG -DSPDLL
for %%i in (*.cpp) do isnewer %%i obj\%%~ni.o && g++ -c -g -std=gnu++14 %%i -w -o obj\%%~ni.o -DDEBUG -DSPDLL
g++ -specs=msvcr100 -g -w -shared -o ../qc6pad10.dll obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lshell32 -lwinmm -lshlwapi -L../link -lboost_regex_1_61_0 -lUniversalSpeech -lpython34 -m32 -mthreads -Wl,--out-implib,../link/libqc6pad10.a -DDEBUG -DSPDLL