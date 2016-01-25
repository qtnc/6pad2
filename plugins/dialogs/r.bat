@echo off
if not exist obj md obj
del obj\*.o
windres res.rc -o obj\rc.o -DRELEASE -DSPDLL
for %%i in (*.cpp) do g++ -c -s -O3 -std=gnu++11 %%i -w -o obj\%%~ni.o -DDRELEASE -DSPDLL -fno-rtti -Os
g++ -specs=msvcr100 -w -s -O3 -shared -o ../qc6paddlgs.pyd obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -L../../link -lqc6pad10 -lpython34 -mthreads -m32 -DRELEASE -DSPDLL -fno-rtti -Os