@echo off
for %%i in (*.cpp) do g++ -c -s -O3 -std=gnu++11 %%i -w -o obj\%%~ni.o -DRELEASE
windres res.rc -o obj\rc.o
g++ -o 6pad++.exe -s -O3 -w obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lboost-regex -L. -lpython34 -mthreads -m32 -DRELEASE
rem -static-libstdc++ -static-libgcc
upx 6pad++.exe *.dll
del obj\*.o
copy a.lng "6pad++.lng"
call pycompile.bat
call d.bat
zip -9 -u -q -r 6pad++.zip 6pad++.exe python34.zip python34.dll libgcc_s_dw2-1.dll libstdc++-6.dll 6pad++.lng doc\*.html