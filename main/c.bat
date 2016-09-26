@echo off
if not exist obj md obj
isnewer res.rc obj\rc.o && windres res.rc -o obj\rc.o
for %%i in (*.cpp) do isnewer %%i obj\%%~ni.o && g++ -c -g -std=gnu++14 %%i -w -o obj\%%~ni.o -I../core -DDEBUG
g++ -specs=msvcr100 -g -w -o ../a.exe obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lshlwapi -L../link -lboost_regex_1_61_0 -lUniversalSpeech -lpython34 -lqc6pad10 -m32 -mthreads -DDEBUG