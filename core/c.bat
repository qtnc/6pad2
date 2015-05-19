@echo off
isnewer res.rc obj\rc.o && windres res.rc -o obj\rc.o -DDEBUG -DSPDLL
for %%i in (*.cpp) do isnewer %%i obj\%%~ni.o && g++ -c -g -std=gnu++11 %%i -w -o obj\%%~ni.o -DDEBUG -DSPDLL
g++ -specs=msvcr100 -g -w -shared -o ../qc6pad10.dll obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lshlwapi -lboost-regex -L../link -lUniversalSpeech -lpython34 -mthreads -m32 -Wl,--out-implib,../link/libqc6pad10.a -DDEBUG -DSPDLL