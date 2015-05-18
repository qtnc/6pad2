@echo off
del obj\*.o
windres res.rc -o obj\rc.o
for %%i in (*.cpp) do g++ -c -s -O3  -std=gnu++11 %%i -w -o obj\%%~ni.o -I../core -DRELEASE
g++ -specs=msvcr100 -s -O3  -w -o "..\6pad++.exe" obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lshlwapi -lboost-regex -L../link -lUniversalSpeech -lpython34 -lqc6pad10 -mthreads -m32 -DRELEASE
del obj\*.o