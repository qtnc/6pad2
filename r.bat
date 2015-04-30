@echo off
for %%i in (*.cpp) do g++ -c -s -O3 -std=gnu++11 %%i -w -o obj\%%~ni.o -DRELEASE
windres res.rc -o obj\rc.o
g++ -o 6pad++.exe -s -O3 -w obj\*.o -luser32 -lkernel32 -lcomdlg32 -lcomctl32 -lgdi32 -lwinmm -lshlwapi -lboost-regex -L. -lpython34 -mthreads -mwindows -m32 -DRELEASE
rem -static-libstdc++ -static-libgcc
del obj\*.o
copy a-french.lng "6pad++-french.lng"
copy a-english.lng "6pad++-english.lng"
call pycompile.bat
call d.bat
zip -9 -u -q -r 6pad++.zip 6pad++.exe python34.zip *.dll 6pad*.lng doc\*.html lib\*.dll lib\*.pyd
del "6pad++.lng"