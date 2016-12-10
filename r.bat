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
zip -9 -u -q -o -r 6pad++.zip 6pad++.exe python34.zip *.dll 6pad*.lng doc\*.html lib\*.dll lib\*.pyd plugins\*.pyd plugins\minide\*.md plugins\minide\*.html plugins\minide\*.py plugins\dialogs\*.html plugins\dialogs\*.md changelog.md readme.md license.txt