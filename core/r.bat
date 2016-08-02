@echo off
if not exist obj md obj
del obj\*.o
windres res.rc -o obj\rc.o -DRELEASE -DSPDLL
for %%i in (*.cpp) do g++ -c -O3 -s  -std=gnu++11 %%i -w -o obj\%%~ni.o -DRELEASE -DSPDLL -fno-rtti
g++ -specs=msvcr100 -s -O3 -w -shared -o ../qc6pad10.dll obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lshell32 -lwinmm -lshlwapi -L../link -lboost_regex_1_55_0 -lUniversalSpeech -lpython34 -mthreads -m32 -Wl,--out-implib,../link/libqc6pad10.a -DRELEASE -DSPDLL -fno-rtti
del obj\*.o