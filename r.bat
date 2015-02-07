@echo off
for %%i in (*.cpp) do g++ -c -s -O3 -std=gnu++11 %%i -w -o obj\%%~ni.o
windres res.rc -o obj\rc.o
g++ -s -O3 -w obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lboost-regex -L. -lpython34 -mthreads -m32 -static-libstdc++ -static-libgcc