import os, sys
import sixpad
from sixpad import window as win

def test () :
	print(win.chooseFolder(folder='C:\\mingw\\progq\\', root='C:\\MinGW\\'))

def pageOpened (p):
 pass

win.addEvent('pageOpened', pageOpened)
for page in win.pages: pageOpened(page)

win.addAccelerator('Ctrl+E', test)
