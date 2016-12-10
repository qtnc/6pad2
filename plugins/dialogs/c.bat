@echo off
if not exist obj md obj
isnewer res.rc obj\rc.o && windres res.rc -o obj\rc.o -DDEBUG -DSPDLL
for %%i in (*.cpp) do isnewer %%i obj\%%~ni.o && g++ -c -g -std=gnu++14 -fno-rtti %%i -w -o obj\%%~ni.o -DDEBUG -DSPDLL
g++ -specs=msvcr100 -g -w -shared -o ../qc6paddlgs.pyd obj\*.o -fno-rtti -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -L../../link -lqc6pad10 -lpython34 -mthreads -m32 -DDEBUG -DSPDLL