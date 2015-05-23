@echo off
cd core
call r.bat
cd ..\main
call r.bat
cd ..
rem -static-libstdc++ -static-libgcc
copy a-french.lng "6pad++-french.lng"
copy a-english.lng "6pad++-english.lng"
call pycompile.bat
call d.bat
zip -9 -u -q -r 6pad++.zip 6pad++.exe python34.zip *.dll 6pad*.lng doc\*.html lib\*.dll lib\*.pyd