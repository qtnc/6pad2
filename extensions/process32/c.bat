@echo off
gcc -std=gnu99 -w -s -O3 -shared -o process32.pyd *.c -L../.. -lpython34