@echo off
cd lib
for /r %%i in (*.py) do isnewer %%i %%io && python -O ..\pycompile.py %%i
zip -9 -u -q -r ..\python34.zip *.pyo *\*.pyo *\*\*.pyo *\*\*\*.pyo
cd ..